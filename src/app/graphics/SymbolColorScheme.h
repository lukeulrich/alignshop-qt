/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SYMBOLCOLORSCHEME_H
#define SYMBOLCOLORSCHEME_H

#include <QtCore/QHash>
#include <QtCore/QVector>

#include "CharColorScheme.h"
#include "TextColorStyle.h"

/**
  * SymbolColorScheme extends the base CharColorScheme implementation enabling TextColorStyle's to be defined for
  * specific character and symbol combinations.
  *
  * SymbolColorScheme is compatible with TextColorStyles defined for individual characters; however, preference is given
  * to any style defined for a character and symbol combination over the individual style of a specific character.
  */
class SymbolColorScheme : public CharColorScheme
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an empty symbol color scheme with defaultTextColorStyle
    SymbolColorScheme(const TextColorStyle &defaultTextColorStyle = TextColorStyle());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    // Also expose the single parameter textColorStyle method in the base class
    using CharColorScheme::textColorStyle;

    //! Sets the color style for character and each symbol combination to textColorStyle (any previous association is overwritten)
    void setSymbolsTextColorStyle(char character, const QByteArray &symbols, const TextColorStyle &textColorStyle);

    //! Returns the color style for the character and symbol combination
    TextColorStyle textColorStyle(char character, char symbol) const;

private:
    int pack(const char &first, const char &second) const
    {
        return second << 8 | first;
    }

    //    QHash<char, QHash<char, TextColorStyle> > symbolTextColorStyles_;

    // Optimized hash containing two chars packed into the first 16 bits of an integer
    // Given a 32-bit integer, we only use the first 16 bits:
    // |--------|--------|--------|--------|
    //                     symbol  character
//    QHash<int, TextColorStyle> symbolTextColorStyles_;

    // Optimized further by constructing a stack-based lookup table for every character symbol combination. A limit
    // to this approach is that only 256 colors may be discretely defined, but this should not pose a problem.
    QVector<TextColorStyle> symbolTextColorStyles_;
    //          symbol -> character -> index into styles_
    uchar lookup_[128][128];

};

#endif // SYMBOLCOLORSCHEME_H
