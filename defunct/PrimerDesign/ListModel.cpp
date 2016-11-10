/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "ListModel.h"

using namespace PrimerDesign;

ListModel::ListModel(int editableColumnMask, const QList<QString> &columns)
{
    editableColumnMask_ = editableColumnMask;
    columns_ = columns;
    list_ = 0;
}

void ListModel::bind(ObservableListBase *list)
{
    beginResetModel();
    unbind();
    list_ = list;

    if (list_)
    {
        QObject::connect(list->notifier(), SIGNAL(itemAdded(int)), this, SLOT(itemAdded(int)));
        QObject::connect(list->notifier(), SIGNAL(itemAdding(int)), this, SLOT(itemAdding(int)));
        QObject::connect(list->notifier(), SIGNAL(itemRemoved(int)), this, SLOT(itemRemoved(int)));
        QObject::connect(list->notifier(), SIGNAL(itemRemoving(int)), this, SLOT(itemRemoving(int)));
        QObject::connect(list->notifier(), SIGNAL(listDeleting()), this, SLOT(listDeleting()));
        QObject::connect(list->notifier(), SIGNAL(majorChanged()), this, SLOT(majorChanged()));
    }

    endResetModel();
}

int ListModel::columnCount(const QModelIndex & /* parent */) const
{
    if (list_ && list_->length())
    {
        return columns_.count();
    }

    return 0;
}

Qt::ItemFlags ListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    int oneBasedColumn = index.column() + 1;
    if (oneBasedColumn && editableColumnMask_ == oneBasedColumn)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant ListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        if (section < 0 || section >= columns_.count())
        {
            return QVariant();
        }
        else
        {
            return QVariant(columns_.at(section));
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
    {
        return section + 1;
    }
}

ObservableListBase * PrimerDesign::ListModel::list() const
{
    return list_;
}

int ListModel::rowCount(const QModelIndex & /* parent */) const
{
    if (list_)
        return list_->length();

    return 0;
}

void ListModel::unbind()
{
    if (list_)
    {
        QObject::disconnect(list_->notifier(), SIGNAL(itemAdded(int)), this, SLOT(itemAdded(int)));
        QObject::disconnect(list_->notifier(), SIGNAL(itemAdding(int)), this, SLOT(itemAdding(int)));
        QObject::disconnect(list_->notifier(), SIGNAL(itemRemoved(int)), this, SLOT(itemRemoved(int)));
        QObject::disconnect(list_->notifier(), SIGNAL(itemRemoving(int)), this, SLOT(itemRemoving(int)));
        QObject::disconnect(list_->notifier(), SIGNAL(majorChanged()), this, SLOT(majorChanged()));
    }
}

void ListModel::itemAdding(int i)
{
    if (list_ && !list_->length())
    {
        beginInsertColumns(QModelIndex(), 0, columns_.length() - 1);
        endInsertColumns();
    }

    beginInsertRows(QModelIndex(), i, i);
}

void ListModel::itemAdded(int /* i */)
{
    endInsertRows();
}

void ListModel::itemRemoving(int i)
{
    beginRemoveRows(QModelIndex(), i, i);
}

void ListModel::itemRemoved(int /* i */)
{
    endRemoveRows();
}

void ListModel::listDeleting()
{
    beginResetModel();
    list_ = 0;
    endResetModel();
}

void ListModel::majorChanged()
{
    beginResetModel();
    ObservableListBase *underlyingList = list();
    unbind();
    bind(underlyingList);
    endResetModel();
}
