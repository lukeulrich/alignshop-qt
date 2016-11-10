/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTRANGEITEM_H
#define ABSTRACTRANGEITEM_H

#include "AbstractLinearItem.h"
#include "../../../core/util/ClosedIntRange.h"

/**
  * AbstractRangeItem extends AbstractLinearItem with methods for managing a range within the discrete integral
  * two-dimensional space.
  */
class AbstractRangeItem : public AbstractLinearItem
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    ClosedIntRange range() const;                   //!< Returns the current range
    QPointF rangeCenter() const;                    //!< Returns the center point of this item relative to its current range
    bool setStart(int newStart);                    //!< Attempts to set the start to newStart and returns true on success; false otherwise
    bool setStop(int newStop);                      //!< Attempts to set the stop to newStop and returns true on success; false otherwise
    int start() const;                              //!< Returns the start position
    int stop() const;                               //!< Returns the stop position


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void startChanged(int start);                   //!< Emitted whenever the start value changes
    void stopChanged(int stop);                     //!< Emitted whenever the stop value changes


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected constructors
    //! Construct an instance spanning start and stop with total length and a child of parentItem
    AbstractRangeItem(int start, int stop, int totalLength, QGraphicsItem *parentItem);
    //! Construct an instance spanning range with total length and as a child of parentItem
    AbstractRangeItem(const ClosedIntRange &range, int totalLength, QGraphicsItem *parentItem);


    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool setStartImpl(int newStart);        //!< Virtual method for actually updating the start value to newStart; returns true on success and false otherwise
    virtual bool setStopImpl(int newStop);          //!< Virtual method for actually updating the sto[ value to newStop; returns true on success and false otherwise


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    ClosedIntRange range_;
};

#endif // ABSTRACTRANGEITEM_H
