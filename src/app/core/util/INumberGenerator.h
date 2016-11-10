/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INUMBERGENERATOR_H
#define INUMBERGENERATOR_H

template<typename T>
class INumberGenerator
{
public:
    virtual T currentValue() const = 0;
    virtual T nextValue() = 0;
};

#endif // INUMBERGENERATOR_H
