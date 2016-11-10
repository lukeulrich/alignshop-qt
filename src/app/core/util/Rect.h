/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RECT_H
#define RECT_H

#include <QtCore/QMetaType>
#include <QtCore/QPoint>
#include <QtCore/QRectF>
#include <QtCore/QSize>

/**
  * Rect defines a mathematically correct rectangular representation in a plane using integer precision.
  *
  * Unfortunately, the QRect class has a funky implementation for historical reasons and several of its methods return
  * unexpected values. For instance, the width function returns x2 - x1 + 1. Thus, by definition all rectangles have a
  * width of at least one -- even if x1 == x2.
  *
  * Rect as nearly as possible attempts to mirror the QRect interface but properly deals with width's and heights and
  * the setting of variables.
  *
  * All getter methods are documented with respect to a proper rectangle. Obviously, if the rectangle is not normal,
  * the actual names may be incorrect in an absolute sense.
  */
class Rect
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    Rect();                                                             //!< Construct a default rectangle from (0,0) -> (0, 0)
    Rect(const QPoint &topLeft, const QPoint &bottomRight);             //!< Construct a rectangle from topLeft to bottomRight
    Rect(const QPoint &topLeft, const QSize &size);                     //!< Construct a rectangle originating at topLeft and with size
    Rect(int x, int y, int width, int height);                          //!< Construct a rectangle at (x,y) and the given width and height

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator!=(const Rect &other) const;                           //!< Retruns true if other is not identical to this rectangle
    bool operator==(const Rect &other) const;                           //!< Returns true if other is identical to this rectangle

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int bottom() const;                                                 //!< Returns the bottom edge
    QPoint bottomLeft() const;                                          //!< Returns the bottom left point
    QPoint bottomRight() const;                                         //!< Returns the bottom right point
    QPoint center() const;                                              //!< Returns the floored center point
    QPointF centerF() const;                                            //!< Returns the exact center point without rounding down
    bool contains(const QPoint &point, bool proper = false) const;      //!< Returns true if point is within or on the rectangle's edges (proper = false) or if point is fully inside the rectangle (proper = true); otherwise returns false
    bool contains(int x, int y, bool proper = false) const;             //!< Returns true if x and y are within or on the rectangle's edges (proper = false) or if x and y are fully inside the rectangle (proper = true); otherwise returns false
    bool contains(const Rect &rect, bool proper = false) const;         //!< Returns true if rect is within or on the the rectangle's edges (proper = false) or if rect is fully inside the rectangle (proper = true); otherwise returns false
    int height() const;                                                 //!< Returns the rectangle's height: y2 - y1
    Rect intersection(const Rect &rect) const;                          //!< Returns the normalized intersection of this rectangle with rect
    bool intersects(const Rect &rect) const;                            //!< Returns true if rect intersects at least one integral unit of area in rect; false otherwise. Touching rectangles are not considered to intersect
    bool isNull() const;                                                //!< Returns true if the width and height are zero
    int left() const;                                                   //!< Returns the left edge; equivalent to x()
    void moveBottom(int y);                                             //!< Moves the bottom edge to y without changing the height
    void moveLeft(int x);                                               //!< Moves the left edge to x without changing the width
    void moveRight(int x);                                              //!< Moves the right edge to x without changing the width
    void moveTop(int y);                                                //!< Moves the top edge to y without changing the height
    Rect normalized() const;                                            //!< Returns a normalized rectangle (one without a negative width or height)
    int right() const;                                                  //!< Returns the right edge
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
    QSize size() const;                                                 //!< Returns the size of this rectangle
    QRectF toQRectF() const;                                            //!< Converts this rect to its floating point equivalent
    int top() const;                                                    //!< Returns the top edge; equivalent to y()
    QPoint topLeft() const;                                             //!< Returns the top left point
    QPoint topRight() const;                                            //!< Returns the top right point
    int width() const;                                                  //!< Returns the rectangle width
    int x() const;                                                      //!< Returns the top left x-coordinate; equivalent to left()
    int y() const;                                                      //!< Returns the top left y-coordinate; equivalent to top()

protected:
    int x1_;
    int y1_;
    int x2_;
    int y2_;

    friend QDataStream &operator<<(QDataStream &stream, const Rect &rect);
    friend QDataStream &operator>>(QDataStream &stream, Rect &rect);
};

