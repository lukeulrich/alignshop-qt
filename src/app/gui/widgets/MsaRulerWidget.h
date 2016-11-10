/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSARULERWIDGET_H
#define MSARULERWIDGET_H

#include <QtGui/QWidget>
#include "../../core/global.h"

class QPaintEvent;

class AbstractMsaView;

class MsaRulerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MsaRulerWidget(QWidget *parent = nullptr);
    MsaRulerWidget(AbstractMsaView *msaView, QWidget *parent = nullptr);

    void setMsaView(AbstractMsaView *msaView);
    void setUnitsPerLabel(const int unitsPerlabel);
    void setBottomVerticalPadding(const int bottomVerticalPadding);
    int unitsPerLabel() const;
    int bottomVerticalPadding() const;

public Q_SLOTS:
    void updateHeight();

protected:
    virtual void mouseMoveEvent(QMouseEvent *mouseEvent);
    virtual void paintEvent(QPaintEvent *paintEvent);

private Q_SLOTS:
    void onViewportMouseMoved(const QPoint &point);
    void resizeFont();

private:
    int tickHeight() const;

    AbstractMsaView *msaView_;
    int bottomVerticalPadding_;
    int unitsPerLabel_;
};

#endif // MSARULERWIDGET_H
