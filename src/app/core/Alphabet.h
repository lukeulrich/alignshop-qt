/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ALPHABET_H
#define ALPHABET_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>         // Required for the qHash method

#include "constants.h"
#include "enums.h"
#include "global.h"

class Alphabet
{
public:
    Alphabet(Grammar grammar = eUnknownGrammar,
             QByteArray characters = QByteArray(),
             char anyCharacter = constants::kDefaultAnyCharacter)
        : grammar_(grammar), characters_(characters), anyCharacter_(anyCharacter)
    {
    }

    bool operator==(const Alphabet &other) const
    {
        return grammar_ == other.grammar_ &&
                characters_ == other.characters_ &&
                anyCharacter_ == other.anyCharacter_;
    }

    bool operator!=(const Alphabet &other) const
    {
        return !operator==(other);
    }

    // An alphabet is less than another alphabet if it has fewer characters - ie. is more specific
    bool operator<(const Alphabet &other) const
    {
        // Case 2: this grammar is unknown
        if (grammar_ == eUnknownGrammar)
            return false;

        // Case 3: other grammar is unknown
        if (other.grammar_ == eUnknownGrammar)
            return true;

        return characters_.length() < other.characters_.length();
    }

    Grammar grammar() const
    {
        return grammar_;
    }

    QByteArray characters() const
    {
        return characters_;
    }

    QByteArray allCharacters() const
    {
        return characters_ + anyCharacter_;
    }

    char anyCharacter() const
    {
        return anyCharacter_;
    }

private:
    Grammar grammar_;
    QByteArray characters_;
    char anyCharacter_;
};

/**
  * @param alphabet [const Alphabet &]
  * @returns int
  */
inline int qHash(const Alphabet &alphabet)
{
    return alphabet.grammar() + qHash(alphabet.allCharacters());
}

/**
  * @param grammar [Grammar]
  * @returns Alphabet
  */
inline Alphabet ambiguousAlphabetFromGrammar(Grammar grammar)
{
    switch (grammar)
    {
    case eAminoGrammar:
        return constants::kAminoAmbiguousAlphabet;
    case eDnaGrammar:
        return constants::kDnaAmbiguousAlphabet;
    case eRnaGrammar:
        return constants::kRnaAmbiguousAlphabet;

    default:
        return Alphabet();
    }
}

Q_DECLARE_TYPEINFO(Alphabet, Q_MOVABLE_TYPE);

#endif // ALPHABET_H
