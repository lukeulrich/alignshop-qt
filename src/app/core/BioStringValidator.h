/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSTRINGVALIDATOR_H
#define BIOSTRINGVALIDATOR_H

#include <QtCore/QSet>

#include "BioString.h"

/**
  * BioStringValidator provides a flexible mechanism for determining whether a BioString's sequence or arbitrary
  * characters are valid (they are a subset of a user-defined character list).
  *
  * A BioString is considered valid if and only if all of its sequence characters are a subset of a user-defined set
  * of characters (passed in as a BioString).
  */
class BioStringValidator
{
public:
    explicit BioStringValidator(const BioString &validCharacters = BioString());

    bool operator==(const BioStringValidator &other) const;
    bool operator!=(const BioStringValidator &other) const;

    bool isValid(const BioString &bioString) const;
    bool isValid(const QByteArray &byteArray) const;
    bool isValid(const QChar &ch) const;
    bool isValid(const char &ch) const;

    BioString validCharacters() const;
    void setValidCharacters(const BioString &validCharacters);

private:
    QSet<char> validCharSet_;
};

#endif // BIOSTRINGVALIDATOR_H
