/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SIMPLESEQPOD_H
#define SIMPLESEQPOD_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "../BioString.h"

struct SimpleSeqPod
{
    QString name_;
    BioString sequence_;
    bool isValid_;

    SimpleSeqPod(const QString &name = QString(), const BioString &sequence = BioString(), bool valid = false)
        : name_(name), sequence_(sequence), isValid_(valid)
    {
    }

    bool operator==(const SimpleSeqPod &other) const
    {
        return name_ == other.name_ &&
               sequence_ == other.sequence_;
    }

    bool operator!=(const SimpleSeqPod &other) const
    {
        return !operator==(other);
    }

    // For iteration purposes (see ConsensusAlphabetDetector)
    BioString sequence() const
    {
        return sequence_;
    }

    static bool identicalSequenceLengths(const QVector<SimpleSeqPod> &simpleSeqPodVector)
    {
        if (simpleSeqPodVector.isEmpty())
            return false;

        QVector<SimpleSeqPod>::ConstIterator it = simpleSeqPodVector.constBegin();
        int referenceLength = it->sequence_.length();
        for (; it != simpleSeqPodVector.constEnd(); ++it)
            if (it->sequence_.length() != referenceLength)
                return false;

        return true;
    }
};

Q_DECLARE_TYPEINFO(SimpleSeqPod, Q_MOVABLE_TYPE);

#endif // SIMPLESEQPOD_H
