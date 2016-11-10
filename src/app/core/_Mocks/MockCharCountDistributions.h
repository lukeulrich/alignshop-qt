/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKCHARCOUNTDISTRIBUTIONS_H
#define MOCKCHARCOUNTDISTRIBUTIONS_H

#include "../CharCountDistribution.h"
#include "../InfoContentDistribution.h"

CharCountDistribution charCountDistribution1();
CharCountDistribution charCountDistribution2();
bool isEqual(const VectorVectorInfoUnit &a, const VectorVectorInfoUnit &b, double precision);


CharCountDistribution charCountDistribution1()
{
    VectorHashCharInt counts;
    // Column 1:
    counts << HashCharInt();
    counts.last().insert('A', 3);
    counts.last().insert('T', 3);
    counts.last().insert('C', 4);
    // Column 2:
    counts << HashCharInt();
    counts.last().insert('G', 10);
    // Column 3:
    counts << HashCharInt();
    counts.last().insert('A', 5);
    counts.last().insert('C', 1);

    return CharCountDistribution(counts, 10);
}

CharCountDistribution charCountDistribution2()
{
    // Divisor: 10
    VectorHashCharInt counts;

    // Column 1
    counts << HashCharInt();
    // Column 2
    counts << HashCharInt();
    counts.last().insert('A', 3);
    // Column 3
    counts << HashCharInt();
    counts.last().insert('T', 1);
    counts.last().insert('A', 5);
    // Column 4
    counts << HashCharInt();
    counts.last().insert('G', 8);
    counts.last().insert('A', 1);
    counts.last().insert('T', 1);
    // Column 5
    counts << HashCharInt();
    counts.last().insert('C', 0);
    counts.last().insert('G', 1);
    counts.last().insert('A', 7);
    counts.last().insert('T', 2);
    // Column 6
    counts << HashCharInt();

    return CharCountDistribution(counts, 10);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
bool isEqual(const VectorVectorInfoUnit &a, const VectorVectorInfoUnit &b, double precision)
{
    if (a.size() != b.size())
        return false;

    int columns = a.size();
    for (int i=0; i< columns; ++i)
    {
        if (a.at(i).size() != b.at(i).size())
            return false;

        int rows = a.at(i).size();
        for (int j=0; j<rows; ++j)
        {
            const InfoUnit &aInfoUnit = a.at(i).at(j);
            const InfoUnit &bInfoUnit = b.at(i).at(j);
            if (aInfoUnit.ch_ != bInfoUnit.ch_ ||
                fabs(aInfoUnit.percent_ - bInfoUnit.percent_) >= precision ||
                fabs(aInfoUnit.info_ - bInfoUnit.info_) >= precision)
            {
                return false;
            }
        }
    }

    return true;
}


#endif // MOCKCHARCOUNTDISTRIBUTIONS_H
