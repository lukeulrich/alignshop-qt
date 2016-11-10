/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKLIVECHARCOUNTDISTRIBUTION_H
#define MOCKLIVECHARCOUNTDISTRIBUTION_H

#include <QtCore/QObject>

#include "../AbstractLiveCharCountDistribution.h"
#include "../CharCountDistribution.h"
#include "../macros.h"

class MockLiveCharCountDistribution : public AbstractLiveCharCountDistribution
{
public:
    MockLiveCharCountDistribution(QObject *parent = nullptr)
        : AbstractLiveCharCountDistribution(parent)
    {
    }

    CharCountDistribution charCountDistribution() const
    {
        return distribution_;
    }

    void setDistribution(const CharCountDistribution &distribution)
    {
        distribution_ = distribution;
    }

    void addData(QByteArray byteArray, int offset = 1)
    {
        distribution_.add(byteArray, '\0', offset);
        emit dataChanged(ClosedIntRange(offset, offset + byteArray.length() - 1));
    }

    void remove(int position, int amount = 1)
    {
        distribution_.remove(position, amount);
        emit columnsRemoved(ClosedIntRange(position, position + amount - 1));
    }

    void insert(int position, int amount = 1)
    {
        distribution_.insertBlanks(position, amount);
        emit columnsInserted(ClosedIntRange(position, position + amount - 1));
    }

    CharCountDistribution distribution_;
};

#endif // MOCKLIVECHARCOUNTDISTRIBUTION_H
