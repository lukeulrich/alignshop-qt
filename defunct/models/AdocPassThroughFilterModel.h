/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCPASSTHROUGHFILTERMODEL_H
#define ADOCPASSTHROUGHFILTERMODEL_H

#include "AdocSortFilterProxyModel.h"

/**
  * AdocPassThroughFilterModel does not filtering whatsoever, but rather provides access to its source
  * model data at the same level of proxy models as the Amino/Rna/Dna filter models.
  *
  * It is intended to simplify the access and manipulation to the data tree model when viewing all items.
  * While this does incur the cost of stepping through additional proxy model layers, this is minimal
  * compared to the maintenance and ease of use it provides.
  */
class AdocPassThroughFilterModel : public AdocSortFilterProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit AdocPassThroughFilterModel(QObject *parent = 0);           //!< Trivial constructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;   //!< Returns 1 because there is only one column when viewing all items
    //!< Returns the data for the given role and section in the header with the specified orientation
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void tweakHorizontalHeader(QHeaderView *header) const;              //!< Tweaks the horizontal header appearance
};

#endif // ADOCPASSTHROUGHFILTERMODEL_H
