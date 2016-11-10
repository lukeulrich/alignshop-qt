/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef POSIRECT_H
#define POSIRECT_H

#include "ClosedIntRange.h"
#include "Rect.h"
#include "../macros.h"

/**
  * PosiRect encapsulates the regular logic of handling 1-based rectangles, which by definition always occupy at least
  * one unit of space.
  *
  * The following constraints:
  * o No coordinate may be negative at any time (assertion enforced)
  * o A valid PosiRect has all coordinates greater than or equal to one
  * o width and height may never be zero (although the underlying x2 - x1 and y2 - y1 may equal zero)
  *
  * The width and height are interpreted as the normal Rect width and height but plus one. Therefore, the rectangle
  * spanning (1,1) -> (1,1) has a width of 1. These conventions must be followed when setting the widths and heights.
  *
  * Setting a negative width and/or height in the constructor is never allowed.
  *
  * It is possible to get unreliable results if the rectangle is not valid. For instance, given a default invalid
  * rectangle. After calling setLeft(1) the x position is at 1; however, the width will be reported as -2 (right - left
  * - 1 = 0 - 1 - 1).
  *
  * Another major difference from Rect is that the PosiRect intersects and intersection methods are based upon any
  * overlapping data point.
  */
class PosiRect : public Rect
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    PosiRect();                                                          //!< Trivial constructor of null instance
    PosiRect(const QPoint &topLeft, const QPoint &bottomRight);          //!< Construct a rectangle from topLeft to bottomRight
    PosiRect(const QPoint &topLeft, const QSize &size);                  //!< Construct a rectangle originating at topLeft and with size
    PosiRect(int x, int y, int width, int height);                       //!< Construct a rectangle at (x,y) and the given width and height
    PosiRect(const Rect &rect);                                          //!< Convert a rectangle into a PosiRect

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int height() const;                                                 //!< Returns the height of PosiRect
    ClosedIntRange horizontalRange() const;                             //!< Convenience method for obtaining the horizontal extents as a ClosedIntRange
    bool isValid() const;                                               //!< Returns true if all coordinates are greater than zero
    PosiRect intersection(const PosiRect &rect) const;                  //!< Returns the normalized intersection of this rectangle with rect
    bool intersects(const PosiRect &rect) const;                        //!< Returns true if both rects are valid and at least one unit of area in rect; false otherwise. Touching rectangles are considered to intersect along their touch points
    bool isNull() const;                                                //!< Returns true if the width and height are both zero; false otherwise
    void moveBottom(int y);                                             //!< Moves the bottom edge to y without changing the height
    void moveLeft(int x);                                               //!< Moves the left edge to x without changing the width
    void moveRight(int x);                                              //!< Moves the right edge to x without changing the width
    void moveTop(int y);                                                //!< Moves the top edge to y without changing the height
    PosiRect normalized() const;                                        //!< Returns a normalized rectangle (one without a negative width or height)
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
    QSize size() const;                                                 //!< Returns the current PosiRect size
    ClosedIntRange verticalRange() const;                               //!< Convenience method for obtaining the vertical extents as a ClosedIntRange
    int width() const;                                                  //!< Returns the rectangle width
};

Q_DECLARE_TYPEINFO(PosiRect, Q_MOVABLE_TYPE);


#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const Rect &rectangle);
#endif
QDataStream &operator<<(QDataStream &stream, const PosiRect &posiRect);
QDataStream &operator>>(QDataStream &stream, PosiRect &posiRect);



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
inline
PosiRect::PosiRect() : Rect()
{
}

/**
  * @param topLeft [const QPoint &]
  * @param bottomRight [const QPoint &]
  */
inline
PosiRect::PosiRect(const QPoint &topLeft, const QPoint &bottomRight) :
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
PosiRect::PosiRect(const QPoint &topLeft, const QSize &size) :
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
PosiRect::PosiRect(int x, int y, int width, int height) :
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
PosiRect::PosiRect(const Rect &rect) :
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
  * A PosiRect differs from a regular Rect in that it returns the Rect height + 1 (if normal, - 1 otherwise). It is not
  * possible to get a height of -1; only -2 and beyond.
  *
  * @returns int
  */
inline
int PosiRect::height() const
{
    return (y2_ >= y1_) ? y2_ - y1_ + 1 : y2_ - y1_ - 1;
}

