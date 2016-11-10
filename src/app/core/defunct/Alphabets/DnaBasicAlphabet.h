/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNABASICALPHABET_H
#define DNABASICALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class DnaBasicAlphabet : public IAlphabet
{
public:
    DnaBasicAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kDnaBasicCharacters)
    {
    }
};

#endif // DNABASICALPHABET_H
