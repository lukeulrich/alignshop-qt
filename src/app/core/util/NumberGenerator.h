/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NUMBERGENERATOR_H
#define NUMBERGENERATOR_H

#include "INumberGenerator.h"

template<typename T>
class NumberGenerator : public INumberGenerator
{
public:
    NumberGenerator(T initialValue, T updateDelta) : INumberGenerator(), value_(initialValue), delta_(updateDelta)
    {
    }

    T currentValue() const
    {
        return value_;
    }

    T nextValue()
    {
        value_ += delta_;
        return value_;
    }

private:
    T value_;
    T delta_;
};

#endif // NUMBERGENERATOR_H
