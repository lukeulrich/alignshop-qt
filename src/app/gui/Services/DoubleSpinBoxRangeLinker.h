/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DOUBLESPINBOXRANGELINKER_H
#define DOUBLESPINBOXRANGELINKER_H

#include <QtCore/QObject>
#include "../../core/global.h"

class QDoubleSpinBox;

/**
  * DoubleSpinBoxRangeLinker links two distinct QDoubleSpinBoxes such that together they define a well-defined floating
  * point range.
  */
class DoubleSpinBoxRangeLinker : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DoubleSpinBoxRangeLinker(QDoubleSpinBox *startSpinBox, QDoubleSpinBox *stopSpinBox, QObject *parent = nullptr);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onStartSpinBoxValueChanged(double newValue);
    void onStopSpinBoxValueChanged(double newValue);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QDoubleSpinBox *startSpinBox_;
    QDoubleSpinBox *stopSpinBox_;
};

#endif // DOUBLESPINBOXRANGELINKER_H
