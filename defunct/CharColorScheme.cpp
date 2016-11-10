/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "CharColorScheme.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
CharColorScheme::CharColorScheme(const TextColorStyle &defaultTextColorStyle) : defaultTextColorStyle_(defaultTextColorStyle)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const CharColorScheme &]
  * @returns bool
  */
bool CharColorScheme::operator==(const CharColorScheme &other) const
{
    if (this == &other)
        return true;

    return defaultTextColorStyle_ == other.defaultTextColorStyle_
           && charColorStyles_ == other.charColorStyles_;
}

/**
  * @param other [const CharColorScheme &]
  * @returns bool
  */
bool CharColorScheme::operator!=(const CharColorScheme &other) const
{
    return !(*this == other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns TextColorStyle
  */
TextColorStyle CharColorScheme::defaultTextColorStyle() const
{
     return defaultTextColorStyle_;
}

/**
  * @param character [char]
  * @returns bool
  */
bool CharColorScheme::hasColorStyleFor(char character) const
{
    return charColorStyles_.contains(character);
}

/**
  * @param character [char]
  * @param textColorStyle [const TextColorStyle &]
  */
void CharColorScheme::setTextColorStyle(char character, const TextColorStyle &textColorStyle)
{
    charColorStyles_.insert(character, textColorStyle);
}

/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
void CharColorScheme::setDefaultTextColorStyle(const TextColorStyle &defaultTextColorStyle)
{
    defaultTextColorStyle_ = defaultTextColorStyle;
}

/**
  * @param character [char]
  * @returns TextColorStyle
  */
TextColorStyle CharColorScheme::textColorStyle(char character) const
{
     if (charColorStyles_.contains(character))
         return charColorStyles_.value(character);

     return defaultTextColorStyle_;
}
