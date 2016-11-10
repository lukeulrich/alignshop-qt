/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QMetaType>

#include "DnaPattern.h"
#include "BioString.h"
#include "macros.h"
#include "misc.h"

static int qRegisterTypes()
{
    qRegisterMetaType<DnaPattern>("DnaPattern");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterTypes)


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param pattern [const QByteArray &]
  */
DnaPattern::DnaPattern(const QByteArray &pattern)
    : valid_(false)
{
    setPattern(pattern);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void DnaPattern::clear()
{
    setPattern(QByteArray());
}

QString DnaPattern::displayText() const
{
    char buffer[2] = " ";

    QString text;
    for (const char *x = pattern_.constData(); *x; ++x)
    {
        buffer[0] = *x;
        QString displayLetter = buffer;

        switch(*x)
        {
        case 'R':
            displayLetter = "[A/G]";
            break;
        case 'Y':
            displayLetter = "[C/T]";
            break;
        case 'M':
            displayLetter = "[A/C]";
            break;
        case 'K':
            displayLetter = "[G/T]";
            break;
        case 'S':
            displayLetter = "[C/G]";
            break;
        case 'W':
            displayLetter = "[A/T]";
            break;
        case 'H':
            displayLetter = "[A/C/T]";
            break;
        case 'B':
            displayLetter = "[C/G/T]";
            break;
        case 'V':
            displayLetter = "[A/C/G]";
            break;
        case 'D':
            displayLetter = "[A/G/T]";
            break;
        case 'N':
            displayLetter = "*";
            break;

        default:
            break;
        }

        text += displayLetter;
    }

    return text;
}

/**
  * @param bioString [const BioString &]
  * @param offset [const int]
  * @returns int
  */
int DnaPattern::indexIn(const BioString &bioString, const int offset) const
{
    if (bioString.isEmpty())
        return -1;

    ASSERT(offset >= 1 && offset <= bioString.length());

    int maxPositionThatCanMatch = bioString.length() - pattern_.length() + 1;
    for (int i=offset; i<=maxPositionThatCanMatch; ++i)
        if (matchesAt(bioString, i))
            return i;

    return -1;
}

/**
  * @returns bool
  */
bool DnaPattern::isEmpty() const
{
    return pattern_.isEmpty();
}

/**
  * @returns bool
  */
bool DnaPattern::isValid() const
{
    return valid_;
}

/**
  * @param pattern [const QByteArray &]
  * @returns bool
  */
bool DnaPattern::isValidPattern(const QByteArray &pattern) const
{
    for (const char *x = pattern.constData(); *x; ++x)
    {
        if (*x == 'A' ||
            *x == 'C' ||
            *x == 'G' ||
            *x == 'T' ||
            *x == 'R' ||
            *x == 'Y' ||
            *x == 'M' ||
            *x == 'K' ||
            *x == 'S' ||
            *x == 'W' ||
            *x == 'H' ||
            *x == 'B' ||
            *x == 'V' ||
            *x == 'D' ||
            *x == 'N' ||
            *x == '-' ||
            *x == ' ')
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}

/**
  * @returns int
  */
int DnaPattern::length() const
{
    return pattern_.length();
}

/**
  * @param bioString [const BioString &]
  * @param offset [const int]
  * @returns bool
  */
bool DnaPattern::matchesAt(const BioString &bioString, const int offset) const
{
    if (bioString.isEmpty())
        return false;

    ASSERT(offset >= 1 && offset <= bioString.length());
    if (offset + pattern_.length() - 1 > bioString.length())
        return false;

    if (pattern_.isEmpty())
        return false;

    const char *queryChar = bioString.constData() + offset - 1;
    for (const char *patternChar = pattern_.constData(); *patternChar; ++patternChar, ++queryChar)
        if (!matches(*queryChar, *patternChar))
            return false;

    return true;
}

/**
  * @param bioString [const BioString &]
  * @returns bool
  */
bool DnaPattern::matchesAtBeginning(const BioString &bioString) const
{
    return matchesAt(bioString, 1);
}

/**
  * @param bioString [const BioString &]
  * @returns bool
  */
bool DnaPattern::matchesAtEnd(const BioString &bioString) const
{
    return matchesAt(bioString, qMax(1, bioString.length() - pattern_.length() + 1));
}

/**
  * @returns QByteArray
  */
QByteArray DnaPattern::pattern() const
{
    return pattern_;
}

/**
  * @param newPattern [const QByteArray &]
  */
void DnaPattern::setPattern(const QByteArray &newPattern)
{
    pattern_ = newPattern;
    valid_ = isValidPattern(pattern_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param queryChar [const char]
  * @param patternChar [const char]
  * @returns bool
  */
bool DnaPattern::matches(const char queryChar, const char patternChar) const
{
    switch (patternChar)
    {
    case 'A':        return queryChar == 'A' || queryChar == 'a';
    case 'C':        return queryChar == 'C' || queryChar == 'c';
    case 'G':        return queryChar == 'G' || queryChar == 'g';
    case 'T':        return queryChar == 'T' || queryChar == 't';

    // Double characters
    case 'R':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'G':
        case 'g':
            return true;

        default:
            return false;
        }
    case 'Y':
        switch (queryChar)
        {
        case 'T':
        case 't':
        case 'C':
        case 'c':
            return true;

        default:
            return false;
        }
    case 'M':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'C':
        case 'c':
            return true;

        default:
            return false;
        }
    case 'K':
        switch (queryChar)
        {
        case 'G':
        case 'g':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }
    case 'S':
        switch (queryChar)
        {
        case 'C':
        case 'c':
        case 'G':
        case 'g':
            return true;

        default:
            return false;
        }
    case 'W':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }

    // Triple characters
    case 'H':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'C':
        case 'c':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }
    case 'B':
        switch (queryChar)
        {
        case 'C':
        case 'c':
        case 'G':
        case 'g':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }
    case 'V':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'C':
        case 'c':
        case 'G':
        case 'g':
            return true;

        default:
            return false;
        }
    case 'D':
        switch (queryChar)
        {
        case 'A':
        case 'a':
        case 'G':
        case 'g':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }

    // Quad characters
    case 'N':
        switch(queryChar)
        {
        case 'A':
        case 'a':
        case 'C':
        case 'c':
        case 'G':
        case 'g':
        case 'T':
        case 't':
            return true;

        default:
            return false;
        }

    // Gap characters
    case '-':
        return ::isGapCharacter(queryChar);

    // Any character
    case ' ':
        return true;

    default:
        return false;
    }
}
