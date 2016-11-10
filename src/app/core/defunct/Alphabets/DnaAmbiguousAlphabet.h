/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAAMBIGUOUSALPHABET_H
#define DNAAMBIGUOUSALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class DnaAmbiguousAlphabet : public IAlphabet
{
public:
    DnaAmbiguousAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kDnaAmbiguousCharacters)
    {
    }
};

#endif // DNAAMBIGUOUSALPHABET_H
