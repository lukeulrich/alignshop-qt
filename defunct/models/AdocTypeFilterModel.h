/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTYPEFILTERMODEL_H
#define ADOCTYPEFILTERMODEL_H

#include "AdocSortFilterProxyModel.h"

#include "../AdocTreeNode.h"

/**
  * Abstract and concrete base class defining the interface for filtering model rows of an AdocTreeModel-compatible
  * model based on a user-supplied list of AdocTreeNode::NodeType's.
  *
  * To use this class, first call setAcceptNodeTypes() with the list of node types to accept during filtering and then
  * set the source model.
  */
class AdocTypeFilterModel : public AdocSortFilterProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    explicit AdocTypeFilterModel(QObject *parent = 0);               //!< Constructs an instance of this class

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QList<AdocTreeNode::NodeType> acceptNodeTypes() const;           //!< Returns the list of currently accepted node types
    //! Sets the list of node types to accept during filtering to acceptNodeTypes and reset the model
    void setAcceptNodeTypes(const QList<AdocTreeNode::NodeType> acceptNodeTypes);
    void tweakHorizontalHeader(QHeaderView *header) const;           //!< Tweaks the appearance of the horizontal headerview

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Virtual function that performs filtering relative to nodeType and returns true if the row is be included in the model or false otherwise
    virtual bool filterAcceptsRow(AdocTreeNode::NodeType nodeType) const;
    //! Extracts the NodeType for this row and returns the result of filterAcceptsRow(AdocTreeNode::NodeType)
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    QList<AdocTreeNode::NodeType> acceptNodeTypes_;
};

#endif // ADOCTYPEFILTERMODEL_H
