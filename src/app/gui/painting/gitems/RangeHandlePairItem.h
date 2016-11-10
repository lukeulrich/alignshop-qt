/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RANGEHANDLEPAIRITEM_H
#define RANGEHANDLEPAIRITEM_H

#include <QtGui/QGraphicsItemGroup>
#include "../../../core/global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AbstractRangeItem;
class RangeHandleItem;


/**
  * RangeHandlePairItem composes a start and stop handle pair of objects and provides a wrapper interface for working
  * with both items through one interface.
  */
class RangeHandlePairItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    RangeHandlePairItem(AbstractRangeItem *targetRangeItem, double diameter, QGraphicsItem *parentItem = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    double bottomStemHeight() const;                            //!< Returns the bottom stem height
    double coreStemHeight() const;                              //!< Returns the core stem height
    double diameter() const;                                    //!< Returns the diameter of the ellipse handle
    double fadeRectOpacity() const;                             //!< Returns the opacity of the fade rect (default .75)
    double height() const;                                      //!< Returns diameter + total stem height
    void setTargetRangeItem(AbstractRangeItem *targetRangeItem);//!< Sets the target range item to targetRangeItem
    RangeHandleItem *startHandle() const;
    RangeHandleItem *stopHandle() const;
    double stemHeight() const;                                  //!< Returns the total stem height
    double topStemHeight() const;                               //!< Returns the top stem height


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setBottomStemHeight(double newBottomStemHeight);       //!< Sets the bottom stem height to newBottomStemHeight
    void setCoreStemHeight(double newCoreStemHeight);           //!< Sets the core stem height to newCoreStemHeight
    void setFadeRectOpacity(double newFadeRectOpacity);         //!< Sets the fade rect opacity to newFadeRectOpacity
    void setHandleIndex(const int newHandleIndex);              //!< Sets the unit modeled by this handle item to newHandleIndex if possible
    void setTopStemHeight(double newTopStemHeight);             //!< Sets the top stem height to newTopStemHeight


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    RangeHandleItem *startHandle_;
    RangeHandleItem *stopHandle_;
};

#endif // RANGEHANDLEPAIRITEM_H
