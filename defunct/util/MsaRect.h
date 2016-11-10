/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSARECT_H
#define MSARECT_H

#include "Rect.h"
#include "global.h"

/**
  * MsaRect encapsulates the regular logic of handling 1-based Msa rectangles, which by definition always occupy at
  * least one unit of space.
  *
  * The following constraints:
  * o No coordinate may be negative at any time (assertion enforced)
  * o A valid MsaRect has all coordinates greater than or equal to one
  * o width and height may never be zero (although the underlying x2 - x1 and y2 - y1 may equal zero)
  *
  * The width and height are interpreted as the normal Rect width and height but plus one. Therefore, the msa
  * rectangle spanning (1,1) -> (1,1) has a width of 1. These conventions must be followed when setting the widths and
  * heights.
  *
  * Setting a negative width and/or height in the constructor is never allowed.
  *
  * It is possible to get unreliable results if the rectangle is not valid. For instance, given a default invalid
  * rectangle. After calling setLeft(1) the x position is at 1; however, the width will be reported as -2 (right - left
  * - 1 = 0 - 1 - 1).
  *
  * Another major difference from Rect is that the MsaRect intersects and intersection methods are based upon any
  * overlapping data point rather than an inte
  */
class MsaRect : public Rect
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    MsaRect();                                                          //!< Trivial constructor of null instance
    MsaRect(const QPoint &topLeft, const QPoint &bottomRight);          //!< Construct a rectangle from topLeft to bottomRight
    MsaRect(const QPoint &topLeft, const QSize &size);                  //!< Construct a rectangle originating at topLeft and with size
    MsaRect(int x, int y, int width, int height);                       //!< Construct a rectangle at (x,y) and the given width and height
    MsaRect(const Rect &rect);                                          //!< Convert a rectangle into a MsaRect

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int height() const;                                                 //!< Returns the height of MsaRect
    bool isValid() const;                                               //!< Returns true if all coordinates are greater than zero
    MsaRect intersection(const MsaRect &rect) const;                    //!< Returns the normalized intersection of this rectangle with rect
    bool intersects(const MsaRect &rect) const;                         //!< Returns true if both rects are valid and at least one unit of area in rect; false otherwise. Touching rectangles are considered to intersect along their touch points
    void moveBottom(int y);                                             //!< Moves the bottom edge to y without changing the height
    void moveLeft(int x);                                               //!< Moves the left edge to x without changing the width
    void moveRight(int x);                                              //!< Moves the right edge to x without changing the width
    void moveTop(int y);                                                //!< Moves the top edge to y without changing the height
    MsaRect normalized() const;                                         //!< Returns a normalized rectangle (one without a negative width or height)
    void setBottom(int y);                                              //!< Sets the bottom edge to y without changing the top edge (although the height may change)
    void setHeight(int height);                                         //!< Sets the rectangle height to height and moves the bottom edge as necessary; however, the top will not be affected
    void setLeft(int x);                                                //!< Sets the left edge to x without changing the right edge (although the width may change)
    void setRect(int x, int y, int width, int height);                  //!< Sets the rectangle to (x,y) and with width and height
    void setRect(const QPoint &topLeft, const QPoint &bottomRight);     //!< Sets the rectangle from topLeft to bottomRight
    void setRight(int x);                                               //!< Sets the right edge to x without changing the left edge (although the width may change)
    void setSize(const QSize &size);                                    //!< Sets the size to size; changes the bottom and right edges without changing the top left point
    void setTop(int y);                                                 //!< Sets the top edge to y without changing the bottom edge (although the height may change)
    void setWidth(int width);                                           //!< Sets the width to width and moves the right edge as necessary; however, the left will not be affected
    void setX(int x);                                                   //!< Sets the top left x-coordinate to x
    void setY(int y);                                                   //!< Sets the top left y-coordinate to y
    QSize size() const;                                                 //!< Returns the current MsaRect size
    int width() const;                                                  //!< Returns the rectangle width
};

Q_DECLARE_METATYPE(MsaRect)
Q_DECLARE_TYPEINFO(MsaRect, Q_MOVABLE_TYPE);




// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
inline
MsaRect::MsaRect() : Rect()
{
}

/**
  * @param topLeft [const QPoint &]
  * @param bottomRight [const QPoint &]
  */
