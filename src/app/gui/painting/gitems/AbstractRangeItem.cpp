/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractRangeItem.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns ClosedIntRange
  */
ClosedIntRange AbstractRangeItem::range() const
{
    return range_;
}

/**
  * @returns QPointF
  */
QPointF AbstractRangeItem::rangeCenter() const
{
    QPointF point = this->boundingRect().center();
    point.setX(leftPositionFromUnit(range_.begin_) + (rightPositionFromUnit(range_.end_) - leftPositionFromUnit(range_.begin_)) / 2.);
    return point;
}

/**
  * @param newStart [int]
  * @returns bool
  */
bool AbstractRangeItem::setStart(int newStart)
{
    if (newStart == range_.begin_)
        return true;

    int oldStart = range_.begin_;
    range_.begin_ = newStart;
    if (!setStartImpl(newStart))
    {
        range_.begin_ = oldStart;
        return false;
    }

    emit startChanged(range_.begin_);

    return true;
}

/**
  * @param newStop [int]
  * @returns bool
  */
bool AbstractRangeItem::setStop(int newStop)
{
    if (newStop == range_.end_)
        return true;

    int oldStop = range_.end_;
    range_.end_ = newStop;
    if (!setStopImpl(newStop))
    {
        range_.end_ = oldStop;
        return false;
    }

    emit stopChanged(range_.end_);

    return true;
}

/**
  * @returns int
  */
int AbstractRangeItem::start() const
{
    return range_.begin_;
}

/**
  * @returns int
  */
int AbstractRangeItem::stop() const
{
    return range_.end_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected constructors
/**
  * The totalLength argument denotes the entire length of the item being modeled - it does not indicate the difference
  * between the start and stop. The start and stop arguments denote the current range within length (1 based).
  *
  * @param start [int]
  * @param stop [int]
  * @param totalLength [int]
  * @param parentItem [QGraphicsItem *]
  */
AbstractRangeItem::AbstractRangeItem(int start, int stop, int totalLength, QGraphicsItem *parentItem)
    : AbstractLinearItem(totalLength, parentItem),
      range_(start, stop)
{
}

/**
  * The totalLength argument denotes the entire length of the item being modeled - it does not indicate the range
  * length.
  *
  * @param range [const ClosedIntRange &]
  * @param totalLength [int]
  * @param parentItem [QGraphicsItem *]
  */
AbstractRangeItem::AbstractRangeItem(const ClosedIntRange &range, int totalLength, QGraphicsItem *parentItem)
    : AbstractLinearItem(totalLength, parentItem),
      range_(range)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Default implementation checks that newStart is between [1 .. current stop] inclusive.
  *
  * @param newStart [int]
  * @returns bool
  */
bool AbstractRangeItem::setStartImpl(int newStart)
{
    return newStart >= 1 && newStart <= range_.end_;
}

/**
  * Default implementation checks that newStop is between [current start .. length] inclusive.
  *
  * @param newStop [int]
  * @returns bool
  */
bool AbstractRangeItem::setStopImpl(int newStop)
{
    return newStop >= range_.begin_ && newStop <= length();
}
