/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IDBIOSTRING_H
#define IDBIOSTRING_H

#include "../BioString.h"

struct IdBioString
{
    int id_;
    BioString bioString_;

    IdBioString()
        : id_(0)
    {
    }

    IdBioString(const int id, const BioString &bioString)
        : id_(id), bioString_(bioString)
    {
    }
};

#endif // IDBIOSTRING_H
