/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TEXTCOLORSTYLE_H
#define TEXTCOLORSTYLE_H

#include <QtGui/QColor>
#include <QtGlobal>

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
        return !(*this == other);
    }
};

Q_DECLARE_TYPEINFO(TextColorStyle, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
inline
QDebug operator<<(QDebug debug, const TextColorStyle &textColorStyle)
{
    debug.nospace() << "TextColorStyle([" << textColorStyle.foreground_ << textColorStyle.background_ << "])";
    return debug.maybeSpace();
}
#endif


/**
  * GLTextColorStyle simply composes foreground and background color variables in a single data structure that is
  * optimized for OpenGL usage.
  *
  * Unless intialized otherwise, the default foreground color is black and the default background color is white.
  */
struct GLTextColorStyle
{
    uchar foreground_[4];
    uchar background_[4];

    // ------------------------------------------------------------------------------------------------
    // Constructor
    /**
      * Construct an instance with foreground (default black) and background (default white) colors.
      *
      * @param foreground [QColor]
      * @param background [QColor]
      */
    GLTextColorStyle(uchar fgRed, uchar fgGreen, uchar fgBlue, uchar fgAlpha,
                     uchar bgRed, uchar bgGreen, uchar bgBlue, uchar bgAlpha)
    {
        foreground_[0] = fgRed;
        foreground_[1] = fgGreen;
        foreground_[2] = fgBlue;
        foreground_[3] = fgAlpha;

        background_[0] = bgRed;
        background_[1] = bgGreen;
        background_[2] = bgBlue;
        background_[3] = bgAlpha;
    }

    GLTextColorStyle(uchar fgRed = 0, uchar fgGreen = 0, uchar fgBlue = 0,
                     uchar bgRed = 255, uchar bgGreen = 255, uchar bgBlue = 255)
    {
        foreground_[0] = fgRed;
        foreground_[1] = fgGreen;
        foreground_[2] = fgBlue;
        foreground_[3] = 255;

        background_[0] = bgRed;
        background_[1] = bgGreen;
        background_[2] = bgBlue;
        background_[3] = 255;
    }

    // ------------------------------------------------------------------------------------------------
    // Operators
    /**
      * Returns true if all private members are equivalent; false otherwise.
      *
      * @param other [const TextColorStyle &]
      * @returns bool
      */
    bool operator==(const GLTextColorStyle &other) const
    {
        if (this == &other)
            return true;

        return foreground_[0] == other.foreground_[0]
               && foreground_[1] == other.foreground_[1]
               && foreground_[2] == other.foreground_[2]
               && foreground_[3] == other.foreground_[3]
               && background_[0] == other.background_[0]
               && background_[1] == other.background_[1]
               && background_[2] == other.background_[2]
               && background_[3] == other.background_[3];
    }

    /**
      * Returns true if one or more private members are not equivalent; false otherwise.
      *
      * @param other [const TextColorStyle &]
      * @returns bool
      */
    bool operator!=(const GLTextColorStyle &other) const
    {
        return !(*this == other);
    }
};

#endif // TEXTCOLORSTYLE_H
