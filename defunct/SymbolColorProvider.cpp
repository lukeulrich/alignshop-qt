/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SymbolColorProvider.h"

#include "LiveSymbolString.h"
#include "Msa.h"
#include "SymbolColorScheme.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param liveSymbolString [const LiveSymbolString &]
  * @param symbolColorScheme [const SymbolColorScheme &]
  */
SymbolColorProvider::SymbolColorProvider(const LiveSymbolString *liveSymbolString, const SymbolColorScheme &symbolColorScheme) :
    PositionalMsaColorProvider(), liveSymbolString_(liveSymbolString), symbolColorScheme_(symbolColorScheme)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * If liveSymbolString_ is defined, then looks up the symbol for the corresponding column and returns the relevant
  * color style.
  *
  * @param msa [const Msa &]
  * @param row [int]
  * @param column [int]
  * @returns TextColorStyle
  */
TextColorStyle SymbolColorProvider::color(const Msa &msa, int row, int column) const
{
    // Get the character
    char character = msa.at(row)->bioString().sequence().at(column-1).toAscii();

    // Get the symbol
    char symbol = ' ';
    if (liveSymbolString_)
        symbol = liveSymbolString_->symbolString().at(column-1).toAscii();

    return symbolColorScheme_.textColorStyle(character, symbol);
}
