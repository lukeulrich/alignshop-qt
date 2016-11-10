/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEG_H
#define SEG_H

#include "IFeature.h"

class Seg : public IFeature
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    Seg()
        : IFeature()
    {
    }

    explicit Seg(ClosedIntRange location)
        : IFeature(location)
    {
    }

    Seg(int id, ClosedIntRange location)
        : IFeature(id, location)
    {
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int type() const
    {
        return SegType;
    }
};

#endif // SEG_H
