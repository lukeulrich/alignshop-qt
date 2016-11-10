/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDataStream>

#include "BioSymbol.h"

#include "global.h"
#include "misc.h"       // For streaming char's
#include "macros.h"

static int registerMetaTypes()
{
    qRegisterMetaType<BioSymbol>("BioSymbol");
    qRegisterMetaTypeStreamOperators<BioSymbol>("BioSymbol");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetaTypes);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * The threshold parameter must be between 0 and 1 or an assertion will be thrown. In release mode,
  * the threshold will be clamped to this range if this requirement is violated.
  *
  * @param symbol [char]
  * @param characters [const QByteArray &]
  * @param threshold [qreal]
  */
BioSymbol::BioSymbol(char symbol, const QByteArray &characters, qreal threshold)
    : label_(symbol),
      symbol_(symbol)
{
    ASSERT_X(threshold >= 0 && threshold <= 1, "threshold out of range");
    if (threshold < 0)
        threshold = 0;
    else if (threshold > 1)
        threshold = 1;

    threshold_ = threshold;
    setCharacters(characters);
}

BioSymbol::BioSymbol(const QString &label, char symbol, const QByteArray &characters, qreal threshold)
    : label_(label),
      symbol_(symbol)
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
  * @param characters [const QByteArray &]
  */
void BioSymbol::addCharacters(const QByteArray &characters)
{
    characterSet_ += stringToCharSet(characters);
}

/**
  * Builds a QByteArray of all the unique characters appended in the order returned by the QSet iterator.
  *
  * @returns QByteArray
  */
QByteArray BioSymbol::characters() const
{
    QByteArray uniqueChars;
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

QString BioSymbol::label() const
{
    return label_;
}

/**
  * @param characters [const QByteArray &]
  */
void BioSymbol::removeCharacters(const QByteArray &characters)
{
    characterSet_ -= stringToCharSet(characters);
}

/**
  * Walks through each character in characters and adds it to the internal set. Obviously, duplicates are ignored by
  * virtue of the QSet<char> container class.
  *
  * @param characters [const QByteArray &]
  */
void BioSymbol::setCharacters(const QByteArray &characters)
{
    characterSet_.clear();
    addCharacters(characters);
}

void BioSymbol::setLabel(const QString &newLabel)
{
    label_ = newLabel;
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
  * @param string [const QByteArray &]
  * @returns QSet<char>
  */
QSet<char> BioSymbol::stringToCharSet(const QByteArray &string) const
{
    QSet<char> charSet;
    const char *x = string.constData();
    while (*x)
    {
        charSet << *x;
        ++x;
    }

    return charSet;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
QDataStream &operator<<(QDataStream &out, const BioSymbol &bioSymbol)
{
    out << bioSymbol.label_;
    out << bioSymbol.characters();
    out << bioSymbol.threshold_;
    out << bioSymbol.symbol_;

    return out;
}

QDataStream &operator>>(QDataStream &in, BioSymbol &bioSymbol)
{
    in >> bioSymbol.label_;
    QByteArray characters;
    in >> characters;
    bioSymbol.setCharacters(characters);
    in >> bioSymbol.threshold_;
    in >> bioSymbol.symbol_;

    return in;
}
