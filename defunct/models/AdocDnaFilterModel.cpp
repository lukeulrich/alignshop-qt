/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocDnaFilterModel.h"

#include <QtGui/QHeaderView>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Trivial constructor
  */
AdocDnaFilterModel::AdocDnaFilterModel(QObject *parent) : AdocTypeFilterModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &] - unused
  * @returns int
  */
int AdocDnaFilterModel::columnCount(const QModelIndex & /* parent */) const
{
    return 9;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocDnaFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case constants::kColumnDnaName:
            return "Name";
        case constants::kColumnDnaParent:
            return "Parent";
        case constants::kColumnDnaSource:
            return "Source";
        case constants::kColumnDnaGene:
            return "Gene";
        case constants::kColumnDnaStart:
            return "Start";
        case constants::kColumnDnaStop:
            return "Stop";
        case constants::kColumnDnaSequence:
            return "Sequence";
        case constants::kColumnDnaNotes:
            return "Notes";
        default:
            return QVariant();
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;
}

/**
  * @param header [QHeaderView *]
  */
void AdocDnaFilterModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocDnaFilterModel::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    AdocTypeFilterModel::tweakHorizontalHeader(header);

    header->setResizeMode(constants::kColumnDnaStart, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnDnaStop, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnDnaSequence, QHeaderView::Stretch);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param nodeType [AdocTreeNode::NodeType]
  * @returns bool
  */
bool AdocDnaFilterModel::filterAcceptsRow(AdocTreeNode::NodeType nodeType) const
{
    return (AdocTreeNode::isContainer(nodeType) ||
            nodeType == AdocTreeNode::SeqDnaType ||
            nodeType == AdocTreeNode::SubseqDnaType);
}

