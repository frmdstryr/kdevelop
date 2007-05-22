/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "identifiedtype.h"
#include "declaration.h"

IdentifiedType::IdentifiedType()
  : m_declaration(0)
{
}

QualifiedIdentifier IdentifiedType::identifier() const
{
  return m_declaration ? m_declaration->qualifiedIdentifier() : QualifiedIdentifier();
}

Declaration* IdentifiedType::declaration() const
{
  return m_declaration;
}

void IdentifiedType::setDeclaration(Declaration* declaration)
{
  m_declaration = declaration;
}

QString IdentifiedType::idMangled() const
{
  return identifier().mangled();
}
