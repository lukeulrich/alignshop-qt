/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QRegExp>

#include "RestrictionEnzymeTableModel.h"
#include "RebaseParser.h"

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
    return eNumberOfColumns;
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
        case eNameColumn:
            return restrictionEnzymes_.at(index.row()).name();
        case eRecognitionSiteColumn:
            return restrictionEnzymes_.at(index.row()).recognitionSite().asByteArray();
        //case kFirstCutColumn:
        //    return restrictionEnzymes_.at(index.row()).cut_pos1_;
        //case kSecondCutColumn:
        //    return restrictionEnzymes_.at(index.row()).cut_pos2_;
        case eBluntStickyColumn:
            return (restrictionEnzymes_.at(index.row()).isBlunt()) ? "Blunt" : "Sticky";
        default:
            return QVariant();
        }
    }

    return QVariant();
}

/**
  * @param name [const QString &]
  * @returns int
  */
int RestrictionEnzymeTableModel::findRowWithName(const QString &name) const
{
    for (int i=0, z=restrictionEnzymes_.size(); i<z; ++i)
        if (name.compare(restrictionEnzymes_.at(i).name(), Qt::CaseInsensitive) == 0)
            return i;

    return -1;
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
        case eNameColumn:
            return "Name";
        case eRecognitionSiteColumn:
            return "Recognition site";
        //case kFirstCutColumn:
        //    return "First cut";
        //case kSecondCutColumn:
        //    return "Second cut";
        case eBluntStickyColumn:
            return "Blunt or sticky";
        default:
            return QVariant();
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return QString("%1.").arg(section + 1);

    return QVariant();
}

/**
  * @param index [const QModelIndex &]
  * @returns RestrictionEnzyme
  */
RestrictionEnzyme RestrictionEnzymeTableModel::restrictionEnzymeAt(const QModelIndex &index) const
{
    if (!index.isValid() || index.model() != this)
        return RestrictionEnzyme();

    return restrictionEnzymes_.at(index.row());
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
    beginResetModel();
    RebaseParser rebaseParser;
    // Use the parser to update the list of enzymes
    restrictionEnzymes_ = rebaseParser.parseRebaseFile(file);
    removeInvalidEnzymes();
    endResetModel();
}

/**
  */
void RestrictionEnzymeTableModel::removeInvalidEnzymes()
{
    QRegExp invalidCharacters("[^ACGT]");

    for (int i=restrictionEnzymes_.size() - 1; i>= 0; i--)
        if (QString(restrictionEnzymes_.at(i).recognitionSite().asByteArray()).contains(invalidCharacters))
            restrictionEnzymes_.remove(i, 1);
}
