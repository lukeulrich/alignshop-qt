/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTNUMBERGENERATOR_H
#define ABSTRACTNUMBERGENERATOR_H

#include "INumberGenerator.h"

template<typename T>
class AbstractNumberGenerator : public INumberGenerator<T>
{
public:
    AbstractNumberGenerator(T initialValue, T updateDelta)
        : INumberGenerator<T>(), currentValue_(initialValue), updateDelta_(updateDelta)
    {
    }

    virtual T currentValue() const
    {
        return currentValue_;
    }

    virtual T nextValue()
    {
        currentValue_ += updateDelta_;
        return currentValue_;
    }

private:
    T currentValue_;
    T updateDelta_;
};

#endif // ABSTRACTNUMBERGENERATOR_H
