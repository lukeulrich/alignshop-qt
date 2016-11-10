/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef HSPPOD_H
#define HSPPOD_H

#include "../BioString.h"
#include "../util/ClosedIntRange.h"

struct HspPod
{
    ClosedIntRange queryRange_;
    int queryFrame_;
    BioString queryAlignment_;

    QByteArray midline_;

    ClosedIntRange subjectRange_;
    int subjectFrame_;
    BioString subjectAlignment_;

    double evalue_;
    double bitScore_;
    int rawScore_;

    int length_;
    int identities_;
    int positives_;
    int gaps_;
};

#endif // HSPPOD_H
