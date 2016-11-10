/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QSpinBox>

#include "SpinBoxRangeLinker.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param startSpinBox [QSpinBox *]
  * @param stopSpinBox [QSpinBox *]
  * @param parent [QObject *]
  */
SpinBoxRangeLinker::SpinBoxRangeLinker(QSpinBox *startSpinBox, QSpinBox *stopSpinBox, QObject *parent)
    : QObject(parent),
      startSpinBox_(startSpinBox),
      stopSpinBox_(stopSpinBox)
{
    if (startSpinBox_ == nullptr || stopSpinBox_ == nullptr)
        return;

    ASSERT(startSpinBox_ != stopSpinBox_);

    connect(startSpinBox_, SIGNAL(valueChanged(int)), SLOT(onStartSpinBoxValueChanged(int)));
    connect(stopSpinBox_, SIGNAL(valueChanged(int)), SLOT(onStopSpinBoxValueChanged(int)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param newValue [int]
  */
void SpinBoxRangeLinker::onStartSpinBoxValueChanged(int newValue)
{
    stopSpinBox_->setMinimum(newValue);
}

/**
  * @param newValue [int]
  */
void SpinBoxRangeLinker::onStopSpinBoxValueChanged(int newValue)
{
    startSpinBox_->setMaximum(newValue);
}
