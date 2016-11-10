/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef AGDOUBLESPINBOX_H
#define AGDOUBLESPINBOX_H

#include <QtGui/QDoubleSpinBox>
#include <cmath>

/**
  * AgDoubleSpinBox extends the standard double spin box with commonly useful modifications including the ability to
  * control the number of visible decimal places and dynamically scale the step amount relative to the current value.
  *
  * QDoubleSpinBox limits the precision of the source value to a certain number of decimal places (I think largely for
  * display purposes, that is, to size the widget to a certain extent). The approach taken here is not to limit the
  * precision of the underlying value, but rather limit the number of visible decimal places. This may be configured via
  * the visibleDecimals property.
  *
  * Additionally, it is often useful to step through the range using a scaled step amount that is related to the current
  * value (e.g. zooming). The current QDoubleSpinBox only provides for stepping by an absolute amount. AgDoubleSpinBox
  * provides a property, stepFactor, which will configure the next positive step value to be 1 + stepFactor multiplied
  * by the current value. If stepFactor is zero, then the absolute value amount will be used.
  *
  * For example, given a stepFactor of .1 and a starting value of 100, then
  *
  * Step 1, value = 100 * (1 + .1) = 110
  * Step 2, value = 110 * (1 + .1) = 121
  * and so forth...
  *
  * Similarly, given a value of 121,
  * Step -1, value = 121 / (1 + .1) = 110
  * Step -2, value = 110 / (1 + .1) = 100
  * and so forth...
  */
class AgDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

    Q_PROPERTY(int visibleDecimals
               READ visibleDecimals
               WRITE setVisibleDecimals);
    Q_PROPERTY(double stepFactor_
               READ stepFactor
               WRITE setStepFactor);

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    /**
      * Constructor. Initializes the widget to have zero visible decimals and a zero stepFactor.
      *
      * @param parent [QWidget *]
      */
    AgDoubleSpinBox(QWidget *parent = 0) :
        QDoubleSpinBox(parent),
        visibleDecimals_(0),
        stepFactor_(0)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // Public methods
    /**
      * If step factor is greater than zero, then take steps steps but scaling the value by stepFactor_. Otherwise,
      * simply call the parent stepBy method.
      *
      * @param steps [int]
      */
    virtual void stepBy(int steps)
    {
        if (qFuzzyCompare(stepFactor_, 0))
        {
            QDoubleSpinBox::stepBy(steps);
            return;
        }

        // Otherwise, we should step relative to the stepFactor_
        setValue(value() * pow(1. + stepFactor_, steps));
    }

    /**
      * Returns the current step factor.
      *
      * @returns double
      */
    double stepFactor() const
    {
        return stepFactor_;
    }

    /**
      * Reimplemented from QDoubleSpinBox. Returns a textual representation of value with the group separator removed.
      * The minimum number of decimals() and visibleDecimals() will be used when determining the return valuel. Thus, it
      * is important to make sure that these are configured as desired.
      *
      * Note: The string value returned here may not reflect the actual value! This is particularly the case if
      * the number of visibleDecimals is less than the number of decimals.
      *
      * @param value [double]
      * @returns QString
      */
    QString textFromValue(double value) const
    {
        return locale().toString(value, 'f', qMin(decimals(), visibleDecimals_)).remove(locale().groupSeparator());
    }

    /**
      * Returns the number of currently visible decimals.
      *
      * @returns int
      */
    int visibleDecimals() const
    {
        return visibleDecimals_;
    }


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    /**
      * Sets the number of visible decimals to visibleDecimals. If visibleDecimals is less than zero, nothing is done.
      *
      * @param visibleDecimals [int]
      */
    void setVisibleDecimals(int visibleDecimals)
    {
        if (visibleDecimals >= 0)
            visibleDecimals_ = visibleDecimals;
    }

    /**
      * Sets the step factor to stepFactor. Additionally, update the single step value. Setting a stepFactor of less
      * than zero does nothing.
      *
      * @param stepFactor [double]
      */
    void setStepFactor(double stepFactor)
    {
        if (stepFactor < 0)
            return;

        stepFactor_ = stepFactor;
    }

private:
    int visibleDecimals_;
    double stepFactor_;
};


#endif // AGDOUBLESPINBOX_H
