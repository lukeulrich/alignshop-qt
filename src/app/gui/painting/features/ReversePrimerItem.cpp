/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "ReversePrimerItem.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ReversePrimerItem::ReversePrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem)
    : AbstractPrimerItem(primerId, name, location, parentItem)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void ReversePrimerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */)
{
    QPen pen = arrowPen();
    pen.setColor(QColor(0, 0, 196));
    painter->setPen(pen);
    QLineF line = midLine();
    painter->drawLine(line);

    double legLength = arrowLegLength();
    QPointF arrowPoints[3] = {
        QPointF(legLength, 0),
        line.p1(),
        QPointF(legLength, height())
    };
    painter->drawPolyline(arrowPoints, 3);
}
