/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "ParsedBioStringTableModel.h"

#include <QtGui/QFont>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * Returns 3 to indicate there are three columns in this model:
  * 1. checkbox and label
  * 2. sequence
  * 3. valid
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int ParsedBioStringTableModel::columnCount(const QModelIndex &/* parent */) const
{
    return 3;
}

/**
  * Fetches the data for the parsedBioString at index.row() and the corresponding ParsedBioString data
  * associated with index.column() for the given role.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant ParsedBioStringTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() > parsedBioStrings_.size())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case kLabelColumn:
            return parsedBioStrings_.at(index.row()).header_;
        case kSequenceColumn:
            return parsedBioStrings_.at(index.row()).bioString_.sequence();
        case kValidColumn:
            return (parsedBioStrings_.at(index.row()).valid_) ? "Yes" : "No";
        }
    }
    else if(role == Qt::CheckStateRole && index.column() == kLabelColumn)
    {
        // Return the value for the checkbox
        if (parsedBioStrings_.at(index.row()).checked_)
            return Qt::Checked;
        else
            return Qt::Unchecked;
    }
    else if (role == Qt::TextAlignmentRole)
    {
        // Horizontally center the text for the valid column
        if (index.column() == kValidColumn)
            return Qt::AlignHCenter;
    }
    else if (role == Qt::FontRole)
    {
        // Assign a monospace font for the sequence data
        if (index.column() == kSequenceColumn)
            return QFont("monospace");
    }
    else if (role == Qt::ForegroundRole)
    {
        if (index.column() == kValidColumn)
        {
            // Highlight valid sequences in green and invalid sequences in red
            if (parsedBioStrings_.at(index.row()).valid_)
                return Qt::darkGreen;
            else
                return Qt::darkRed;
        }
    }

    return QVariant();
}

/**
  * Specific flag settings:
  * o Label column = editable and if corresponding ParsedBioString is valid, then it is
  *   checkable, otherwise, it is not enabled.
  *
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags ParsedBioStringTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.isValid())
    {
        switch(index.column())
        {
        case kLabelColumn:
            flags |= Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
            if (!parsedBioStrings_.at(index.row()).valid_)
                flags &= ~Qt::ItemIsEnabled;
            break;
        }
    }

    return flags;
}

/**
  * Provide header data associated with the three columns and row numbers.
  *
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant ParsedBioStringTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case kLabelColumn:
            return "Label";
        case kSequenceColumn:
            return "Sequence";
        case kValidColumn:
            return "Valid";
        }
    }
    else // Row headers. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;

    return QVariant();
}

/**
  * Returns the number of items within the parsedBioStrings list.
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int ParsedBioStringTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return parsedBioStrings_.size();
}

/**
  * Because the model is editable, this function provides the actual mechanism for editing the label and checkbox
  * data per ParsedBioString.
  *
  * In the flags method, the valid column is not editable via the user; however, it is possible to change the valid
  * status programmatically via this function as appropriate (e.g. after validation with a BioStringValidator).
  *
  * A similar concept is with the label column. When the row is invalid, it is not possible to check the checkbox (yet
  * it is valid to uncheck it). Moreover, it is not possible to directly edit the label from the UI, but it is possible
  * programmatically to accomplish this.
  *
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool ParsedBioStringTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::EditRole)
    {
        if (index.column() == kLabelColumn)
        {
            parsedBioStrings_[index.row()].header_ = value.toString();
            emit dataChanged(index, index);
            return true;
        }
        else if (index.column() == kValidColumn)
        {
            // Only update the value if it is different
            bool new_valid = value.toBool();
            bool old_valid = parsedBioStrings_[index.row()].valid_;
            if (new_valid != old_valid)
            {
                parsedBioStrings_[index.row()].valid_ = new_valid;
                emit dataChanged(index, index);

                // A checkbox may only be checked if it is valid, thus, we set the checkbox state based on the
                // value of new_valid
                setData(this->index(index.row(), 0), new_valid, Qt::CheckStateRole);
            }

            return true;
        }
    }
    else if (role == Qt::CheckStateRole
             && index.column() == 0
             && (parsedBioStrings_[index.row()].valid_
                 || value.toBool() == false))
    {
        // Only update the value if it is different
        if (parsedBioStrings_[index.row()].checked_ != value.toBool())
        {
            parsedBioStrings_[index.row()].checked_ = value.toBool();
            emit dataChanged(index, index);
            emit checkedChange(index);
        }

        // This operation will be successful every time
        return true;
    }

    return false;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void ParsedBioStringTableModel::clear()
{
    parsedBioStrings_.clear();
    reset();
}

/**
  * @returns QList<ParsedBioString>
  */
QList<ParsedBioString> ParsedBioStringTableModel::parsedBioStrings() const
{
    return parsedBioStrings_;
}

/**
  * If any invalid biostrings are checked, set their checked state to false.
  *
  * @param parsedBioStrings [const QList<ParsedBioString>]
  */
void ParsedBioStringTableModel::setParsedBioStrings(const QList<ParsedBioString> &parsedBioStrings)
{
    parsedBioStrings_ = parsedBioStrings;
    for (int i=0, z=parsedBioStrings_.size(); i<z; ++i)
    {
        if (parsedBioStrings_.at(i).valid_ == false
            && parsedBioStrings.at(i).checked_)
        {
            parsedBioStrings_[i].checked_ = false;
        }
    }
    reset();
}
