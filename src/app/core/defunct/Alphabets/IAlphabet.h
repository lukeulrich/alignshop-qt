/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IALPHABET_H
#define IALPHABET_H

#include "../enums.h"

class IAlphabet
{
public:
    virtual ~IAlphabet() {}

    virtual Grammar grammar() const = 0;
    virtual const char *characters() const = 0;
};

#endif // IALPHABET_H
