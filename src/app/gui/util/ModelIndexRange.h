/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MODELINDEXRANGE_H
#define MODELINDEXRANGE_H

#include <QtCore/QModelIndex>
#include <QtCore/QVector>

struct ModelIndexRange
{
    QModelIndex parentIndex_;
    int start_;
    int count_;

    ModelIndexRange(const QModelIndex parentIndex = QModelIndex(), int start = 0, int count = -1)
        : parentIndex_(parentIndex), start_(start), count_(count)
    {
    }
};

template<typename T>
QVector<ModelIndexRange> convertIndicesToRanges(T indices)
{
    if (indices.isEmpty())
        return QVector<ModelIndexRange>();

    qStableSort(indices);

    QVector<ModelIndexRange> ranges;
    ranges << ModelIndexRange(indices.at(0).parent(), indices.at(0).row(), 1);
    for (int i=1, z=indices.size(); i<z; ++i)
    {
        ModelIndexRange &last = ranges.last();
        int lastRow = last.start_ + last.count_ - 1;
        if (lastRow + 1 == indices.at(i).row())
            ++last.count_;
        else if (lastRow != indices.at(i).row())
            ranges << ModelIndexRange(indices.at(i).parent(), indices.at(i).row(), 1);
    }

    return ranges;
}

#endif // MODELINDEXRANGE_H
