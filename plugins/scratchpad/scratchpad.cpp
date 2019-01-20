/* This file is part of KDevelop
 *
 * Copyright 2018 Amish K. Naidu <amhndu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "scratchpad.h"
#include "scratchpadview.h"
#include "scratchpadjob.h"

#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iruncontroller.h>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KActionCollection>

#include <QStandardItemModel>
#include <QStandardPaths>
#include <QDir>
#include <QFileIconProvider>
#include <QHash>

#include <algorithm>

K_PLUGIN_FACTORY_WITH_JSON(ScratchpadFactory, "scratchpad.json", registerPlugin<Scratchpad>(); )

class ScratchpadToolViewFactory
    : public KDevelop::IToolViewFactory
{
public:
    explicit ScratchpadToolViewFactory(Scratchpad* plugin)
        : m_plugin(plugin)
    {}

    QWidget* create(QWidget* parent = nullptr) override
    {
        return new ScratchpadView(parent, m_plugin);
    }

    Qt::DockWidgetArea defaultPosition() override
    {
        return Qt::LeftDockWidgetArea;
    }

    QString id() const override
    {
        return QStringLiteral("org.kdevelop.scratchpad");
    }

private:
    Scratchpad* const m_plugin;
};

namespace {
KConfigGroup scratchCommands()
{
    return KSharedConfig::openConfig()->group("Scratchpad").group("Commands");
}

KConfigGroup mimeCommands()
{
    return KSharedConfig::openConfig()->group("Scratchpad").group("Mime Commands");
}

QString commandForScratch(const QFileInfo& file)
{
    if (scratchCommands().hasKey(file.fileName())) {
        return scratchCommands().readEntry(file.fileName());
    }

    const auto suffix = file.suffix();
    if (mimeCommands().hasKey(suffix)) {
        return mimeCommands().readEntry(suffix);
    }

    const static QHash<QString, QString> defaultCommands = {
        {QStringLiteral("cpp"), QStringLiteral("g++ -std=c++11 -o /tmp/a.out $f && /tmp/a.out")},
        {QStringLiteral("py"), QStringLiteral("python $f")},
        {QStringLiteral("js"), QStringLiteral("node $f")},
        {QStringLiteral("c"), QStringLiteral("gcc -o /tmp/a.out $f && /tmp/a.out")},
    };

    return defaultCommands.value(suffix);
}
}

Scratchpad::Scratchpad(QObject* parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("scratchpad"), parent)
    , m_factory(new ScratchpadToolViewFactory(this))
    , m_model(new QStandardItemModel(this))
    , m_runAction(new QAction(this))
{
    Q_UNUSED(args);

    qCDebug(PLUGIN_SCRATCHPAD) << "Scratchpad plugin is loaded!";

    core()->uiController()->addToolView(i18n("Scratchpad"), m_factory);

    const QDir dataDir(dataDirectory());
    if (!dataDir.exists()) {
        qCDebug(PLUGIN_SCRATCHPAD) << "Creating directory" << dataDir;
        dataDir.mkpath(QStringLiteral("."));
    }

    const QFileInfoList scratches = dataDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (const auto& fileInfo : scratches) {
        addFileToModel(fileInfo);

        // TODO if scratch is open (happens when restarting), set pretty name, below code doesn't work
//         auto* document = core()->documentController()->documentForUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
//         if (document) {
//             document->setPrettyName(i18n("scratch:%1", fileInfo.fileName()));
//         }
    }
}

QStandardItemModel* Scratchpad::model() const
{
    return m_model;
}

QString Scratchpad::dataDirectory()
{
    const static QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                                                              + QLatin1String("/kdevscratchpad/scratches/");
    return dir;
}

void Scratchpad::openScratch(const QModelIndex& index)
{
    const QUrl scratchUrl = QUrl::fromLocalFile(index.data(FullPathRole).toString());
    auto* const document = core()->documentController()->openDocument(scratchUrl);
    document->setPrettyName(i18nc("prefix to distinguish scratch tabs", "scratch:%1", index.data().toString()));
}

void Scratchpad::runScratch(const QModelIndex& index)
{
    qCDebug(PLUGIN_SCRATCHPAD) << "run" << index.data().toString();

    auto command = index.data(RunCommandRole).toString();
    command.replace(QLatin1String("$f"), index.data(FullPathRole).toString());
    if (!command.isEmpty()) {
        auto* job = new ScratchpadJob(command, index.data().toString(), this);
        core()->runController()->registerJob(job);
    }
}

void Scratchpad::removeScratch(const QModelIndex& index)
{
    const QString path = index.data(FullPathRole).toString();
    if (auto* document = core()->documentController()->documentForUrl(QUrl::fromLocalFile(path))) {
        document->close();
    }

    if (QFile::remove(path)) {
        qCDebug(PLUGIN_SCRATCHPAD) << "removed" << index.data(FullPathRole);
        scratchCommands().deleteEntry(index.data().toString());
        m_model->removeRow(index.row());
    } else {
        emit actionFailed(i18n("Failed to remove scratch: %1", index.data().toString()));
    }
}

void Scratchpad::createScratch(const QString& name)
{
    if (!m_model->findItems(name).isEmpty()) {
        emit actionFailed(i18n("Failed to create scratch: Name already in use"));
        return;
    }

    QFile file(dataDirectory() + name);
    if (!file.exists() && file.open(QIODevice::WriteOnly)) { // create a new file if it doesn't exist
        file.close();
    }

    if (file.exists()) {
        addFileToModel(file);
    } else {
        emit actionFailed(i18n("Failed to create new scratch"));
    }
}

void Scratchpad::renameScratch(const QModelIndex& index, const QString& previousName)
{
    const QString newName = index.data().toString();
    if (newName.contains(QDir::separator())) {
        m_model->setData(index, previousName); // undo
        emit actionFailed(i18n("Failed to rename scratch: Names must not include path seperator"));
        return;
    }

    const QString previousPath = dataDirectory() + previousName;
    const QString newPath = dataDirectory() + index.data().toString();
    if (previousPath == newPath) {
        return;
    }

    if (QFile::rename(previousPath, newPath)) {
        qCDebug(PLUGIN_SCRATCHPAD) << "renamed" << previousPath << "to" << newPath;

        m_model->setData(index, newPath, Scratchpad::FullPathRole);
        m_model->itemFromIndex(index)->setIcon(m_iconProvider.icon(QFileInfo(newPath)));
        auto config = scratchCommands();
        config.deleteEntry(previousName);
        config.writeEntry(newName, index.data(Scratchpad::RunCommandRole));

        // close old and re-open the closed document
        if (auto* document = core()->documentController()->documentForUrl(QUrl::fromLocalFile(previousPath))) {
            // FIXME is there a better way ? this feels hacky
            document->close();
            document = core()->documentController()->openDocument(QUrl::fromLocalFile(newPath));
            document->setPrettyName(i18nc("prefix to distinguish scratch tabs", "scratch:%1", index.data().toString()));
        }
    } else {
        qCWarning(PLUGIN_SCRATCHPAD) << "failed renaming" << previousPath << "to" << newPath;
        // rollback
        m_model->setData(index, previousName);
        emit actionFailed(i18n("Failed renaming scratch."));
    }
}

void Scratchpad::addFileToModel(const QFileInfo& fileInfo)
{
    auto* const item = new QStandardItem(m_iconProvider.icon(fileInfo), fileInfo.fileName());
    item->setData(fileInfo.absoluteFilePath(), FullPathRole);
    const auto command = commandForScratch(fileInfo);
    item->setData(command, RunCommandRole);
    scratchCommands().writeEntry(item->text(), item->data(RunCommandRole));
    m_model->appendRow(item);
}

void Scratchpad::setCommand(const QModelIndex& index, const QString& command)
{
    qCDebug(PLUGIN_SCRATCHPAD) << "set command" << index.data();
    m_model->setData(index, command, RunCommandRole);
    scratchCommands().writeEntry(index.data().toString(), command);

    mimeCommands().writeEntry(QFileInfo(index.data().toString()).suffix(), command);
}

QAction* Scratchpad::runAction() const
{
    return m_runAction;
}

void Scratchpad::createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions)
{
    Q_UNUSED(window);

    xmlFile = QStringLiteral("kdevscratchpad.rc");

    // add to gui action collection, so that the shorcut is easily configurable
    // action setup done in ScratchpadView
    actions.addAction(QStringLiteral("run_scratch"), m_runAction);
}


#include "scratchpad.moc"