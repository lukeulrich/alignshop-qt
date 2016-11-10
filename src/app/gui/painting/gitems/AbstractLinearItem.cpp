/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractLinearItem.h"
#include "../../../core/macros.h"

//#include <QtDebug>
//#include <typeinfo>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QRectF
  */
QRectF AbstractLinearItem::boundingRect() const
{
//    qDebug() << Q_FUNC_INFO << typeid(*this).name() << width() << height();

    return QRectF(0, 0, width(), height());
}

double AbstractLinearItem::halfHeight() const
{
    return height() * .5;
}

/**
  * @param unit [const int]
  * @returns double
  */
double AbstractLinearItem::leftPositionFromUnit(const int unit) const
{
    return (unit - 1) * pixelsPerUnit_;
}

/**
  * @returns int
  */
int AbstractLinearItem::length() const
{
    return length_;
}

/**
  * @param unit [const int]
  * @returns double
  */
double AbstractLinearItem::middlePositionFromUnit(const int unit) const
{
    return leftPositionFromUnit(unit) + pixelsPerUnit_ / 2.;
}

/**
  * @param painter [QPainter *]
  * @param option [const QStyleOptionGraphicsItem *]
  * @param widget [QWidget *]
  */
void AbstractLinearItem::paint(QPainter * /* painter */,
                               const QStyleOptionGraphicsItem * /* option */,
                               QWidget * /* widget */)
{
}

/**
  * @returns double
  */
double AbstractLinearItem::pixelsPerUnit() const
{
    return pixelsPerUnit_;
}

/**
  * @param unit [const int]
  * @returns double
  */
double AbstractLinearItem::rightPositionFromUnit(const int unit) const
{
    return unit * pixelsPerUnit_;
}

/**
  * @param position [const double]
  * @returns int
  */
int AbstractLinearItem::unitFromPosition(const double position) const
{
    return 1 + (position / pixelsPerUnit_);
}

/**
  * @returns double
  */
double AbstractLinearItem::unitsPerPixel() const
{
    return 1. / pixelsPerUnit_;
}

/**
  * @returns double
  */
double AbstractLinearItem::width() const
{
    return length_ * pixelsPerUnit_;
}

/**
  * @param units [const int]
  * @returns double
  */
double AbstractLinearItem::width(const int units) const
{
    return units * pixelsPerUnit_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newPixelsPerUnit [const double]
  */
void AbstractLinearItem::setPixelsPerUnit(const double newPixelsPerUnit)
{
    ASSERT(newPixelsPerUnit > 0.);
    if (qFuzzyCompare(newPixelsPerUnit, pixelsPerUnit_))
        return;

    prepareGeometryChange();
    pixelsPerUnit_ = newPixelsPerUnit;
    onPixelsPerUnitChanged();
    emit pixelsPerUnitChanged(pixelsPerUnit_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected constructor
/**
  * @param length [int]
  * @param parentItem [QGraphicsItem *]
  */
AbstractLinearItem::AbstractLinearItem(int length, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem),
      length_(length),
      pixelsPerUnit_(1.)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void AbstractLinearItem::onPixelsPerUnitChanged()
{
}
