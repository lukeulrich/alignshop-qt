/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INCREMENTNUMBERGENERATOR_H
#define INCREMENTNUMBERGENERATOR_H

#include "IntNumberGenerator.h"

class IncrementNumberGenerator : public IntNumberGenerator
{
public:
    IncrementNumberGenerator(int initialValue = 0) : IntNumberGenerator(initialValue, 1)
    {
    }
};

#endif // INCREMENTNUMBERGENERATOR_H
