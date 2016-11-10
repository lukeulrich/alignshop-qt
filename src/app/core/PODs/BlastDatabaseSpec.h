/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASESPEC_H
#define BLASTDATABASESPEC_H

#include <QtCore/QString>
#include "../enums.h"

struct BlastDatabaseSpec
{
    TriBool isProtein_;
    QString databaseName_;
    qint64 nLetters_;
    qint64 nSequences_;
    qint64 nBytes_;

    BlastDatabaseSpec()
        : isProtein_(eUnknown),
          nLetters_(0),
          nSequences_(0),
          nBytes_(0)
    {
    }

    BlastDatabaseSpec(const TriBool isProtein, const QString &name, const qint64 nLetters, const qint64 nSequences, const qint64 nBytes)
        : isProtein_(isProtein),
          databaseName_(name),
          nLetters_(nLetters),
          nSequences_(nSequences),
          nBytes_(nBytes)
    {
    }
};

Q_DECLARE_TYPEINFO(BlastDatabaseSpec, Q_MOVABLE_TYPE);

#endif // BLASTDATABASESPEC_H
