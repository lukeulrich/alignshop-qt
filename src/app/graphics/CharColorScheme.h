/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARCOLORSCHEME_H
#define CHARCOLORSCHEME_H

#include <QtCore/QHash>

#include "TextColorStyle.h"

/**
  * CharColorScheme defines a configurable scheme for associating specific TextColorStyles with invidividual characters,
  * and a default TextColorStyle for all other characters.
  */
class CharColorScheme
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an empty color scheme with defaultTextColorStyle
    CharColorScheme(const TextColorStyle &defaultTextColorStyle = TextColorStyle());

    // ------------------------------------------------------------------------------------------------
    // Operator methods
    bool operator==(const CharColorScheme &other) const;            //!< Returns true if other is equal to this object; false otherwise
    bool operator!=(const CharColorScheme &other) const;            //!< Returns true if other is equal to this object; false otherwise

    // ------------------------------------------------------------------------------------------------
    // Public methods
    TextColorStyle defaultTextColorStyle() const;                   //!< Returns the default text color style
    bool hasColorStyleFor(char character) const;                    //!< Returns true if a color style has been defined for character; false otherwise
    //! Sets the color style for character to textColorStyle (any previous definition is overwritten)
    void setTextColorStyle(char character, const TextColorStyle &textColorStyle);
    //!< Sets the default color style to defaultTextColorStyle
    void setDefaultTextColorStyle(const TextColorStyle &defaultTextColorStyle);
    TextColorStyle textColorStyle(char character) const;            //!< Returns the color style for character or the default text color style if none has been defined for character

protected:
    // Again, it is much more efficient to simply pre-allocate all 128 TextColorStyles to avoid using a hash lookup
    TextColorStyle charColorStyles_[128];

private:
    bool definedStyles_[128];
    TextColorStyle defaultTextColorStyle_;
};

#endif // CHARCOLORSCHEME_H
