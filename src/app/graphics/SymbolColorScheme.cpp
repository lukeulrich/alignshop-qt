/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SymbolColorScheme.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
SymbolColorScheme::SymbolColorScheme(const TextColorStyle &defaultTextColorStyle) :
    CharColorScheme(defaultTextColorStyle)
{
    memset(lookup_, 0, 128 * 128 * sizeof(char));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param character [char]
  * @param symbols [const QByteArray &]
  * @param textColorStyle [const TextColorStyle &]
  */
void SymbolColorScheme::setSymbolsTextColorStyle(char character, const QByteArray &symbols, const TextColorStyle &textColorStyle)
{
    ASSERT(character >= 0);
    if (character < 0)
        return;

    symbolTextColorStyles_ << textColorStyle;
    int index = symbolTextColorStyles_.size();
    ASSERT(index < 256);

    for (const char *x = symbols.constData(); *x; ++x)
    {
        ASSERT(*x >= 0);
        if (*x < 0)
            continue;

        lookup_[static_cast<int>(character)][static_cast<int>(*x)] = index;
    }

//    for (const char *x = symbols.constData(); *x; ++x)
//        symbolTextColorStyles_[pack(character, *x)] = textColorStyle;

//    for (int i=0, z=symbols.length(); i<z; ++i)
//        symbolTextColorStyles_[character].insert(symbols.at(i), textColorStyle);
}

/**
  * Returns the color style for character and symbol if it exists otherwise, the color style for character alone is
  * returned.
  *
  * Originally used 5 hash lookups to determine the text color style! Now it is down to 2 by packing the character and
  * symbol into an integer.
  *
  * @param character [char]
  * @param symbol [char]
  * @returns TextColorStyle
  */
TextColorStyle SymbolColorScheme::textColorStyle(char character, char symbol) const
{
    ASSERT(character >= 0);
    if (character > 0)
    {
        int index = lookup_[static_cast<int>(character)][static_cast<int>(symbol)];
        if (index != 0)
            return symbolTextColorStyles_[index-1];
    }

    return CharColorScheme::textColorStyle(character);

//    int key = pack(character, symbol);
//    if (symbolTextColorStyles_.contains(key))
//        return symbolTextColorStyles_.value(key);

//    return CharColorScheme::textColorStyle(character);
}
