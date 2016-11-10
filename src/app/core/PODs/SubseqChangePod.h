/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SUBSEQCHANGEPOD_H
#define SUBSEQCHANGEPOD_H

#include <QtCore/QVector>

#include "../util/ClosedIntRange.h"
#include "../BioString.h"
#include "../macros.h"

// Really describes a change event in the context of a Msa.
struct SubseqChangePod
{
    enum TrimExtOp
    {
        eExtendLeft = 0,
        eExtendRight,
        eTrimLeft,
        eTrimRight,
        eInternal       // Such as from a collapse operation
    };

    int row_;
    ClosedIntRange columns_;        // Msa space
    TrimExtOp operation_;
    BioString difference_;

    SubseqChangePod() : row_(0), columns_(ClosedIntRange(0, 0)), operation_(eExtendLeft)
    {
    }

    SubseqChangePod(int row, const ClosedIntRange &columns, TrimExtOp operation, const BioString &difference)
        : row_(row), columns_(columns), operation_(operation), difference_(difference)
    {
        ASSERT(columns_.length() == difference_.length());
    }

    bool operator==(const SubseqChangePod &other) const
    {
        return row_ == other.row_ &&
                columns_ == other.columns_ &&
                operation_ == other.operation_ &&
                difference_ == other.difference_;
    }

    bool isNull() const
    {
        return row_ <= 0;
    }
};

Q_DECLARE_TYPEINFO(SubseqChangePod, Q_MOVABLE_TYPE);

typedef QVector<SubseqChangePod> SubseqChangePodVector;

#endif // SUBSEQCHANGEPOD_H
