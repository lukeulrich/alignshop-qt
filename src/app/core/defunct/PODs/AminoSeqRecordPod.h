/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQRECORDPOD_H
#define AMINOSEQRECORDPOD_H

#include <QtCore/QString>

struct AminoSeqRecordPod
{
    int id_;
    int astringId_;
    int start_;
    int stop_;
    QString name_;
    QString description_;

    AminoSeqRecordPod(int id = 0, int astringId = 0, int start = 0, int stop = 0, const QString &name = QString(), const QString &description = QString())
        : id_(id), astringId_(astringId), start_(start), stop_(stop), name_(name), description_(description)
    {}
};

Q_DECLARE_TYPEINFO(AminoSeqRecordPod, Q_MOVABLE_TYPE);

#endif // AMINOSEQRECORDPOD_H
