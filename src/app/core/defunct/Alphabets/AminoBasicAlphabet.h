/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOBASICALPHABET_H
#define AMINOBASICALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class AminoBasicAlphabet : public IAlphabet
{
public:
    AminoBasicAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kAminoBasicCharacters)
    {
    }
};

#endif // AMINOBASICALPHABET_H
