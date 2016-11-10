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

#include "SliceProxyModel.h"
#include "AdocTreeNode.h"
#include "DataRow.h"

#include <QtDebug>

struct VirtualSliceSignal
{
    QString type_;
    int start_;
    int end_;

    VirtualSliceSignal(QString type, int start = -1, int end = -1) :
            type_(type), start_(start), end_(end)
    {
    }
};


/**
  * Mock slice model that only accepts AdocTreeNodes with labels that begin with [AaBb].
  * Nodes that begin with [Bb] have two columns, the first is the actual label, and the
  * second is the label appended with -swank.
  * Used for testing the SliceModel interface.
  */
class MockSliceProxyModel : public SliceProxyModel
{
    Q_OBJECT

public:
    MockSliceProxyModel(QObject *parent = 0) : SliceProxyModel(parent)
    {
    }

    // Return the maximum column count
    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount())
            return QVariant();

        AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
        if (index.column() == 0)
            return data_.at(index.row()).first;
        else if (index.column() == 1 && node->label_.startsWith('B', Qt::CaseInsensitive))
            return data_.at(index.row()).second;

        return QVariant();
    }

    void setSourceParent(const QModelIndex &sourceParentIndex)
    {
        data_.clear();

        SliceProxyModel::setSourceParent(sourceParentIndex);
    }

    bool isEditableColumn(int column) const
    {
        if (column == 0 || column == 1)
            return true;

        return false;
    }

    bool filterAcceptsNode(AdocTreeNode *node) const
    {
        if (node->label_.startsWith('A', Qt::CaseInsensitive))
        {
            data_ << qMakePair(node->label_, QString());
            return true;
        }
        else if (node->label_.startsWith('B', Qt::CaseInsensitive))
        {
            data_ << qMakePair(node->label_, node->label_ + QString("-swank"));
            return true;
        }

        return false;
    }

protected:
    void sliceCleared()
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceCleared"));
    }

    void sliceRowsAboutToBeRemoved(int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsAboutToBeRemoved", start, end));
    }

    void sliceRowsInserted(int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsInserted", start, end));
    }

    void sliceRowsRemoved(int start, int end)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("sliceRowsRemoved", start, end));
    }

    void taggedSliceCreated(int tag)
    {
        virtualSignalCounts_.append(VirtualSliceSignal("taggedSliceCreated"));

        SliceProxyModel::taggedSliceCreated(tag);
    }

private:
    QList<VirtualSliceSignal> virtualSignalCounts_;

    mutable QList<QPair<QString, QString> > data_;

    friend class TestSliceProxyModel;
};

#endif
