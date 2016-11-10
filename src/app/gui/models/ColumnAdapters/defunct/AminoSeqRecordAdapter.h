/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQRECORDADAPTER_H
#define AMINOSEQRECORDADAPTER_H

#include "IColumnAdapter.h"
#include "../../../core/Entities/AminoSeq.h"

class AminoSeqRecordAdapter : public IColumnAdapter
{
    Q_OBJECT

public:
    enum Columns
    {
        eCoilsColumn = 0,

        eMaxColumn
    };

    AminoSeqRecordAdapter(QObject *parent = 0) : ISeqRecordAdapter(parent)
    {
    }

    int uniqueColumnCount()
    {
        return eMaxColumn;
    }

    QVariant uniqueData(const AminoSeqRecord *aminoSeqRecord, int column) const
    {
        ASSERT(column >= 0 && column <= eMaxColumn);
        ASSERT(aminoSeqRecord);

        switch (column)
        {
        case eCoilsColumn:
            return aminoSeqRecord->anonSeq()->coils().size();

        default:
            return QVariant();
        }
    }

    bool setUniqueData(AminoSeqRecord *aminoSeqRecord, int column, const QVariant &value) const
    {
        ASSERT(column >= 0 && column <= eMaxColumn);
        ASSERT(aminoSeqRecord);

        switch (column)
        {

        default:
            return false;
        }
    }
};

#endif // AMINOSEQRECORDADAPTER_H
