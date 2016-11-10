/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKSLICEMODEL_H
#define MOCKSLICEMODEL_H

#include <QtSql/QSqlField>

#include "SliceModel.h"
#include "AdocTreeNode.h"
#include "DataRow.h"

struct VirtualSliceSignal
{
    QString type_;
    Slice *slice_;
    int start_;
    int end_;

    VirtualSliceSignal(QString type, Slice *slice = 0, int start = -1, int end = -1) :
            type_(type), slice_(slice), start_(start), end_(end)
    {
    }
};


/**
  * Mock slice model that only accepts AdocTreeNodes with labels that begin with [AaBb].
  * Nodes that begin with [Bb] have two columns, the first is the actual label, and the
  * second is the label appended with -swank.
  * Used for testing the SliceModel interface.
  */
class MockSliceModel : public SliceModel
{
    Q_OBJECT

public:
    MockSliceModel(QObject *parent = 0) : SliceModel(parent)
    {
    }

    bool isValidColumn(Slice *slice, int row, int column) const
    {
        AdocTreeNode *node = slice->sourceNodes_.at(row);
        if (node->label_.startsWith('B', Qt::CaseInsensitive)
            && column < 2)
        {
            return true;
        }

        if (column == 0)
            return true;

        return false;
    }

    // Return the maximum column count
    int columnCount(const QModelIndex & /* parent */) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        Slice *slice = getParentSlice(index);
        if (!slice)
            return QVariant();

        AdocTreeNode *node = slice->sourceNodes_.at(index.row());
        if (index.column() == 0)
            return slice->rows_.at(index.row()).value("label");
        else if (index.column() == 1 && node->label_.startsWith('B', Qt::CaseInsensitive))
            return slice->rows_.at(index.row()).value("rlabel");

        return QVariant();
    }

    int primaryColumn() const
    {
        return 0;
    }

    bool isEditableColumn(int column) const
    {
        if (column == 0 || column == 1)
            return true;

        return false;
    }

    Slice *createSlice(AdocTreeNode *parent, int start, int end) const
    {
        Slice *slice = new Slice();

        // Because end is inclusive and in the for loop we utilized the condition
//        int z = (end == -1) ? parent->childCount() : end + 1;

        for (int i=start; i<=end; ++i)
        {
            AdocTreeNode *node = static_cast<AdocTreeNode *>(parent->childAt(i));
            if (node->label_.startsWith('A', Qt::CaseInsensitive))
            {
                DataRow row;
                row.append(QSqlField("label", QVariant::String));
                row.setValue("label", node->label_);

                slice->sourceNodes_.append(node);
                slice->sourceRowIndices_.append(i);
                slice->rows_.append(row);
            }
            else if (node->label_.startsWith('B', Qt::CaseInsensitive))
            {
                DataRow row;
                row.append(QSqlField("label", QVariant::String));
                row.setValue("label", node->label_);
                row.append(QSqlField("rlabel", QVariant::String));
                row.setValue("rlabel", node->label_ + QString("-swank"));

                slice->sourceNodes_.append(node);
                slice->sourceRowIndices_.append(i);
                slice->rows_.append(row);
            }
        }

        return slice;
    }

protected:
    // Override the virtual slice signals and append their parameters to the internal virtualSignalCounts_ list for
    // testing purposes.
    void sliceAboutToBeRemoved(Slice *slice)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceAboutToBeRemoved", slice));
    }

    void sliceInserted(Slice *slice)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceInserted", slice));
    }

    void slicesReset()
    {
        virtualSignalCounts_.append(VirtualSliceSignal("slicesReset"));
    }

    void sliceRowsAboutToBeRemoved(Slice *slice, int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsAboutToBeRemoved", slice, start, end));
    }

    void sliceRowsInserted(Slice *slice, int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsInserted", slice, start, end));
    }

    void sliceRowsRemoved(Slice *slice, int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsRemoved", slice, start, end));
    }

private:
    QList<VirtualSliceSignal> virtualSignalCounts_;

    friend class TestSliceModel;
};

#endif
