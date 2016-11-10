/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOAMBIGUOUSALPHABET_H
#define AMINOAMBIGUOUSALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class AminoAmbiguousAlphabet : public IAlphabet
{
public:
    AminoAmbiguousAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kAminoAmbiguousCharacters)
    {
    }
};

#endif // AMINOAMBIGUOUSALPHABET_H
