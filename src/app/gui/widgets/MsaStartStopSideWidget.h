/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSASTARTSTOPSIDEWIDGET_H
#define MSASTARTSTOPSIDEWIDGET_H

#include <QtCore/QString>

#include "AbstractMsaSideWidget.h"

class ObservableMsa;

class MsaStartStopSideWidget : public AbstractMsaSideWidget
{
    Q_OBJECT

public:
    enum PositionType
    {
        StartType = 0,
        StopType,
        InverseStartType,
        InverseStopType
    };

    explicit MsaStartStopSideWidget(QWidget *parent = nullptr);
    MsaStartStopSideWidget(AbstractMsaView *msaView, const PositionType positionType, QWidget *parent = nullptr);

    PositionType positionType() const;
    void setPositionType(const PositionType positionType);

Q_SIGNALS:
    void positionTypeChanged();

public Q_SLOTS:
    void updateWidth();

protected:
    virtual void paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow);

private:
    QString longestStringForType(const ObservableMsa *msa, const PositionType positionType) const;

    PositionType positionType_;
};

#endif // MSASTARTSTOPSIDEWIDGET_H
