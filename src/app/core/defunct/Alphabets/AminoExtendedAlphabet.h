/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOEXTENDEDALPHABET_H
#define AMINOEXTENDEDALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class AminoExtendedAlphabet : public IAlphabet
{
public:
    AminoExtendedAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kAminoExtendedCharacters)
    {
    }
};

#endif // AMINOEXTENDEDALPHABET_H
