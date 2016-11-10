/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TEXTCOLORSTYLE_H
#define TEXTCOLORSTYLE_H

#include <QtGui/QColor>

/**
  * TextColorStyle simply composes foreground and background color variables in a single data structure.
  *
  * Unless intialized otherwise, the default foreground color is black and the default background color is white.
  */
struct TextColorStyle
{
    QColor foreground_;
    QColor background_;

    // ------------------------------------------------------------------------------------------------
    // Constructor
    /**
      * Construct an instance with foreground (default black) and background (default white) colors.
      *
      * @param foreground [QColor]
      * @param background [QColor]
      */
    TextColorStyle(const QColor &foreground = Qt::black, const QColor &background = Qt::white) :
        foreground_(foreground), background_(background)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // Operators
    /**
      * Returns true if all private members are equivalent; false otherwise.
      *
      * @param other [const TextColorStyle &]
      * @returns bool
      */
    bool operator==(const TextColorStyle &other) const
    {
        if (this == &other)
            return true;

        return foreground_ == other.foreground_ && background_ == other.background_;
    }

    /**
      * Returns true if one or more private members are not equivalent; false otherwise.
      *
      * @param other [const TextColorStyle &]
      * @returns bool
      */
    bool operator!=(const TextColorStyle &other) const
    {
        return !operator==(other);
    }
};

Q_DECLARE_TYPEINFO(TextColorStyle, Q_MOVABLE_TYPE);

#endif // TEXTCOLORSTYLE_H
