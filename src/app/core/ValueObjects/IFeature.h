/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IFEATURE_H
#define IFEATURE_H

#include "ValueObject.h"
#include "../util/ClosedIntRange.h"
#include "../macros.h"

class IFeature : public ValueObject
{
public:
    enum FeatureTypes
    {
        SegType = 0,
        CoilType,
        PfamType   // Or domain type?
    };

    // ------------------------------------------------------------------------------------------------
    // Constructors
    IFeature()
    {
    }

    IFeature(ClosedIntRange location)
        : ValueObject(0),
          location_(location)
    {
        ASSERT(location.begin_ > 0 && location.end_ >= location.begin_);
    }

    IFeature(int id, ClosedIntRange location)
        : ValueObject(id),
          location_(location)
    {
        ASSERT(location.begin_ > 0 && location.end_ >= location.begin_);
    }


    // ------------------------------------------------------------------------------------------------
    // Operators
    virtual bool operator==(const IFeature &other) const
    {
        return location_ == other.location_ &&
               type() == other.type();
    }

    bool operator!=(const IFeature &other) const
    {
        return !operator==(other);
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int begin() const
    {
        return location_.begin_;
    }

    int end() const
    {
        return location_.end_;
    }

    ClosedIntRange location() const
    {
        return location_;
    }

    virtual int type() const = 0;


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    ClosedIntRange location_;
};

#endif // IFEATURE_H
