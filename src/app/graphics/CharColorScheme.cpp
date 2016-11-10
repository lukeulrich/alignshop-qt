/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "CharColorScheme.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
CharColorScheme::CharColorScheme(const TextColorStyle &defaultTextColorStyle)
    : defaultTextColorStyle_(defaultTextColorStyle)
{
    memset(definedStyles_, 0, 128 * sizeof(bool));

    for (int i=0; i< 128; ++i)
        charColorStyles_[i] = defaultTextColorStyle_;
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

    if (defaultTextColorStyle_ != other.defaultTextColorStyle_)
        return false;

    // Default color is the same check that all defined colors are the same
    for (int i=0; i< 128; ++i)
    {
        if (definedStyles_[i] != other.definedStyles_[i])
            return false;

        if (definedStyles_[i] && charColorStyles_[i] != other.charColorStyles_[i])
            return false;
    }

    return true;

//    return defaultTextColorStyle_ == other.defaultTextColorStyle_
//           && charColorStyles_ == other.charColorStyles_;
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
    ASSERT(character >= 0);
    if (character < 0)
        return false;

    return definedStyles_[static_cast<int>(character)];
}

/**
  * @param character [char]
  * @param textColorStyle [const TextColorStyle &]
  */
void CharColorScheme::setTextColorStyle(char character, const TextColorStyle &textColorStyle)
{
    ASSERT(character >= 0);
    if (character < 0)
        return;

    definedStyles_[static_cast<int>(character)] = true;
    charColorStyles_[static_cast<int>(character)] = textColorStyle;
}

/**
  * @param defaultTextColorStyle [const TextColorStyle &]
  */
void CharColorScheme::setDefaultTextColorStyle(const TextColorStyle &defaultTextColorStyle)
{
    defaultTextColorStyle_ = defaultTextColorStyle;

    for (int i=0; i< 128; ++i)
        if (!definedStyles_[i])
            charColorStyles_[i] = defaultTextColorStyle_;
}

/**
  * @param character [char]
  * @returns TextColorStyle
  */
TextColorStyle CharColorScheme::textColorStyle(char character) const
{
    ASSERT(character >= 0);
    if (character < 0)
        return defaultTextColorStyle_;

    return charColorStyles_[static_cast<int>(character)];
}
