/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COIL_H
#define COIL_H

#include <QtCore/QVector>

#include "IFeature.h"

class Coil : public IFeature
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    Coil()
        : IFeature()
    {
    }

    explicit Coil(ClosedIntRange location)
        : IFeature(location)
    {
    }

    Coil(int id, ClosedIntRange location)
        : IFeature(id, location)
    {
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int type() const
    {
        return CoilType;
    }
};

#endif // COIL_H
