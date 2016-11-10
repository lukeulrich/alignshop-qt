/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocAminoFilterModel.h"

#include <QtGui/QHeaderView>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Trivial constructor
  */
AdocAminoFilterModel::AdocAminoFilterModel(QObject *parent) : AdocTypeFilterModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &] - unused
  * @returns int
  */
int AdocAminoFilterModel::columnCount(const QModelIndex & /* parent */) const
{
    return 9;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocAminoFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case constants::kColumnAminoName:
            return "Name";
        case constants::kColumnAminoParent:
            return "Parent";
        case constants::kColumnAminoSource:
            return "Source";
        case constants::kColumnAminoProtein:
            return "Protein";
        case constants::kColumnAminoStart:
            return "Start";
        case constants::kColumnAminoStop:
            return "Stop";
        case constants::kColumnAminoSequence:
            return "Sequence";
        case constants::kColumnAminoNotes:
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
void AdocAminoFilterModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocAminoFilterModel::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    AdocTypeFilterModel::tweakHorizontalHeader(header);

    // Specify sizes of columns
    header->setResizeMode(constants::kColumnAminoStart, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnAminoStop, QHeaderView::ResizeToContents);
    header->setResizeMode(constants::kColumnAminoSequence, QHeaderView::Stretch);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param nodeType [AdocTreeNode::NodeType]
  * @returns bool
  */
bool AdocAminoFilterModel::filterAcceptsRow(AdocTreeNode::NodeType nodeType) const
{
    return (AdocTreeNode::isContainer(nodeType)
            || nodeType == AdocTreeNode::SeqAminoType
            || nodeType == AdocTreeNode::SubseqAminoType);
}
