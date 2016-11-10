/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "BioSymbolColorScheme.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param bioSymbolGroup [const BioSymbolGroup &]
  * @param charColorScheme [const CharColorScheme &]
  */
BioSymbolColorScheme::BioSymbolColorScheme(const BioSymbolGroup &bioSymbolGroup, const CharColorScheme &charColorScheme)
    : bioSymbolGroup_(bioSymbolGroup), charColorScheme_(charColorScheme)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioSymbolGroup
  */
BioSymbolGroup BioSymbolColorScheme::bioSymbolGroup() const
{
     return bioSymbolGroup_;
}

/**
  * @returns CharColorScheme
  */
CharColorScheme BioSymbolColorScheme::charColorScheme() const
{
     return charColorScheme_;
}

/**
  * @param bioSymbolGroup [const BioSymbolGroup &]
  */
void BioSymbolColorScheme::setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup)
{
    bioSymbolGroup_ = bioSymbolGroup;
}

/**
  * @param charColorScheme [const CharColorScheme &]
  */
void BioSymbolColorScheme::setCharColorScheme(const CharColorScheme &charColorScheme)
{
    charColorScheme_ = charColorScheme;
}

/**
  * @param symbol [char]
  * @param textColorStyle [const TextColorStyle &]
  */
void BioSymbolColorScheme::setSymbolColorStyle(char symbol, const TextColorStyle &textColorStyle)
{
    symbolColorStyles_.insert(symbol, textColorStyle);
}

/**
  * The appropriate color to determine is based upon the following:
  * o If character is defined in CharColorScheme, then use its color style - regardless of the symbol
  * o If a color style is defined for the character and symbol combination, return that color style
  * o Otherwise, return the CharColorScheme's default color style
  *
  * @param character [char]
  * @param symbol [char]
  * @returns TextColorStyle
  */
TextColorStyle BioSymbolColorScheme::symbolColorStyle(char character, char symbol) const
{
    // Is there a constant color defined for character?
    if (charColorScheme_.hasColorStyleFor(character))
        return charColorScheme_.textColorStyle(character);

    // Is there a defined color style for this character/symbol combination
    if (symbolColorStyles_.contains(symbol)
        && bioSymbolGroup_.isCharAssociatedWithSymbol(character, symbol))
    {
        return symbolColorStyles_.value(symbol);
    }

    // Simply return the default color style
    return charColorScheme_.defaultTextColorStyle();
}

/**
  * @returns QHash<char, TextColorStyle>
  */
QHash<char, TextColorStyle> BioSymbolColorScheme::symbolColorStyles() const
{
    return symbolColorStyles_;
}
