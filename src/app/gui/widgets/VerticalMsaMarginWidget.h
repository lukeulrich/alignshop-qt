/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef VERTICALMSAMARGINWIDGET_H
#define VERTICALMSAMARGINWIDGET_H

#include <QtCore/QStringList>
#include <QtGui/QAbstractScrollArea>
#include "../../core/global.h"

class VerticalMsaMarginWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    VerticalMsaMarginWidget(QWidget *parent = nullptr)
        : QAbstractScrollArea(parent),
          renderXShift_(0.)
    {
    }

    qreal renderXShift() const
    {
        return renderXShift_;
    }


public Q_SLOTS:
    virtual void updateHeight()
    {
    }

    virtual void updateScrollBarRanges()
    {
    }

    void setRenderXShift(qreal xShift)
    {
        renderXShift_ = xShift;
    }

private:
    qreal renderXShift_;
};

#endif // VERTICALMSAMARGINWIDGET_H
