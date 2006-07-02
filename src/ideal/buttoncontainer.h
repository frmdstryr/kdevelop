/***************************************************************************
 *   Copyright (C) 2004-2006 by Alexander Dymo                             *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef IDEALBUTTONCONTAINER_H
#define IDEALBUTTONCONTAINER_H

#include <QWidget>
#include <QList>
#include <QLayout>

#include "idealdefs.h"

namespace Ideal {

class Button;
class ButtonContainer;

/**
@short A layout for a ButtonContainer class.

Overrides minimumSize method to allow shrinking button bar buttons.*/
class IDEAL_EXPORT ButtonLayout: public QBoxLayout{
public:
    ButtonLayout(Direction dir, ButtonContainer *parent);

    virtual QSize minimumSize() const;

private:
    ButtonContainer *m_buttonBar;
};

/**
@short A bar with tool buttons.

Looks like a toolbar but has another behaviour. It is suitable for
placing on the left(right, bottom, top) corners of a window as a bar with slider.
*/
class ButtonContainer: public QWidget {
    Q_OBJECT
public:
    ButtonContainer(Place place, ButtonMode mode = IconsAndText,
        QWidget *parent = 0);
    virtual ~ButtonContainer();

    /**Adds a button to the bar.*/
    virtual void addButton(Button *button, bool isShown = true);
    /**Removes a button from the bar and deletes the button.*/
    virtual void removeButton(Button *button);

    /**Sets the mode.*/
    void setMode(ButtonMode mode);
    /**@returns the mode.*/
    ButtonMode mode() const;

    /**@returns the place.*/
    Place place() const;

    bool autoResize() const;
    void setAutoResize(bool b);

    bool isEmpty();

    /**Shrinks the button bar so all buttons are visible. Shrinking is done by
    reducing the amount of text shown on buttons. Button icon size is a minimum size
    of a button. If a button does not have an icon, it displays "...".*/
    virtual void shrink(int preferredDimension, int actualDimension);
    virtual void deshrink(int preferredDimension, int actualDimension);
    /**Restores the size of button bar buttons.*/
    virtual void unshrink();

protected:
    virtual void resizeEvent(QResizeEvent *ev);

    int originalDimension();

private:
    void fixDimensions();
    void setButtonsPlace(Ideal::Place place);
    QString squeeze(const QString &str, int maxlen);

    typedef QList<Button*> ButtonList;
    ButtonList m_buttons;

    ButtonMode m_mode;
    Place m_place;

    ButtonLayout *m_buttonLayout;

    bool m_shrinked;
    bool m_autoResize;
};

}

#endif
