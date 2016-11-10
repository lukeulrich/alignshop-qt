/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ANONSEQPOD_H
#define ANONSEQPOD_H

#include <QtCore/QByteArray>

struct AnonSeqPod
{
    int id_;
    QByteArray sequence_;
    QByteArray digest_;

    AnonSeqPod(int id = 0, const QByteArray &sequence = QByteArray(), const QByteArray &digest = QByteArray())
        : id_(id), sequence_(sequence), digest_(digest)
    {
    }
};

Q_DECLARE_TYPEINFO(AnonSeqPod, Q_MOVABLE_TYPE);

#endif // ANONSEQPOD_H
