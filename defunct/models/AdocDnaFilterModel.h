/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCDNAFILTERMODEL_H
#define ADOCDNAFILTERMODEL_H

#include "AdocTypeFilterModel.h"

class QHeaderView;

/**
  * Concete class that filters out all nodes except container nodes and DNA nodes (SeqDnaType and
  * SubseqDnaType).
  */
class AdocDnaFilterModel : public AdocTypeFilterModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    explicit AdocDnaFilterModel(QObject *parent = 0);         //!< Constructs an instance of this class

    int columnCount(const QModelIndex &parent = QModelIndex()) const;   //!< Return the number of columns for the children of parent
    //!< Returns the data for the given role and section in the header with the specified orientation
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void tweakHorizontalHeader(QHeaderView *header) const;      //!< Tweaks the appearance of the horizontal headerview

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Returns true if nodeType is a container, SeqDnaType, or SubseqDnaType; false otherwise
    bool filterAcceptsRow(AdocTreeNode::NodeType nodeType) const;
};

#endif // ADOCDNAFILTERMODEL_H
