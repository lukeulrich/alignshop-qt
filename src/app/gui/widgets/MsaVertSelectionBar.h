/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAVERTSELECTIONBAR_H
#define MSAVERTSELECTIONBAR_H

#include "AbstractMsaSideWidget.h"
#include "../../core/util/ClosedIntRange.h"

class QPainter;

class AbstractMsaView;
class PosiRect;

/**
  * MsaVertSelectionBar displays a small rectangle denoting the currently selected vertical area.
  */
class MsaVertSelectionBar : public AbstractMsaSideWidget
{
    Q_OBJECT

public:
    explicit MsaVertSelectionBar(QWidget *parent = nullptr);
    MsaVertSelectionBar(AbstractMsaView *msaView, QWidget *parent = nullptr);

    void setMsaView(AbstractMsaView *msaView);

protected:
    virtual void paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow);

private Q_SLOTS:
    void onMsaMouseCursorVisibleChanged(bool visible);
    void onMsaSelectionChanged(const PosiRect &currentSelection);

private:
    ClosedIntRange verticalSelection_;
    bool showMouseBar_;
};

#endif // MSAVERTSELECTIONBAR_H
