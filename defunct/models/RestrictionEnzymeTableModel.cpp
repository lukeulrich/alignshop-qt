/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "RestrictionEnzymeTableModel.h"
#include <QtCore>


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
RestrictionEnzymeTableModel::RestrictionEnzymeTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * Because there are only three columns, we always return three.
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int RestrictionEnzymeTableModel::columnCount(const QModelIndex & /* parent */) const
{
    return 3;
}

/**
  * After ensuring that we have a valid index and it indexes into a valid row within restrictionEnzymes_,
  * simply return the value for that column or an empty QVariant if the column is not valid.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant RestrictionEnzymeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // Ensure that the requested row is within the bounds of our internal list of restriction enzymes
    if (index.row() < 0 || index.row() > restrictionEnzymes_.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case kNameColumn:
            return restrictionEnzymes_.at(index.row()).name_;
        case kRecognitionSiteColumn:
            return restrictionEnzymes_.at(index.row()).recognitionSequence_;
        //case kFirstCutColumn:
        //    return restrictionEnzymes_.at(index.row()).cut_pos1_;
        //case kSecondCutColumn:
        //    return restrictionEnzymes_.at(index.row()).cut_pos2_;
        case kBluntStickyColumn:
            return restrictionEnzymes_.at(index.row()).bluntOrSticky_;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

/**
  * This function returns the label data for both the horizontal and vertical headers. For the horizontal headers,
  * we simply return the desired user-friendly label. The vertical header simply reflects the row number.
  *
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  */
QVariant RestrictionEnzymeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case kNameColumn:
            return "Name";
        case kRecognitionSiteColumn:
            return "Recognition site";
        //case kFirstCutColumn:
        //    return "First cut";
        //case kSecondCutColumn:
        //    return "Second cut";
        case kBluntStickyColumn:
            return "Blunt or sticky";
        default:
            return QVariant();
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;

    return QVariant();
}

/**
  * The number of rows always equals the number of elements within the restrictionEnzymes_ list.
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int RestrictionEnzymeTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return restrictionEnzymes_.size();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * At the current time, file is unused because the RebaseParser class has a hard-coded reference to the file
  * containing the relevant restriction enzyme data. This will eventually be parameterized.
  *
  * @param file [const QString &]
  */
void RestrictionEnzymeTableModel::loadRebaseFile(const QString &file)
{
    Q_UNUSED(file)

    // Use the parser to update the list of enzymes
    restrictionEnzymes_ = rebaseParser_.parseRebaseFile();

    removeInvalidEnzymes();

    // Tell all attached views that the underlying data has been updated and that they should refresh their
    // display.
    reset();
}

void RestrictionEnzymeTableModel::removeInvalidEnzymes()
{
    QRegExp invalidCharacters("[^ACGT]");
    int i = 0;

    while (i < restrictionEnzymes_.length())
    {
        if (restrictionEnzymes_[i].recognitionSequence_.contains(invalidCharacters))
        {
            restrictionEnzymes_.removeAt(i);
        }
        else
        {
            ++i;
        }
    }
}
