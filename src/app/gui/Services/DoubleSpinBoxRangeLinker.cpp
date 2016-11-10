/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QDoubleSpinBox>

#include "DoubleSpinBoxRangeLinker.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param startSpinBox [QDoubleSpinBox *]
  * @param stopSpinBox [QDoubleSpinBox *]
  * @param parent [QObject *]
  */
DoubleSpinBoxRangeLinker::DoubleSpinBoxRangeLinker(QDoubleSpinBox *startSpinBox, QDoubleSpinBox *stopSpinBox, QObject *parent)
    : QObject(parent),
      startSpinBox_(startSpinBox),
      stopSpinBox_(stopSpinBox)
{
    if (startSpinBox_ == nullptr || stopSpinBox_ == nullptr)
        return;

    ASSERT(startSpinBox_ != stopSpinBox_);

    connect(startSpinBox_, SIGNAL(valueChanged(double)), SLOT(onStartSpinBoxValueChanged(double)));
    connect(stopSpinBox_, SIGNAL(valueChanged(double)), SLOT(onStopSpinBoxValueChanged(double)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param newValue [double]
  */
void DoubleSpinBoxRangeLinker::onStartSpinBoxValueChanged(double newValue)
{
    stopSpinBox_->setMinimum(newValue);
}

/**
  * @param newValue [double]
  */
void DoubleSpinBoxRangeLinker::onStopSpinBoxValueChanged(double newValue)
{
    startSpinBox_->setMaximum(newValue);
}
