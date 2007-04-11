/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSMAINVIEW_H
#define CVSMAINVIEW_H

#include <QWidget>
#include <KJob>

#include "ui_cvsmainview.h"

class CvsPart;
class CvsGenericOutputView;

/**
 * This class is the main output view of KDevelop's CVS plugin.
 * It only constists out of a KTabWidget. 
 * 
 * When created, a CvsGenericOutputView will be inserted.
 * Inserting text into that default output view is possible via the
 * slotJobFinished() slot. 
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsMainView : public QWidget, private Ui::CvsMainViewBase {
    Q_OBJECT
public:
    CvsMainView(CvsPart *part, QWidget* parent);
    virtual ~CvsMainView();

public slots:
    /**
     * Inserts @p tag into the KTabWidget and calls it @p label
     */
    void slotAddTab(QWidget* tab, QString label);

    /**
     * When this slot gets called, the output of the job will be written to 
     * the default outputview of the KTabWidget.
     */
    void slotJobFinished(KJob* job);

private:
    CvsPart* m_part;
    CvsGenericOutputView* m_mainview;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
