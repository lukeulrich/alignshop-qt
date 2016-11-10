/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSASIDEWIDGET_H
#define ABSTRACTMSASIDEWIDGET_H

#include <QtGui/QWidget>

#include "../../core/global.h"

class AbstractMsaView;

class AbstractMsaSideWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbstractMsaSideWidget(QWidget *parent = nullptr);
    AbstractMsaSideWidget(AbstractMsaView *msaView, QWidget *parent = nullptr);

    Qt::Alignment alignment() const;
    int horizontalPadding() const;
    void setAlignment(const Qt::Alignment alignment);   // Default align left
    void setHorizontalPadding(const int horizontalPadding);
    void setMsaView(AbstractMsaView *msaView);

protected:
    virtual void paintEvent(QPaintEvent *paintEvent);
    virtual void paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow);

    AbstractMsaView *msaView_;

protected Q_SLOTS:
    void resizeFont();

private:
    Qt::Alignment alignment_;
    int horizontalPadding_;
};

#endif // ABSTRACTMSASIDEWIDGET_H
