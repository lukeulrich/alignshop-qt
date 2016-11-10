/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTALPHABET_H
#define ABSTRACTALPHABET_H

#include "IAlphabet.h"
#include "../enums.h"

class AbstractAlphabet : public IAlphabet
{
public:
    Grammar grammar() const
    {
        return grammar_;
    }

    const char *characters() const
    {
        return characters_;
    }

protected:
    AbstractAlphabet(Grammar grammar, const char *characters) : IAlphabet(), grammar_(grammar), characters_(characters)
    {
    }

private:
    Grammar grammar_;
    const char *characters_;
};

#endif // ABSTRACTALPHABET_H
