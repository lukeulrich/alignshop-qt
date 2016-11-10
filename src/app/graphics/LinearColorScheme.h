/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LINEARCOLORSCHEME_H
#define LINEARCOLORSCHEME_H

#include <QtGui/QColor>

#include "CharColorScheme.h"
#include "LinearColorStyle.h"
#include "TextColorStyle.h"

#include "../core/macros.h"
#include "../core/types.h"

/**
  * LinearColorScheme associates distinct LinearColorStyles for both foreground and background colors on a per character
  * basis.
  *
  * If a LinearColorStyle has not been defined for a given character's background or foreground colors, then the
  * TextColorStyle for that character as determined by the CharColorScheme is returned.
  */
class LinearColorScheme : public CharColorScheme
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an empty linear color scheme with defaultTextColorStyle
    LinearColorScheme(const TextColorStyle &defaultTextColorStyle = TextColorStyle());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns the background color for character and value
    QColor backgroundColor(const char character, const double value) const;
    //! Returns the foreground color for character and value
    QColor foregroundColor(const char character, const double value) const;
    //! Sets the background linear color style for character to linearColorStyle
    void setBackgroundLinearColors(char character, const LinearColorStyle &linearColorStyle);
    //! Sets the foreground linear color style for character to linearColorStyle
    void setForegroundLinearColors(char character, const LinearColorStyle &linearColorStyle);
    // Overloading the parent CharColorScheme method, but with different arguments. Since the parent class method is
    // still applicable, re-expose it here.
    using CharColorScheme::textColorStyle;
    //! Returns the color style for the character and value combination
    TextColorStyle textColorStyle(char character, double value) const;

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QHash<char, LinearColorStyle> foregroundColors_;
    QHash<char, LinearColorStyle> backgroundColors_;
};

#endif // LINEARCOLORSCHEME_H
