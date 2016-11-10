/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IALPHABETDETECTOR_H
#define IALPHABETDETECTOR_H

#include <QtCore/QObject>

#include "../Alphabet.h"

class BioString;
class BioStringValidator;

class IAlphabetDetector : public QObject
{
public:
    virtual ~IAlphabetDetector() {}

    virtual Alphabet detectAlphabet(const BioString &bioString) const = 0;
    virtual BioStringValidator validatorForAlphabet(const Alphabet &alphabet) const = 0;

protected:
    IAlphabetDetector(QObject *parent) : QObject(parent)
    {
    }
};

#endif // IALPHABETDETECTOR_H
