/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SymbolColorScheme.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
SymbolColorScheme::SymbolColorScheme(const TextColorStyle &defaultTextColorStyle) :
    CharColorScheme(defaultTextColorStyle)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param character [char]
  * @param symbols [const QString &]
  * @param textColorStyle [const TextColorStyle &]
  */
void SymbolColorScheme::setSymbolsTextColorStyle(char character, const QString &symbols, const TextColorStyle &textColorStyle)
{
    QByteArray symbolage = symbols.toAscii();
    for (int i=0, z=symbolage.length(); i<z; ++i)
        symbolTextColorStyles_[character].insert(symbolage.at(i), textColorStyle);
}

/**
  * Returns the color style for character and symbol if it exists otherwise, the color style for character alone is
  * returned.
  *
  * @param character [char]
  * @param symbol [char]
  * @returns TextColorStyle
  */
TextColorStyle SymbolColorScheme::textColorStyle(char character, char symbol) const
{
    if (symbolTextColorStyles_.contains(character) &&
        symbolTextColorStyles_.value(character).contains(symbol))
    {
        return symbolTextColorStyles_.value(character).value(symbol);
    }

    return CharColorScheme::textColorStyle(character);
}
