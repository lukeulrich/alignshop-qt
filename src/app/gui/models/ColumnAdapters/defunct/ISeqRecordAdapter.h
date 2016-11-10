/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ISEQRECORDADAPTER_H
#define ISEQRECORDADAPTER_H

#include "ICommonColumnAdapter.h"
#include "../Entities/ISeqRecord.h"

class ISeqRecordAdapter : public ICommonColumnAdapter
{
    Q_OBJECT

public:
    enum Columns
    {
        eIdColumn = 0,
        eNameColumn,
        eDescriptionColumn,

        eMaxColumn
    };

    ISeqRecordAdapter(QObject *parent = 0) : ICommonColumnAdapter(parent)
    {
    }

    virtual int commonColumnCount() const
    {
        return eMaxColumn;
    }

    QVariant commonData(const ISeqRecord *seqRecord, int column) const
    {
        ASSERT(column >= 0 && column < eMaxColumn);
        ASSERT(seqRecord);

        switch(column)
        {
        case eIdColumn:
            return seqRecord->id();
        case eNameColumn:
            return seqRecord->name_;
        case eDescriptionColumn:
            return seqRecord->description_;

        default:
            return QVariant();
        }
    }

    bool setCommonData(ISeqRecord *seqRecord, int column, const QVariant &value) const
    {
        ASSERT(column >= 0 && column < eMaxColumn);
        ASSERT(seqRecord);

        switch(column)
        {
        case eNameColumn:
            seqRecord->name_ = value.toString();
            emit dataChanged(column);
            return true;
        case eDescriptionColumn:
            seqRecord->description_ = value.toString();
            emit dataChanged(column);
            return true;

        default:
            return false;
        }
    }
};

#endif // ISEQRECORDADAPTER_H
