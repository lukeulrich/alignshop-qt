/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COILMAPPER_H
#define COILMAPPER_H

#include "IValueObjectMapper.h"
#include "../ValueObjects/Coil.h"
#include "../PODs/CoilPod.h"

class CoilMapper : public IValueObjectMapper<Coil, CoilPod>
{
public:
    Coil mapToObject(const CoilPod &coilPod) const
    {
        return Coil(coilPod.id_, ClosedIntRange(coilPod.start_, coilPod.stop_));
    }

    CoilPod mapFromObject(const Coil &coil) const
    {
        return CoilPod(coil.id_, coil.begin(), coil.end());
    }
};

#endif // COILMAPPER_H
