/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FORWARDPRIMERITEM_H
#define FORWARDPRIMERITEM_H

#include "AbstractPrimerItem.h"

class ForwardPrimerItem : public AbstractPrimerItem
{
    Q_OBJECT

public:
    ForwardPrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // FORWARDPRIMERITEM_H
