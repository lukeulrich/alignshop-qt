/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SymbolColorProvider.h"
#include "SymbolColorScheme.h"
#include "../core/LiveSymbolString.h"
#include "../core/ObservableMsa.h"

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
    char character = msa.at(row)->at(column);
    char symbol = (liveSymbolString_ != nullptr) ? liveSymbolString_->symbolString().at(column-1) : ' ';
    return symbolColorScheme_.textColorStyle(character, symbol);
}

/**
  * @param msa [const Msa &]
  * @param row [int]
  * @param columns [const ClosedIntRange &]
  * @returns QVector<TextColorStyle>
  */
QVector<TextColorStyle> SymbolColorProvider::colors(const Msa &msa, int row, const ClosedIntRange &columns) const
{
    QVector<TextColorStyle> styles;
    styles.reserve(columns.length());

    const char *ch = msa.at(row)->constData() + columns.begin_ - 1;
    if (liveSymbolString_ != nullptr)
    {
        QByteArray symbolString = liveSymbolString_->symbolString();
        const char *symbol = symbolString.constData() + columns.begin_ - 1;
        for (int i=0, z= columns.length(); i< z; ++i)
        {
            styles << symbolColorScheme_.textColorStyle(*ch, *symbol);
            ++ch;
            ++symbol;
        }
    }
    else
    {
        for (int i=0, z=columns.length(); i<z; ++i)
        {
            styles << symbolColorScheme_.textColorStyle(*ch, ' ');
            ++ch;
        }
    }

    return styles;
}
