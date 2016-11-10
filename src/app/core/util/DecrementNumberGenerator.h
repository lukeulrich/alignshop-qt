/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DECREMENTNUMBERGENERATOR_H
#define DECREMENTNUMBERGENERATOR_H

#include "IntNumberGenerator.h"

class DecrementNumberGenerator : public IntNumberGenerator
{
public:
    DecrementNumberGenerator(int initialValue = 0) : IntNumberGenerator(initialValue, -1)
    {
    }
};

#endif // DECREMENTNUMBERGENERATOR_H
