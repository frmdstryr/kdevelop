/* This file is part of the KDE project
   Copyright 2006 David Nolden <david.nolden.kde@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef IQUICKOPEN_H
#define IQUICKOPEN_H

#include <iextension.h>
#include <languageexport.h>

namespace KDevelop
{

class QuickOpenDataProviderBase;

/**
 * Interface to quickopen
*/
class KDEVPLATFORMLANGUAGE_EXPORT IQuickOpen
{
public:
    virtual ~IQuickOpen() {}

    /**
     * Registers a new provider under a specified name.
     * There may be multiple providers with the same name, they will be used simultaneously in that case.
     * @param name Name of the provider, Example: "Files". The name will be shown in the GUI, so should be translated.
     * @param provider The provider. It does not need to be explicitly removed before its destruction.
     * */
    virtual void registerProvider( const QString& name, QuickOpenDataProviderBase* provider ) = 0;

    /**
     * Remove provider.
     * @param provider The provider to remove
     * @return Whether a provider was removed. If false, the provider was not attached.
     * */
    virtual bool removeProvider( QuickOpenDataProviderBase* provider ) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS(KDevelop, IQuickOpen, "org.kdevelop.IQuickOpen")
Q_DECLARE_INTERFACE(KDevelop::IQuickOpen, "org.kdevelop.IQuickOpen")

#endif // IQuickOpen_H
