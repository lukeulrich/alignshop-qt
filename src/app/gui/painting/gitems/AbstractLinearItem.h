/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTLINEARITEM_H
#define ABSTRACTLINEARITEM_H

#include <QtGui/QGraphicsItem>

/**
  * AbstractLinearItem encapsulates much of the basic functionality required to represent a two-dimensional graphical
  * item of an integral length rendered at an arbitrary resolution.
  *
  * All linear items must have a non-zero length which is represented in virtual units. All horizontal dimensions and
  * calculations perform mapping relative to a user-defined pixels per unit variable. By modifying the pixels per unit,
  * it is possible to increase the horizontal resolution.
  *
  * Notes:
  * o At present, the length is a static variable that may not be modified.
  * o All units are 1-based, but positions are 0-based.
  */
class AbstractLinearItem : public QObject,
                           public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    QRectF boundingRect() const;                                //!< Returns the rectangle encompassed by this linear item
    virtual double height() const = 0;                          //!< Returns the height
    double halfHeight() const;                                  //!< Returns half of the height
    double leftPositionFromUnit(const int unit) const;          //!< Maps unit to its left coordinate
    virtual int length() const;                                 //!< Returns the number of units represented by this item
    double middlePositionFromUnit(const int unit) const;        //!< Maps unit to its middle coordinate
    //! Subclasses should override this method as necessary to render the item; this implementation does nothing
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    double pixelsPerUnit() const;                               //!< Returns the number of pixels occupied by a single unit
    double rightPositionFromUnit(const int unit) const;         //!< Maps unit to its right coordinate
    int unitFromPosition(const double position) const;          //!< Maps position (pixel-coordinates) to its corresponding unit
    double unitsPerPixel() const;                               //!< Returns the number of units occupied by a single pixel; the inverse of pixelsPerUnit()
    double width() const;                                       //!< Returns the width in pixels
    double width(const int units) const;                        //!< Returns the width in pixels covered by units


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setPixelsPerUnit(const double newPixelsPerUnit);       //!< Sets the number of pixels per unit to newPixelsPerUnit


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void pixelsPerUnitChanged(double currentPixelsPerUnit);     //!< Emitted when the pixels per unit changes to currentPixelsPerUnit


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected constructor
    //! Constructs an item with length units (must be positive) and is a child of parent
    AbstractLinearItem(int length, QGraphicsItem *parentItem = 0);


    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void onPixelsPerUnitChanged();                      //!< Virtual stub called whenever the the pixels per unit has changed


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    int length_;                    //!< Number of units represented by this item
    double pixelsPerUnit_;          //!< Pixels per unit; default 1.
};

#endif // ABSTRACTLINEARITEM_H
