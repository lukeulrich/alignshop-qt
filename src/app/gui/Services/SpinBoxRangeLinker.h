/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SPINBOXRANGELINKER_H
#define SPINBOXRANGELINKER_H

#include <QtCore/QObject>
#include "../../core/global.h"

class QSpinBox;

/**
  * SpinBoxRangeLinker links two distinct QSpinBoxes such that together they define a well-defined integral range.
  *
  * Note: when configuring the initial values, be sure to set the larger one first or setting the value of the start
  * value will not work as expected.
  */
class SpinBoxRangeLinker : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SpinBoxRangeLinker(QSpinBox *startSpinBox, QSpinBox *stopSpinBox, QObject *parent = nullptr);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onStartSpinBoxValueChanged(int newValue);
    void onStopSpinBoxValueChanged(int newValue);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QSpinBox *startSpinBox_;
    QSpinBox *stopSpinBox_;
};

#endif // SPINBOXRANGELINKER_H
