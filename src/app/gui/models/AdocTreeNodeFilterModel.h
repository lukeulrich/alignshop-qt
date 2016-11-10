/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREENODEFILTERMODEL_H
#define ADOCTREENODEFILTERMODEL_H

#include <QtCore/QSet>
#include <QtGui/QSortFilterProxyModel>
#include "../../core/enums.h"

class AdocTreeModel;

class AdocTreeNodeFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit AdocTreeNodeFilterModel(QObject *parent = 0);

    void setAcceptableNodeTypes(QSet<AdocNodeType> acceptableNodeTypes);
    QSet<AdocNodeType> acceptableNodeTypes() const;

    void setSourceModel(AdocTreeModel *sourceModel);

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    AdocTreeModel *adocTreeModel_;
    QSet<AdocNodeType> acceptableNodeTypes_;
};

#endif // ADOCTREENODEFILTERMODEL_H
