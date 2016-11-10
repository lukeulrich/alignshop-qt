/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAEXTENDEDALPHABET_H
#define DNAEXTENDEDALPHABET_H

#include "AbstractAlphabet.h"
#include "../constants.h"

class DnaExtendedAlphabet : public IAlphabet
{
public:
    DnaExtendedAlphabet() : AbstractAlphabet(eDnaGrammar, constants::kDnaExtendedCharacters)
    {
    }
};

#endif // DNAEXTENDEDALPHABET_H