Q_DECLARE_METATYPE(Rect)
Q_DECLARE_TYPEINFO(Rect, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const Rect &rectangle);
#endif
QDataStream &operator<<(QDataStream &stream, const Rect &rect);
QDataStream &operator>>(QDataStream &stream, Rect &rect);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Inline definitions

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static methods

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  */
inline
Rect::Rect() : x1_(0), y1_(0), x2_(0), y2_(0)
{
}

/**
  * @param topLeft [const QPoint &]
  * @param bottomRight [const QPoint &]
  */
inline
Rect::Rect(const QPoint &topLeft, const QPoint &bottomRight)
    : x1_(topLeft.x()), y1_(topLeft.y()), x2_(bottomRight.x()), y2_(bottomRight.y())
{
}

/**
  * @param topLeft [const QPoint &]
  * @param size [const QSize &]
  */
inline
Rect::Rect(const QPoint &topLeft, const QSize &size)
    : x1_(topLeft.x()), y1_(topLeft.y()), x2_(x1_ + size.width()), y2_(y1_ + size.height())
{
}

/**
  * @param x [int]
  * @param y [int]
  * @param width [int]
  * @param height [int]
  */
inline
Rect::Rect(int x, int y, int width, int height)
    : x1_(x), y1_(y), x2_(x1_ + width), y2_(y1_ + height)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const Rect &]
  * @returns bool
  */
inline
bool Rect::operator!=(const Rect &other) const
{
    return !(*this == other);
}

/**
  * @param other [const Rect &]
  * @returns bool
  */
inline
bool Rect::operator==(const Rect &other) const
{
    return (this == &other || (x1_ == other.x1_ && y1_ == other.y1_ && x2_ == other.x2_ && y2_ == other.y2_));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
inline
int Rect::bottom() const
{
    return y2_;
}

/**
  * @returns QPoint
  */
inline
QPoint Rect::bottomLeft() const
{
    return QPoint(x1_, y2_);
}

/**
  * @returns QPoint
  */
inline
QPoint Rect::bottomRight() const
{
    return QPoint(x2_, y2_);
}

/**
  * @returns QPoint
  */
inline
QPoint Rect::center() const
{
    return QPoint(x1_ + (x2_ - x1_) / 2,
                  y1_ + (y2_ - y1_) / 2);
}

/**
  * @returns QPointF
  */
inline
QPointF Rect::centerF() const
{
    return QPointF(x1_ + (x2_ - x1_) / 2.,
                   y1_ + (y2_ - y1_) / 2.);
}

/**
  * @param point [const QPoint &]
  * @param proper [bool]
  * @returns bool
  */
inline
bool Rect::contains(const QPoint &point, bool proper) const
{
    return contains(point.x(), point.y(), proper);
}

/**
  * @param x [int]
  * @param y [int]
  * @param proper [bool]
  * @returns bool
  */
inline
bool Rect::contains(int x, int y, bool proper) const
{
    if (proper == false)
    {
        return (x >= x1_
                && x <= x2_
                && y >= y1_
                && y <= y2_);
    }

    // Proper = true; point must lie completely within the rectangle bounds
    return (x > x1_
            && x < x2_
            && y > y1_
            && y < y2_);
}

/**
  * @param rect [const Rect &]
  * @param proper [bool]
  * @returns bool
  */
inline
bool Rect::contains(const Rect &rect, bool proper) const
{
    return contains(rect.x1_, rect.y1_, proper) && contains(rect.x2_, rect.y2_, proper);
}

/**
  * @returns int
  */
inline
int Rect::height() const
{
    return y2_ - y1_;
}

/**
  * If no intersection is found, a null rectangle is returned.
  *
  * @param rect [const Rect &]
  * @returns Rect
  */
inline
Rect Rect::intersection(const Rect &rect) const
{
    if (!intersects(rect))
        return Rect();

    Rect r1 = normalized();
    Rect r2 = rect.normalized();

    return Rect(QPoint(qMax(r1.x1_, r2.x1_), qMax(r1.y1_, r2.y1_)),
                QPoint(qMin(r1.x2_, r2.x2_), qMin(r1.y2_, r2.y2_)));
}

/**
  * @param rect [const Rect &]
  * @returns bool
  */
inline
bool Rect::intersects(const Rect &rect) const
{
    // Check that both rectangles have at least one integral unit of area to overlap with
    if (width() == 0 || height() == 0 ||
            rect.width() == 0 || rect.height() == 0)
    {
        return false;
    }

    Rect r1 = normalized();
    Rect r2 = rect.normalized();

    return *this == rect || (r2.x1_ < r1.x2_ &&
                             r2.x2_ > r1.x1_ &&
                             r2.y1_ < r1.y2_ &&
                             r2.y2_ > r1.y1_);
}

/**
  * @returns bool
  */
inline
bool Rect::isNull() const
{
    return width() == 0 && height() == 0;
}

/**
  * @returns int
  */
inline
int Rect::left() const
{
    return x1_;
}

/**
  * @param y [int]
  */
inline
void Rect::moveBottom(int y)
{
    y1_ = y - height();
    y2_ = y;
}

/**
  * @param x [int]
  */
inline
void Rect::moveLeft(int x)
{
    x2_ = x + width();
    x1_ = x;
}

/**
  * @param x [int]
  */
inline
void Rect::moveRight(int x)
{
    x1_ = x - width();
    x2_ = x;
}

/**
  * @param y [int]
  */
inline
void Rect::moveTop(int y)
{
    y2_ = y + height();
    y1_ = y;
}

/**
  * @returns Rect
  */
inline
Rect Rect::normalized() const
{
    const int *x1 = &x1_;
    const int *x2 = &x2_;
    if (x2_ < x1_)
    {
        x1 = &x2_;
        x2 = &x1_;
    }

    const int *y1 = &y1_;
    const int *y2 = &y2_;
    if (y2_ < y1_)
    {
        y1 = &y2_;
        y2 = &y1_;
    }

    return Rect(QPoint(*x1, *y1), QPoint(*x2, *y2));
}

/**
  * @returns int
  */
inline
int Rect::right() const
{
    return x2_;
}

/**
  * @param y [int]
  */
inline
void Rect::setBottom(int y)
{
    y2_ = y;
}

/**
  * @param height [int]
  */
inline
void Rect::setHeight(int height)
{
    y2_ = y1_ + height;
}

/**
  * @param x [int]
  */
inline
void Rect::setLeft(int x)
{
    x1_ = x;
}

/**
  * @param x [int]
  * @param y [int]
  * @param width [int]
  * @param height [int]
  */
inline
void Rect::setRect(int x, int y, int width, int height)
{
    x1_ = x;
    y1_ = y;
    x2_ = x1_ + width;
    y2_ = y1_ + height;
}

/**
  * @param topLeft [const QPoint &]
  * @param bottomRight [const QPoint &]
  */
inline
void Rect::setRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    x1_ = topLeft.x();
    y1_ = topLeft.y();
    x2_ = bottomRight.x();
    y2_ = bottomRight.y();
}