inline
MsaRect::MsaRect(const QPoint &topLeft, const QPoint &bottomRight) :
    Rect(topLeft, bottomRight)
{
    ASSERT_X(topLeft.x() >= 0, "topLeft.x() must be greater than or equal to 0");
    ASSERT_X(topLeft.y() >= 0, "topLeft.y() must be greater than or equal to 0");
    ASSERT_X(bottomRight.x() >= 0, "bottomRight.x() must be greater than or equal to 0");
    ASSERT_X(bottomRight.y() >= 0, "bottomRight.y() must be greater than or equal to 0");
}

/**
  * @param topLeft [const QPoint &]
  * @param size [const QSize &]
  */
inline
MsaRect::MsaRect(const QPoint &topLeft, const QSize &size) :
    Rect(topLeft, size + QSize(-1, -1))
{
    ASSERT_X(topLeft.x() >= 0, "topLeft.x() must be greater than or equal to 0");
    ASSERT_X(topLeft.y() >= 0, "topLeft.y() must be greater than or equal to 0");
    ASSERT_X(size.width() > 0, "size.width() must be positive");
    ASSERT_X(size.height() > 0, "size.height() must be positive");
    ASSERT_X(x2_ >= 0, "topLeft().x() + size.width() must be greater than or equal to 0");
    ASSERT_X(y2_ >= 0, "topLeft().y() + size.height() must be greater than or equal to 0");
}

/**
  * @param x [int]
  * @param y [int]
  * @param width [int]
  * @param height [int]
  */
inline
MsaRect::MsaRect(int x, int y, int width, int height) :
    Rect(x, y, width-1, height-1)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to 0");
    ASSERT_X(y >= 0, "y must be greater than or equal to 0");
    ASSERT_X(width > 0, "width must be positive");
    ASSERT_X(height > 0, "height must be positive");
    ASSERT_X(x2_ >= 0, "x + width must be greater than or equal to 0");
    ASSERT_X(y2_ >= 0, "y + height must be greater than or equal to 0");
}

/**
  * @param rect [const Rect &]
  */
inline
MsaRect::MsaRect(const Rect &rect) :
    Rect(rect)
{
    ASSERT_X(rect.topLeft().x() >= 0, "rect.topLeft.x() must be greater than or equal to 0");
    ASSERT_X(rect.topLeft().y() >= 0, "rect.topLeft.y() must be greater than or equal to 0");
    ASSERT_X(rect.bottomRight().x() >= 0, "rect.bottomRight.x() must be greater than or equal to 0");
    ASSERT_X(rect.bottomRight().y() >= 0, "rect.bottomRight.y() must be greater than or equal to 0");
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * A MsaRect differs from a regular Rect in that it returns the Rect height + 1 (if normal, - 1 otherwise). It is not
  * possible to get a height of -1; only -2 and beyond.
  *
  * @returns int
  */
inline
int MsaRect::height() const
{
    return (y2_ >= y1_) ? y2_ - y1_ + 1 : y2_ - y1_ - 1;
}

/**
  * @returns bool
  */
inline
bool MsaRect::isValid() const
{
    return x1_ > 0 &&
            y1_ > 0 &&
            x2_ > 0 &&
            y2_ > 0;
}

/**
  * @param rect [const MsaRect &]
  * @returns MsaRect
  */
inline
MsaRect MsaRect::intersection(const MsaRect &rect) const
{
    if (!intersects(rect))
        return MsaRect();

    MsaRect r1 = normalized();
    MsaRect r2 = rect.normalized();

    return MsaRect(QPoint(qMax(r1.x1_, r2.x1_), qMax(r1.y1_, r2.y1_)),
                   QPoint(qMin(r1.x2_, r2.x2_), qMin(r1.y2_, r2.y2_)));
}

/**
  * @param rect [const MsaRect &]
  * @returns bool
  */
inline
bool MsaRect::intersects(const MsaRect &rect) const
{
    if (!isValid() || !rect.isValid())
        return false;

    MsaRect r1 = normalized();
    MsaRect r2 = rect.normalized();

    return *this == rect || (r2.x1_ <= r1.x2_ &&
                             r2.x2_ >= r1.x1_ &&
                             r2.y1_ <= r1.y2_ &&
                             r2.y2_ >= r1.y1_);
}

/**
  * @param y [int]
  */
inline
void MsaRect::moveBottom(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::moveBottom(y);
    ASSERT_X(y1_ >= 0, "resulting topLeft().y() must be greater than or equal to zero");
}

/**
  * @param x [int]
  */
inline
void MsaRect::moveLeft(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::moveLeft(x);
}

/**
  * @param x [int]
  */
inline
void MsaRect::moveRight(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::moveRight(x);
    ASSERT_X(x1_ >= 0, "resulting topLeft().x() must be greater than or equal to zero");
}

/**
  * @param y [int]
  */
inline
void MsaRect::moveTop(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::moveTop(y);
}

/**
  * @returns MsaRect
  */
inline
MsaRect MsaRect::normalized() const
{
    return MsaRect(Rect::normalized());
}

/**
  * @param y [int]
  */
inline
void MsaRect::setBottom(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setBottom(y);
}

/**
  * @param height [int]
  */
inline
void MsaRect::setHeight(int height)
{
    ASSERT_X(height != 0, "height must not be zero");
    if (height > 0)
        Rect::setHeight(height-1);
    else
        Rect::setHeight(height+1);
    ASSERT_X(y2_ >= 0, "resulting bottomRight.y() must be greater than or equal to zero");
}

/**
  * @param x [int]
  */
inline
void MsaRect::setLeft(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::setLeft(x);
}

/**
  * @param x [int]
  * @param y [int]
  * @param width [int]
  * @param height [int]
  */
inline
void MsaRect::setRect(int x, int y, int width, int height)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    ASSERT_X(height != 0, "height must not be zero");
    ASSERT_X(width != 0, "width must not be zero");
    ASSERT_X(x + width >= 0, "x + width must be greater than or equal to zero");
    ASSERT_X(y + height >= 0, "y + height must be greater than or equal to zero");
    Rect::setRect(x, y,
                  (width > 0) ? width - 1 : width + 1,
                  (height > 0) ? height - 1 : height + 1);
}

