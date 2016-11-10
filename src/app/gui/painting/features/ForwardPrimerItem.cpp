/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "ForwardPrimerItem.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ForwardPrimerItem::ForwardPrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem)
    : AbstractPrimerItem(primerId, name, location, parentItem)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void ForwardPrimerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */)
{
    QPen pen = arrowPen();
    pen.setColor(QColor(0, 196, 0));
    painter->setPen(pen);
    QLineF line = midLine();
    painter->drawLine(line);

    double legLength = arrowLegLength();
    QPointF arrowPoints[3] = {
        QPointF(line.x2() - legLength, 0),
        line.p2(),
        QPointF(line.x2() - legLength, height())
    };
    painter->drawPolyline(arrowPoints, 3);
}
