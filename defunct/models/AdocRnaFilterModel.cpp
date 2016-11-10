/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocRnaFilterModel.h"

#include <QtGui/QHeaderView>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Trivial constructor
  */
AdocRnaFilterModel::AdocRnaFilterModel(QObject *parent) : AdocTypeFilterModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &] - unused
  * @returns int
  */
int AdocRnaFilterModel::columnCount(const QModelIndex & /* parent */) const
{
    return 9;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocRnaFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case constants::kColumnRnaName:
            return "Name";
        case constants::kColumnRnaParent:
            return "Parent";
        case constants::kColumnRnaSource:
            return "Source";
        case constants::kColumnRnaGene:
            return "Gene";
        case constants::kColumnRnaStart:
            return "Start";
        case constants::kColumnRnaStop:
            return "Stop";
        case constants::kColumnRnaSequence:
            return "Sequence";
        case constants::kColumnRnaNotes:
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
void AdocRnaFilterModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocRnaFilterModel::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    AdocTypeFilterModel::tweakHorizontalHeader(header);

    header->setResizeMode(constants::kColumnRnaStart, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnRnaStop, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnRnaSequence, QHeaderView::Stretch);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param nodeType [AdocTreeNode::NodeType]
  * @returns bool
  */
bool AdocRnaFilterModel::filterAcceptsRow(AdocTreeNode::NodeType nodeType) const
{
    return (AdocTreeNode::isContainer(nodeType) ||
            nodeType == AdocTreeNode::SeqRnaType ||
            nodeType == AdocTreeNode::SubseqRnaType);
}
