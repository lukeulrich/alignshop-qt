/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CPDETASKMODEL_H
#define CPDETASKMODEL_H

#include <QtCore/QAbstractItemModel>
#include "../../core/global.h"

class TaskModel;
class TaskTreeNode;

class CpdeTaskModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CpdeTaskModel(QObject *parent = nullptr);

    // Reimplemented public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    // Public methods
    TaskModel *currentTaskModel() const;
    TaskModel *pendingTaskModel() const;
    TaskModel *doneTaskModel() const;
    TaskModel *errorTaskModel() const;

private Q_SLOTS:
//    void onTaskModelAboutToBeReset();
    void onTaskModelDataChanged(const QModelIndex &begin, const QModelIndex &end);
//    void onTaskModelReset();
    void onTaskModelRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void onTaskModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void onTaskModelRowsInserted(const QModelIndex &parent, int start, int end);
    void onTaskModelRowsRemoved(const QModelIndex &parent, int start, int end);

private:
    TaskModel *modelFromRow(int row) const;
    TaskModel *modelWithRoot(TaskTreeNode *rootTaskNode) const;
    int rowFromModel(TaskModel *model) const;

    TaskModel *currentTaskModel_;
    TaskModel *pendingTaskModel_;
    TaskModel *doneTaskModel_;
    TaskModel *errorTaskModel_;
};

#endif // CPDETASKMODEL_H
