/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTRANGEFEATUREITEM_H
#define ABSTRACTRANGEFEATUREITEM_H

#include "AbstractFeaureItem.h"
#include "../../../core/util/ClosedIntRange.h"

class AbstractRangeFeatureItem : public AbstractFeatureItem
{
    Q_OBJECT

public:
    ClosedIntRange location() const
    {
        return location_;
    }

protected:
    AbstractRangeFeatureItem(const int type, const int id, const ClosedIntRange &location, QGraphicsItem *parentItem = nullptr)
        : AbstractFeatureItem(type, id, location.length(), parentItem),
          location_(location)
    {
        setX(leftPositionFromUnit(location_.begin_));
    }

    void onPixelsPerUnitChanged()
    {
        setX(leftPositionFromUnit(location_.begin_));
    }


private:
    ClosedIntRange location_;
};

#endif // ABSTRACTRANGEFEATUREITEM_H
