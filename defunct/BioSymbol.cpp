/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "BioSymbol.h"

#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * The threshold parameter must be between 0 and 1 or an assertion will be thrown. In release mode,
  * the threshold will be clamped to this range if this requirement is violated.
  *
  * @param symbol [char]
  * @param characters [const QString &]
  * @param threshold [qreal]
  */
BioSymbol::BioSymbol(char symbol, const QString &characters, qreal threshold) : symbol_(symbol)
{
    ASSERT_X(threshold >= 0 && threshold <= 1, "threshold out of range");
    if (threshold < 0)
        threshold = 0;
    else if (threshold > 1)
        threshold = 1;

    threshold_ = threshold;
    setCharacters(characters);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const BioSymbol &]
  * @returns bool
  */
bool BioSymbol::operator==(const BioSymbol &other) const
{
    if (this == &other)
        return true;

    return this->symbol_ == other.symbol_
            && this->threshold_ == other.threshold_
            && this->characterSet_ == other.characterSet_;
}

/**
  * @param other [const BioSymbol &]
  * @returns bool
  */
bool BioSymbol::operator!=(const BioSymbol &other) const
{
    return !(*this == other);
}

// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Walks through each character in characters and adds it to the internal set. Obviously, duplicates are ignored by
  * virtue of the QSet<char> container class.
  *
  * @param characters [const QString &]
  */
void BioSymbol::addCharacters(const QString &characters)
{
    characterSet_ += stringToCharSet(characters);
}

/**
  * Builds a QString of all the unique characters appended in the order returned by the QSet iterator.
  *
  * @returns QString
  */
QString BioSymbol::characters() const
{
    QString uniqueChars;
    QSet<char>::const_iterator it = characterSet_.begin();
    while (it != characterSet_.end())
    {
        uniqueChars += *it;
        ++it;
    }

    return uniqueChars;
}

/**
  * @returns QSet<char>
  */
QSet<char> BioSymbol::characterSet() const
{
    return characterSet_;
}

/**
  * @param character [char]
  * @returns bool
  */
bool BioSymbol::hasCharacter(char character) const
{
    return characterSet_.contains(character);
}

/**
  * @param characters [const QString &]
  */
void BioSymbol::removeCharacters(const QString &characters)
{
    characterSet_ -= stringToCharSet(characters);
}

/**
  * Walks through each character in characters and adds it to the internal set. Obviously, duplicates are ignored by
  * virtue of the QSet<char> container class.
  *
  * @param characters [const QString &]
  */
void BioSymbol::setCharacters(const QString &characters)
{
    characterSet_.clear();
    addCharacters(characters);
}

/**
  * @param symbol [char]
  */
void BioSymbol::setSymbol(char symbol)
{
    symbol_ = symbol;
}

/**
  *
  *
  * @param threshold [qreal]
  */
void BioSymbol::setThreshold(qreal threshold)
{
    threshold_ = threshold;
}

/**
  * @returns char
  */
char BioSymbol::symbol() const
{
    return symbol_;
}

/**
  * @returns qreal
  */
qreal BioSymbol::threshold() const
{
    return threshold_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param string [const QString &]
  * @returns QSet<char>
  */
QSet<char> BioSymbol::stringToCharSet(const QString &string) const
{
    QSet<char> charSet;
    const QChar *x = string.constData();
    while (!x->isNull())
    {
        charSet << x->toAscii();
        ++x;
    }

    return charSet;
}


