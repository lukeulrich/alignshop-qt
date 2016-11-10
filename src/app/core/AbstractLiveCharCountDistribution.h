/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTLIVECHARCOUNTDISTRIBUTION_H
#define ABSTRACTLIVECHARCOUNTDISTRIBUTION_H

#include "CharCountDistribution.h"
#include "global.h"
#include "util/ClosedIntRange.h"

/**
  * AbstractLiveCharCountDistribution defines the abstract interface for representing an observable character count
  * distribution.
  *
  * At its most basic level, a LiveCharCountDistribution must define a method for returning the raw character count
  * distribution and emit the appropriate signals when the distribution is changed.
  */
class AbstractLiveCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Trivial destructor
    virtual ~AbstractLiveCharCountDistribution() {}

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual CharCountDistribution charCountDistribution() const = 0;    //!< Returns the current character count distribution
    int length() const
    {
        return charCountDistribution().length();
    }

signals:
    void columnsInserted(const ClosedIntRange &range);                  //!< Emitted after the columns in range (inclusive, 1-based) have been inserted
    void columnsRemoved(const ClosedIntRange &range);                   //!< Emitted after the columns in range (inclusive, 1-based) have been removed)
    void dataChanged(const ClosedIntRange &range);                      //!< Emitted when any of the character counts in range (inclusive, 1-baed) have changed

protected:
    // ------------------------------------------------------------------------------------------------
    // Trivial constructor
    AbstractLiveCharCountDistribution(QObject *parent = nullptr) : QObject(parent) {}
};

#endif // ABSTRACTLIVECHARCOUNTDISTRIBUTION_H