/**
  * @param x [int]
  */
inline
void Rect::setRight(int x)
{
    x2_ = x;
}

/**
  * @param size [const QSize &]
  */
inline
void Rect::setSize(const QSize &size)
{
    x2_ = x1_ + size.width();
    y2_ = y1_ + size.height();
}

/**
  * @param y [int]
  */
inline
void Rect::setTop(int y)
{
    y1_ = y;
}

/**
  * @param width [int]
  */
inline
void Rect::setWidth(int width)
{
    x2_ = x1_ + width;
}

/**
  * @param x [int]
  */
inline
void Rect::setX(int x)
{
    x1_ = x;
}

/**
  * @param y [int]
  */
inline
void Rect::setY(int y)
{
    y1_ = y;
}

/**
  * @returns QSize
  */
inline
QSize Rect::size() const
{
    return QSize(width(), height());
}

/**
  * @returns QRectF
  */
inline
QRectF Rect::toQRectF() const
{
    return QRectF(QPointF(x1_, y1_), QPointF(x2_, y2_));
}

/**
  * @returns int
  */
inline
int Rect::top() const
{
    return y1_;
}

/**
  * @returns QPoint
  */
inline
QPoint Rect::topLeft() const
{
    return QPoint(x1_, y1_);
}

/**
  * @returns QPoint
  */
inline
QPoint Rect::topRight() const
{
    return QPoint(x2_, y1_);
}

/**
  * @returns int
  */
inline
int Rect::width() const
{
    return x2_ - x1_;
}

/**
  * @returns int
  */
inline
int Rect::x() const
{
    return x1_;
}

/**
  * @returns int
  */
inline
int Rect::y() const
{
    return y1_;
}

#endif // RECT_H
