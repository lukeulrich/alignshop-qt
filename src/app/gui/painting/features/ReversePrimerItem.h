/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REVERSEPRIMERITEM_H
#define REVERSEPRIMERITEM_H

#include "AbstractPrimerItem.h"

class ReversePrimerItem : public AbstractPrimerItem
{
    Q_OBJECT

public:
    ReversePrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // REVERSEPRIMERITEM_H