/**
  * @param topLeft [const QPoint &]
  * @param bottomRight [const QPoint &]
  */
inline
void MsaRect::setRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    ASSERT_X(topLeft.x() >= 0, "topLeft.x() must be greater than or equal to 0");
    ASSERT_X(topLeft.y() >= 0, "topLeft.y() must be greater than or equal to 0");
    ASSERT_X(bottomRight.x() >= 0, "bottomRight.x() must be greater than or equal to 0");
    ASSERT_X(bottomRight.y() >= 0, "bottomRight.y() must be greater than or equal to 0");
    Rect::setRect(topLeft, bottomRight);
}

/**
  * @param x [int]
  */
inline
void MsaRect::setRight(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::setRight(x);
}

/**
  * @param size [const QSize &]
  */
inline
void MsaRect::setSize(const QSize &size)
{
    ASSERT_X(size.width() > 0, "size.width() must be positive");
    ASSERT_X(size.height() > 0, "size.height() must be positive");

    Rect::setSize(size + QSize(-1, -1));

    ASSERT_X(x2_ >= 0, "resulting bottomRight.x() must be greater than or equal to zero");
    ASSERT_X(y2_ >= 0, "resulting bottomRight.y() must be greater than or equal to zero");
}

/**
  * @param y [int]
  */
inline
void MsaRect::setTop(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setTop(y);
}

/**
  * @param width [int]
  */
inline
void MsaRect::setWidth(int width)
{
    ASSERT_X(width != 0, "width must not be zero");
    if (width > 0)
        Rect::setWidth(width-1);
    else
        Rect::setWidth(width+1);
    ASSERT_X(x2_ >= 0, "resulting bottomRight.x() must be greater than or equal to zero");
}

/**
  * @param x [int]
  */
inline
void MsaRect::setX(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::setLeft(x);
}

/**
  * @param y [int]
  */
inline
void MsaRect::setY(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setTop(y);
}

/**
  * @returns QSize
  */
inline
QSize MsaRect::size() const
{
    return QSize(width(), height());
}

/**
  * A MsaRect differs from a regular Rect in that it returns the Rect width + 1 (if normal, - 1 otherwise). It is not
  * possible to get a width of -1; only -2 and beyond.
  *
  * @returns int
  */
inline
int MsaRect::width() const
{
    return (x2_ >= x1_) ? x2_ - x1_ + 1 : x2_ - x1_ - 1;
}

#endif // MSARECT_H
