/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INTNUMBERGENERATOR_H
#define INTNUMBERGENERATOR_H

#include "AbstractNumberGenerator.h"

class IntNumberGenerator : public AbstractNumberGenerator<int>
{
public:
    IntNumberGenerator(int initialValue, int updateDelta) : AbstractNumberGenerator<int>(initialValue, updateDelta)
    {
    }
};

#endif // INTNUMBERGENERATOR_H
