/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RESTRICTIONENZYMETABLEMODEL_H
#define RESTRICTIONENZYMETABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QVector>

#include "RestrictionEnzyme.h"

/**
  * RestrictionEnzymeTableModel provides a read-only interface to a table of restriction enzyme data.
  *
  * After initialization, client classes must call loadRebaseFile to populate the internal list of
  * restriction enzymes, which is parsed using an instance of RebaseParser.
  *
  * The table structure closely follows the fields in the restrictionEnzyme structure and are as follows:
  * 1. Name
  * 2. Recognition site
  * 3. First cut position
  * 4. Second cut position
  * 5. Blunt or sticky
  */
class RestrictionEnzymeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        eNameColumn = 0,
        eRecognitionSiteColumn,
        eBluntStickyColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    RestrictionEnzymeTableModel(QObject *parent = 0);   //!< Trivial constructor

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //!< Returns the number of columns for the children under parent
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    //!< Returns the data stored under the given role for the item referred to by index
    QVariant data(const QModelIndex &index, int role) const;
    //!< Returns the data for the given role and section in the header with the specified orientation
    int findRowWithName(const QString &name) const;         //!< Returns the integer row that has name; or -1 if it is not found
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    RestrictionEnzyme restrictionEnzymeAt(const QModelIndex &index) const;
    //!< Retruns the number of rows under the given parent
    int rowCount(const QModelIndex &parent = QModelIndex()) const;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void loadRebaseFile(const QString &file);           //!< Resets the model with the restriction enzyme data contained in file
    void removeInvalidEnzymes();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QVector<RestrictionEnzyme> restrictionEnzymes_;       //!< Internal list of restriction enzymes
};

#endif // RESTRICTIONENZYMETABLEMODEL_H
