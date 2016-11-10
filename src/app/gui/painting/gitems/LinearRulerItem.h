/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LINEARRULERITEM_H
#define LINEARRULERITEM_H

#include "AbstractLinearItem.h"
#include "../../../core/global.h"

/**
  * LinearRulerItem displays a ruler item corresponding to a particular length with major and minor tick marks.
  *
  * Major tick marks are hard-coded at every 10 units and minor tick marks every 5 units.
  */
class LinearRulerItem : public AbstractLinearItem
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    LinearRulerItem(int length, QGraphicsItem *parentItem = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    double height() const;                                      //!< Returns the ruler height
    //! Paint the ruler
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool labelIncrementFits(const int labelIncrement) const;
    double minimumWidthBetweenLabels() const;
    int nextLabelIncrement(const int rank) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    QFont font_;                    //!< Font to draw the numeric labels
    QFontMetrics fontMetrics_;      //!< Metrics to compute precise bounds

    double majorTickHeight_;        //!< Height of major tick marks
    double minorTickHeight_;        //!< Height of minor tick marks
};

#endif // LINEARRULERITEM_H
