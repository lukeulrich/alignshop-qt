/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef PARSEDBIOSTRINGTABLEMODEL_H
#define PARSEDBIOSTRINGTABLEMODEL_H

#include <QtCore/QAbstractTableModel>

#include "../BioString.h"
#include "../ParsedBioString.h"

/**
  * ParsedBioStringTableModel provides a concrete model for managing a list of ParsedBioString objects.
  *
  * The sequence import process provides the motivating need for this model. After parsing all sequences contained in
  * a sequence file, it is necessary to display them to the user for selecting which ones to import and whether
  * they are valid/invalid.
  *
  * There are three columns:
  * 1. checkbox indicating whether this parsed bio string has been selected for import and its arbitrary label
  * 2. sequence
  * 3. valid
  *
  * Constraints:
  * o An invalid ParsedBioString cannot be checked
  * o A valid ParsedBioString may or may not be checked
  * o Valid sequences are by default checked
  */
class ParsedBioStringTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //! Return the number of columns for the children under parent
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    //! Return the data corresponding to index for the given role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;            //!< Return the flags for index
    //! Returns the data for the given role and section in the header with the specified orientation.
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;  //!< Return the number of rows in this model
    //! Sets the role data for the item at index to value
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void clear();                                                   //!< Clear all parsedBioStrings and reset the model
    QList<ParsedBioString> parsedBioStrings() const;                //!< Return the list of parsedBioStrings
    //!< Set the model data to parsedBioStrings
    void setParsedBioStrings(const QList<ParsedBioString> &parsedBioStrings);

    // ------------------------------------------------------------------------------------------------
    // Static constants for facilatating column access and referral
    static const int kLabelColumn = 0;
    static const int kSequenceColumn = 1;
    static const int kValidColumn = 2;

signals:
    void checkedChange(const QModelIndex &index);                   //!< Emitted when a checkbox has changed its checked status

private:
    QList<ParsedBioString> parsedBioStrings_;   //!< List of all Parsed BioStrings currently
};

#endif // PARSEDBIOSTRINGTABLEMODEL_H
