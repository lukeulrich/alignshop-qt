/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LinearColorScheme.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
LinearColorScheme::LinearColorScheme(const TextColorStyle &defaultTextColorStyle)
    : CharColorScheme(defaultTextColorStyle)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param character [const char]
  * @param value [const double]
  * @returns QColor
  */
QColor LinearColorScheme::backgroundColor(const char character, const double value) const
{
    if (backgroundColors_.contains(character))
        return backgroundColors_.value(character).linearColor(value);

    return CharColorScheme::textColorStyle(character).background_;
}

/**
  * @param character [const char]
  * @param value [const double]
  * @returns QColor
  */
QColor LinearColorScheme::foregroundColor(const char character, const double value) const
{
    if (foregroundColors_.contains(character))
        return foregroundColors_.value(character).linearColor(value);

    return CharColorScheme::textColorStyle(character).foreground_;
}

/**
  * @param character [char]
  * @param linearColorStyle [const LinearColorStyle &]
  */
void LinearColorScheme::setBackgroundLinearColors(char character, const LinearColorStyle &linearColorStyle)
{
    backgroundColors_.insert(character, linearColorStyle);
}

/**
  * @param character [char]
  * @param linearColorStyle [const LinearColorStyle &]
  */
void LinearColorScheme::setForegroundLinearColors(char character, const LinearColorStyle &linearColorStyle)
{
    foregroundColors_.insert(character, linearColorStyle);
}

/**
  * @param character [char]
  * @param value [double]
  * @returns TextColorStyle
  */
TextColorStyle LinearColorScheme::textColorStyle(char character, double value) const
{
    return TextColorStyle(foregroundColor(character, value), backgroundColor(character, value));
}
