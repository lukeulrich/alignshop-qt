/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ALPHABETDETECTOR_H
#define ALPHABETDETECTOR_H

#include "IAlphabetDetector.h"
#include "../Alphabet.h"
#include "../BioStringValidator.h"
#include "../global.h"

class QObject;

/**
  * Ignores any gap characters when determining the alphabet
  */
class AlphabetDetector : public IAlphabetDetector
{
public:
    AlphabetDetector(QObject *parent = nullptr);

    bool acceptAnyCharacter() const;
    Alphabet detectAlphabet(const BioString &bioString) const;
    void setAcceptAnyCharacter(bool acceptAnyCharacter);
    void setAlphabets(const QVector<Alphabet> &alphabets);
    BioStringValidator validatorForAlphabet(const Alphabet &alphabet) const;

private:
    QVector<Alphabet> alphabets_;
    QVector<BioStringValidator> validators_;
    bool acceptAnyCharacter_;
};

#endif // ALPHABETDETECTOR_H
