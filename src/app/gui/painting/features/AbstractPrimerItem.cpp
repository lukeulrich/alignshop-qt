/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPen>

#include "AbstractPrimerItem.h"
#include "FeatureTypes.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
AbstractPrimerItem::AbstractPrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem)
    : AbstractRangeFeatureItem(constants::Features::kPrimerFeatureType, primerId, location, parentItem),
      primerId_(primerId)
{
    setName(name);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
double AbstractPrimerItem::arrowLegLength() const
{
    return halfHeight();
}

QPen AbstractPrimerItem::arrowPen() const
{
    QPen pen;
    pen.setWidth(2);
    return pen;
}

QLineF AbstractPrimerItem::midLine() const
{
    QPointF midLineLeft = QPointF(0, halfHeight());
    QPointF midLineRight = QPointF(width(), midLineLeft.y());
    return QLineF(midLineLeft, midLineRight);
}

int AbstractPrimerItem::primerId() const
{
    return primerId_;
}

QString AbstractPrimerItem::name() const
{
    return name_;
}

double AbstractPrimerItem::height() const
{
    return 7.;
}

void AbstractPrimerItem::setName(const QString &name)
{
    name_ = name;
    setToolTip(name_);
}
