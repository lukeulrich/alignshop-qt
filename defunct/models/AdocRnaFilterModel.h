/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCRNAFILTERMODEL_H
#define ADOCRNAFILTERMODEL_H

#include "AdocTypeFilterModel.h"

class QHeaderView;

/**
  * Concete class that filters out all nodes except container nodes and DNA nodes (SeqRnaType and
  * SubseqRnaType).
  */
class AdocRnaFilterModel : public AdocTypeFilterModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    explicit AdocRnaFilterModel(QObject *parent = 0);         //!< Constructs an instance of this class

    int columnCount(const QModelIndex &parent = QModelIndex()) const;   //!< Return the number of columns for the children of parent
    //!< Returns the data for the given role and section in the header with the specified orientation
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void tweakHorizontalHeader(QHeaderView *header) const;      //!< Tweaks the styling of the horizontal headerview

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Returns true if nodeType is a container, SeqRnaType, or SubseqRnaType; false otherwise
    bool filterAcceptsRow(AdocTreeNode::NodeType nodeType) const;
};

#endif // ADOCRNAFILTERMODEL_H
