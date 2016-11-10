/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ANTITOGGLELINEITEM_H
#define ANTITOGGLELINEITEM_H

#include <QtGui/QGraphicsLineItem>
#include <QtGui/QPainter>
#include "../../../core/global.h"
#include "../../../core/enums.h"

/**
  * AntiToggleLineItem functions identical to QGraphicsLineItem except that it provides for toggling anti-aliasing per
  * line or use the default defined antialias render hint.
  *
  * By default, it does not use antialiasing.
  */
class AntiToggleLineItem : public QGraphicsLineItem
{
public:
    explicit AntiToggleLineItem(QGraphicsItem *parent = nullptr) :
        QGraphicsLineItem(parent),
        antiAlias_(eFalse)
    {
    }

    AntiToggleLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = nullptr)
        : QGraphicsLineItem(x1, y1, x2, y2, parent),
          antiAlias_(eFalse)
    {
    }

    void setAntiAlias(TriBool antialias)
    {
        if (antiAlias_ == antialias)
            return;

        antiAlias_ = antialias;
        update();
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        if (antiAlias_ == eTrue)
            painter->setRenderHint(QPainter::Antialiasing, true);
        else if (antiAlias_ == eFalse)
            painter->setRenderHint(QPainter::Antialiasing, false);

        QGraphicsLineItem::paint(painter, option, widget);
    }

private:
    TriBool antiAlias_;
};

#endif // ANTITOGGLELINEITEM_H
