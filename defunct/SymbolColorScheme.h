/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SYMBOLCOLORSCHEME_H
#define SYMBOLCOLORSCHEME_H

#include "CharColorScheme.h"
#include "TextColorStyle.h"

#include <QtCore/QHash>

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
    void setSymbolsTextColorStyle(char character, const QString &symbols, const TextColorStyle &textColorStyle);

    //! Returns the color style for the character and symbol combination
    TextColorStyle textColorStyle(char character, char symbol) const;

private:
    QHash<char, QHash<char, TextColorStyle> > symbolTextColorStyles_;
};

#endif // SYMBOLCOLORSCHEME_H
