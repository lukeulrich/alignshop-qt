/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PFAM_H
#define PFAM_H

#include "IDomain.h"

class Pfam : public IDomain
{
public:
    Pfam(ClosedIntRange location, const QString &name, double score, double evalue)
        : IDomain(location, name), score_(score), evalue_(evalue)
    {
    }

    const double score_;
    const double evalue_;
};

#endif // PFAM_H
