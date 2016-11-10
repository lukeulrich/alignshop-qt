/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FILTERCOLUMNPROXYMODEL_H
#define FILTERCOLUMNPROXYMODEL_H

#include <QtCore/QSet>
#include <QtGui/QSortFilterProxyModel>

#include "../../core/global.h"

class FilterColumnProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    FilterColumnProxyModel(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void excludeColumn(int sourceColumn);
    void includeColumn(int sourceColumn);
    int mapFromSource(const int sourceColumn) const;
    void setSourceModel(QAbstractItemModel *sourceModel);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParentIndex) const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool isValidSourceColumn(const int sourceColumn) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    QSet<int> excludedColumns_;
};

#endif // FILTERCOLUMNPROXYMODEL_H
