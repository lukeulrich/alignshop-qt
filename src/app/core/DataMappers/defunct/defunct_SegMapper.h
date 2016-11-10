/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEGMAPPER_H
#define SEGMAPPER_H

#include "IValueObjectMapper.h"
#include "../ValueObjects/Seg.h"
#include "../PODs/SegPod.h"

class SegMapper : public IValueObjectMapper<Seg, SegPod>
{
public:
    Seg mapToObject(const SegPod &segPod) const
    {
        return Seg(segPod.id_, ClosedIntRange(segPod.start_, segPod.stop_));
    }

    SegPod mapFromObject(const Seg &seg) const
    {
        return SegPod(seg.id_, seg.begin(), seg.end());
    }
};


#endif // SEGMAPPER_H