/**
  * @returns ClosedIntRange
  */
inline
ClosedIntRange PosiRect::horizontalRange() const
{
    return ClosedIntRange(x1_, x2_);
}

/**
  * @returns bool
  */
inline
bool PosiRect::isNull() const
{
    return x1_ == 0 &&
           y1_ == 0 &&
           x2_ == 0 &&
           y2_ == 0;
}

/**
  * @returns bool
  */
inline
bool PosiRect::isValid() const
{
    return x1_ > 0 &&
           y1_ > 0 &&
           x2_ > 0 &&
           y2_ > 0;
}

/**
  * @param rect [const PosiRect &]
  * @returns PosiRect
  */
inline
PosiRect PosiRect::intersection(const PosiRect &rect) const
{
    if (!intersects(rect))
        return PosiRect();

    PosiRect r1 = normalized();
    PosiRect r2 = rect.normalized();

    return PosiRect(QPoint(qMax(r1.x1_, r2.x1_), qMax(r1.y1_, r2.y1_)),
                   QPoint(qMin(r1.x2_, r2.x2_), qMin(r1.y2_, r2.y2_)));
}

/**
  * @param rect [const PosiRect &]
  * @returns bool
  */
inline
bool PosiRect::intersects(const PosiRect &rect) const
{
    if (!isValid() || !rect.isValid())
        return false;

    PosiRect r1 = normalized();
    PosiRect r2 = rect.normalized();

    return *this == rect || (r2.x1_ <= r1.x2_ &&
                             r2.x2_ >= r1.x1_ &&
                             r2.y1_ <= r1.y2_ &&
                             r2.y2_ >= r1.y1_);
}

/**
  * @param y [int]
  */
inline
void PosiRect::moveBottom(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::moveBottom(y);
    ASSERT_X(y1_ >= 0, "resulting topLeft().y() must be greater than or equal to zero");
}

/**
  * @param x [int]
  */
inline
void PosiRect::moveLeft(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::moveLeft(x);
}

/**
  * @param x [int]
  */
inline
void PosiRect::moveRight(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::moveRight(x);
    ASSERT_X(x1_ >= 0, "resulting topLeft().x() must be greater than or equal to zero");
}

/**
  * @param y [int]
  */
inline
void PosiRect::moveTop(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::moveTop(y);
}

/**
  * @returns PosiRect
  */
inline
PosiRect PosiRect::normalized() const
{
    return PosiRect(Rect::normalized());
}

/**
  * @param y [int]
  */
inline
void PosiRect::setBottom(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setBottom(y);
}

/**
  * @param height [int]
  */
inline
void PosiRect::setHeight(int height)
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
void PosiRect::setLeft(int x)
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
void PosiRect::setRect(int x, int y, int width, int height)
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
void PosiRect::setRect(const QPoint &topLeft, const QPoint &bottomRight)
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
void PosiRect::setRight(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::setRight(x);
}

/**
  * @param size [const QSize &]
  */
inline
void PosiRect::setSize(const QSize &size)
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
void PosiRect::setTop(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setTop(y);
}

/**
  * @param width [int]
  */
inline
void PosiRect::setWidth(int width)
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
void PosiRect::setX(int x)
{
    ASSERT_X(x >= 0, "x must be greater than or equal to zero");
    Rect::setLeft(x);
}

/**
  * @param y [int]
  */
inline
void PosiRect::setY(int y)
{
    ASSERT_X(y >= 0, "y must be greater than or equal to zero");
    Rect::setTop(y);
}

/**
  * @returns QSize
  */
inline
QSize PosiRect::size() const
{
    return QSize(width(), height());
}

/**
  * @returns ClosedIntRange
  */
inline
ClosedIntRange PosiRect::verticalRange() const
{
    return ClosedIntRange(y1_, y2_);
}

/**
  * A PosiRect differs from a regular Rect in that it returns the Rect width + 1 (if normal, - 1 otherwise). It is not
  * possible to get a width of -1; only -2 and beyond.
  *
  * @returns int
  */
inline
int PosiRect::width() const
{
    return (x2_ >= x1_) ? x2_ - x1_ + 1 : x2_ - x1_ - 1;
}

#endif // POSIRECT_H
