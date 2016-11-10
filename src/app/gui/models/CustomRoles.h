/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREEMODELROLES_H
#define ADOCTREEMODELROLES_H

#include <QtCore/qnamespace.h>  // To obtain the value of Qt::UserRole

namespace CustomRoles
{
    const int kIsCutRole = Qt::UserRole + 1;
    const int kFullBlastPathRole = Qt::UserRole + 2;
    const int kMinRole = Qt::UserRole + 3;
    const int kMaxRole = Qt::UserRole + 4;
}

#endif // ADOCTREEMODELROLES_H
