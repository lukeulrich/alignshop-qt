/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef PERCENTSPINBOX_H
#define PERCENTSPINBOX_H

#include "AgDoubleSpinBox.h"

/**
  * PercentSpinBox represents percentages in both its percent style form (which is visible) and its decimal equivalent.
  *
  * In addition to the standard value changed signal, which is emitted whenever the percentage representation changes,
  * the decimalPercentChanged signal is emitted with the corresponding value divided by 100.
  */
class PercentSpinBox : public AgDoubleSpinBox
{
    Q_OBJECT

    Q_PROPERTY(double decimalPercent
               READ decimalPercent
               WRITE setDecimalPercent);

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    /**
      * Trivial constructor
      *
      * @param parent [QWidget *]
      */
    PercentSpinBox(QWidget *parent = 0) : AgDoubleSpinBox(parent)
    {
        connect(this, SIGNAL(valueChanged(double)), SLOT(convertAndSendDecimalPercent()));
    }

    // ------------------------------------------------------------------------------------------------
    // Public method
    /**
      * Returns the decimal represesentation of the current percentage.
      *
      * @returns double
      */
    double decimalPercent() const
    {
        return value() / 100.;
    }

public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    /**
      * Sets the percentage to decimalPercent * 100.
      *
      * @param decimalPercent
      */
    void setDecimalPercent(double decimalPercent)
    {
        setValue(decimalPercent * 100.);
    }

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void decimalPercentChanged(double value);

private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    /**
      * Simply emits the decimalPercentChanged signal with the value determined by decimalPercent()
      *
      * @see decimalPercent(), decimalPercentChanged()
      */
    void convertAndSendDecimalPercent()
    {
        emit decimalPercentChanged(decimalPercent());
    }
};

#endif // PERCENTAGESPINBOX_H
