/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippet.h"
#include "globals.h"

#include <QtDebug>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>

#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

#include "snippetstore.h"
#include "snippetrepository.h"

Snippet::Snippet(const QString& filename, SnippetRepository* repo)
    : QStandardItem(filename), repo_(repo), name_(filename)
{
    // append ourself to the given parent repo
    repo->addSnippet( this );

    // tell the new snippet to get it's data from the file
    QTimer::singleShot(0, this, SLOT(slotSyncSnippet()));

    kDebug() << "created new snippet" << getFileName() << this << repo_;
}

Snippet::~Snippet()
{
}

void Snippet::slotSyncSnippet()
{
    kDebug() << "syncing snippet" << getFileName() << this << repo_;

    if (!QFile::exists( getFileName() ))
        return;

    QFile f( getFileName() );
    if(f.open(QIODevice::ReadOnly)) {
        QTextStream input( &f );
        setRawData( input.readAll() );
    }
}

void Snippet::save()
{
    QFile f( getFileName() );
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << snippetText_;

        foreach(const QString& keyword, keywords_) {
            input << endl << SNIPPET_METADATA << "keyword=" << keyword;
        }
    }
}

const QString Snippet::getFileName()
{
    return repo_->getLocation() + QDir::separator() + name_;
}

void Snippet::changeName(const QString& newName)
{
    // If there is no name yet, e.g. the snippet has just been created
    // we don't need to care about changing the filename
    if (name_.isEmpty()) {
        name_ = newName;
        setText( newName );
        return;
    }

    // In case the user changed the name of the snippet,
    // we need to move the current file.
    if (name_ != newName) {
        QString origFileName = getFileName();
        name_ = newName;
        QString newFileName = getFileName();

        if ( QFile::exists( origFileName ) ) {
            if (QFile::rename(origFileName, newFileName)) {
                setText( newName );
            } else {
                KMessageBox::error(
                    QApplication::activeWindow(),
                    i18n("Could not move snippet file from \"%1\" to \"%2\".", origFileName, newFileName)
                );
            }
        } else {
            setText( newName );
            save();
        }
    }

    // no need to do anything if the name didn't change
}

void Snippet::setRawData(const QString& data)
{
    QStringList rows = data.split( QRegExp("[\\r\\n]+"), QString::SkipEmptyParts );
    QStringList metadata;

    QString newText;

    // A snippet file can contain meta information
    // which needs to be separated from the snippet's text
    QStringListIterator it(rows);
    bool needNewline = false;
    while (it.hasNext()) {
        QString str = it.next();

        if (str.startsWith(SNIPPET_METADATA)) {
            metadata << str;
        } else {
            if ( needNewline ) {
                newText += SNIPPET_END_OF_LINE;
            }
            newText += str;
            needNewline = true;
        }
    }

    setSnippetText( newText );
    keywords_.clear();

    if (metadata.count() > 0) {
        // if there is meta information, call handleMetaData() which is
        // capable of parsing them
        handleMetaData( metadata );
    }
}

void Snippet::handleMetaData(QStringList& metadata)
{
    //Each line looks like this:
    //##META## keyword = abcdef
    QRegExp rx("(\\w+)\\s*=\\s*(\\w*)");

    QString str;
    foreach(str, metadata) {
        // strip off the metadata prefix from the lines
        str.remove(QString(SNIPPET_METADATA));

        // now use the regexp to get the name and the value from this metadata-line
        int pos = rx.indexIn( str );
        if (pos > -1) {
            QString name = rx.cap(1);
            QString value = rx.cap(2);

            // keywords get moved into the keywords list
            if (name.toLower() == "keyword") {
                keywords_ << value;
            }
        }
    }
}

void Snippet::removeSnippetFile()
{
    if (!QFile::exists( getFileName() ) || QFile::remove( getFileName() )) {
        // if the file has been removed, also remove the Snippet from the model
        QStandardItem::parent()->removeRows( row(), 1 );
    } else {
        KMessageBox::error(
            QApplication::activeWindow(),
            i18n("Could not remove snippet file \"%1\".", getFileName())
        );
    }
}

#include "snippet.moc"
