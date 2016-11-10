/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INFOUNITITEM_H
#define INFOUNITITEM_H

#include <QtGui/QGraphicsPathItem>
#include <QtGui/QPainterPath>
#include "../../../core/PODs/InfoUnit.h"

#include <QtDebug>

class InfoUnitItem : public QGraphicsPathItem
{
public:
    InfoUnitItem(const InfoUnit &infoUnit, const QPainterPath &path, QGraphicsItem *parent)
        : QGraphicsPathItem(path, parent), infoUnit_(infoUnit)
    {
    }

    Qt::MouseButtons acceptedMouseButtons() const
    {
        return Qt::LeftButton;
    }

    void mousePressEvent(QGraphicsSceneMouseEvent * /* event */)
    {
        qDebug() << infoUnit_.ch_ << infoUnit_.percent_ << infoUnit_.info_;
    }

    InfoUnit infoUnit_;
};

#endif // INFOUNITITEM_H
