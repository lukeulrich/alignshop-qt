/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASEMETAPOD_H
#define BLASTDATABASEMETAPOD_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include "../enums.h"

/**
  * Note that file_ does not necessarily correlate to a real file in the traditional sense. While it must exist at some
  * point in time as a FASTA formatted file, it is possible that it could have been renamed or deleted after the BLAST
  * database has been constructed. Nonetheless, it is not essential for it to exist to still function with the BLAST
  * tool.
  */
struct BlastDatabaseMetaPod
{
    QString title_;
    QString file_;
    TriBool isProtein_;
    qint64 nLetters_;
    qint64 nSequences_;
    qint64 nBytes_;
    QString note_;
    bool missingDependency_;

    BlastDatabaseMetaPod()
        : isProtein_(eUnknown),
          nLetters_(0),
          nSequences_(0),
          nBytes_(0),
          missingDependency_(false)
    {
    }

    bool operator==(const BlastDatabaseMetaPod &other) const
    {
        return title_ == other.title_ &&
               file_ == other.file_ &&
               isProtein_ == other.isProtein_ &&
               nLetters_ == other.nLetters_ &&
               nSequences_ == other.nSequences_ &&
               nBytes_ == other.nBytes_ &&
               missingDependency_ == other.missingDependency_;
    }

    bool operator!=(const BlastDatabaseMetaPod &other) const
    {
        return !operator==(other);
    }

    bool isProtein() const
    {
        return isProtein_ == eTrue;
    }

    bool isNucleotide() const
    {
        return isProtein_ == eFalse;
    }

    bool isUnknownType() const
    {
        return isProtein_ == eUnknown;
    }
};

Q_DECLARE_TYPEINFO(BlastDatabaseMetaPod, Q_MOVABLE_TYPE);

typedef QVector<BlastDatabaseMetaPod> BlastDatabaseMetaPodVector;

#endif // BLASTDATABASEMETAPOD_H
