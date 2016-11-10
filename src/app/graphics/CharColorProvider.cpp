/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "CharColorProvider.h"

#include "../core/Msa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param charColorScheme [const CharColorScheme &]
  */
CharColorProvider::CharColorProvider(const CharColorScheme &charColorScheme) : charColorScheme_(charColorScheme)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param msa [const Msa &]
  * @param row [int]
  * @param column [int]
  * @returns TextColorStyle
  */
TextColorStyle CharColorProvider::color(const Msa &msa, int row, int column) const
{
    char character = msa.at(row)->at(column);
    return charColorScheme_.textColorStyle(character);
}

/**
  * @param msa [const Msa &]
  * @param row [int]
  * @param columns [const ClosedIntRange &]
  * @returns QVector<TextColorStyle>
  */
QVector<TextColorStyle> CharColorProvider::colors(const Msa &msa, int row, const ClosedIntRange &columns) const
{
    QVector<TextColorStyle> styles;
    styles.reserve(columns.length());

    const char *x = msa.at(row)->constData() + columns.begin_ - 1;
    for (int i=columns.begin_; i<= columns.end_; ++i, ++x)
        styles << charColorScheme_.textColorStyle(*x);

    return styles;
}

