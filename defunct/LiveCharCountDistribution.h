/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef LIVECHARCOUNTDISTRIBUTION_H
#define LIVECHARCOUNTDISTRIBUTION_H

#include "CharCountDistribution.h"

/**
  * LiveCharCountDistribution defines the abstract interface for representing an observable character count
  * distribution.
  *
  * At its most basic level, a LiveCharCountDistribution must define a method for returning the raw character count
  * distribution and emit the appropriate signals when the distribution is changed.
  */
class LiveCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Trivial constructor and destructor
    LiveCharCountDistribution(QObject *parent = 0) : QObject(parent)
    {
    }

    virtual ~LiveCharCountDistribution()
    {
    }

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual CharCountDistribution charCountDistribution() const = 0;    //!< Returns the current character count distribution
    virtual int divisor() const = 0;                                    //!< Returns an integer value denoting the divisor value for converting integer counts to proportions

signals:
    void columnsInserted(int from, int to);                             //!< Emitted after the columns between from and to (inclusive, 1-based) have been inserted
    void columnsRemoved(int from, int to);                              //!< Emitted after the columns between from and to (inclusive, 1-based) have been removed)
    void dataChanged(int startColumn, int stopColumn);                  //!< Emitted when any of the character counts between startColumn and stopColumn (inclusive, 1-baed) have changed
};

#endif // LIVECHARCOUNTDISTRIBUTION_H
