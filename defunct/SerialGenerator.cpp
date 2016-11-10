/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SerialGenerator.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * If the sequence isCalled, then increment the currentValue by 1 and return this value. Otherwise, return
  * the currentValue without changing it. All subsequent calls will increment the currentValue.
  * @return integer, next sequence value
  */
int SerialGenerator::nextValue()
{
    if (isCalled_)
        ++currentValue_;
    else
        isCalled_ = true;

    return currentValue_;
}

/**
  * Convenience function for setting both the currentValue and isCalled
  * @param currentValue integer value o the current sequence value
  * @param isCalled boolean indicates whether the current value has been fetched
  * @see setValue(), isCalled()
  */
void SerialGenerator::setValue(int currentValue, bool isCalled)
{
    setValue(currentValue);
    setCalled(isCalled);
}
