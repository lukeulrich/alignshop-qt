/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQCOLUMNADAPTER_H
#define AMINOSEQCOLUMNADAPTER_H

#include "AbstractColumnAdapter.h"
#include "../../../core/global.h"

class AminoSeqColumnAdapter : public AbstractColumnAdapter
{
    Q_OBJECT

public:
    enum Columns
    {
        eIdColumn = 0,
        eTypeColumn,
        eNameColumn,
        eSourceColumn,
        eStartColumn,
        eStopColumn,
        eLengthColumn,
        eDescriptionColumn,
        eNotesColumn,
        eSequenceColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    AminoSeqColumnAdapter(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int columnCount() const;
    Qt::ItemFlags flags(int column) const;
    QVariant data(const IEntitySPtr &entity, int column, int role = Qt::DisplayRole) const;
    QVariant headerData(int column) const;


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    // setData() which calls this method checks if the data is different and if it is then calls this method. Thus, it
    // is not necessary to check for a different value in the switch statement.
    bool setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value);
};

#endif // AMINOSEQCOLUMNADAPTER_H
