/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "UngappedSubseq.h"

#include "macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parentSeq [const Seq &]
  */
UngappedSubseq::UngappedSubseq(const Seq &parentSeq)
    : BioString(parentSeq.constData(), parentSeq.grammar()),
      parentSeq_(parentSeq),
      start_(0),
      stop_(0),
      relativeStart_(0)
{
    if (!parentSeq_.isEmpty())
    {
        start_ = 1;
        stop_ = parentSeq_.length();
        relativeStart_ = 1;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const UngappedSubseq &]
  * @returns bool
  */
bool UngappedSubseq::operator==(const UngappedSubseq &other) const
{
    if (this == &other)
        return true;

    return parentSeq_== other.parentSeq_ &&
           start_ == other.start_ &&
           stop_ == other.stop_ &&
           relativeStart_ == other.relativeStart_;
}

/**
  * @param other [const UngappedSubseq &]
  * @returns bool
  */
bool UngappedSubseq::operator!=(const UngappedSubseq &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * For example:
  *
  *             12345678
  * Parent Seq: ABCDEFGH
  *
  *         34  56
  * Subseq: CD--EF
  *
  * Would return: -6
  *
  * @returns int
  */
int UngappedSubseq::inverseStart() const
{
    return -parentSeq_.length() + start_ - 1;
}

/**
  * For example:
  *
  *             12345678
  * Parent Seq: ABCDEFGH
  *
  *         34  56
  * Subseq: CD--EF
  *
  * Would return: -3
  *
  * @returns int
  */
int UngappedSubseq::inverseStop() const
{
    return -parentSeq_.length() + stop_ - 1;
}

/**
  * start_ will be moved as many dstart characters as is possible. Obviously, if dstart equals zero, nothing will be
  * changed.
  *
  * NOTE: Non-virtual because implementation calls the virtual function setStart which is responsible for updating the
  * core sequence.
  *
  * @param dstart [int]
  * @returns int
  * @see moveStop()
  */
int UngappedSubseq::moveStart(int dstart)
{
    int oldStart = start_;
    int newStart = start_ + dstart;

    // Move as many characters as possible and clamp to sequence boundaries
    if (newStart < 1)
        newStart = 1;
    else
    {
        int x = parentSeq_.length();
        if (newStart > x)
            newStart = x;
    }

    setStart(newStart);

    return qAbs(start_ - oldStart);
}

/**
  * stop_ will be moved as many dstop characters as is possible. Obviously, if dstop equals zero, nothing will be
  * changed.
  *
  * NOTE: Non-virtual because implementation calls the virtual function setStop which is responsible for updating the
  * core sequence.
  *
  * @param dstop [int]
  * @returns int
  */
int UngappedSubseq::moveStop(int dstop)
{
    int old_stop = stop_;
    int new_stop = stop_ + dstop;

    // Move as many characters as possible and clamp to sequence boundaries
    if (new_stop < 1)
        new_stop = 1;
    else
    {
        int x = parentSeq_.length();
        if (new_stop > x)
            new_stop = x;
    }

    setStop(new_stop);

    return qAbs(stop_ - old_stop);
}

/**
  * @returns int
  */
int UngappedSubseq::relativeStart() const
{
    return relativeStart_;
}

/**
  * @returns int
  */
int UngappedSubseq::relativeStop() const
{
    return relativeStart_ + stop_ - 1;
}

/**
  * @param relativeStart [int]
  */
void UngappedSubseq::setRelativeStart(int relativeStart)
{
    ASSERT(relativeStart > 0);

    relativeStart_ = relativeStart;
}

/**
  * Only updates the start position if it references a valid index within the parent Seq BioString object. If start
  * is valid and greater than stop, the stop position is also updated.
  *
  *
  * >>> Examples
  * 123456
  * ABCDEF     (parent Seq)
  *
  * CDE        (start = 3, stop = 5)
  *
  * setStart(2)  -> BCDE
  * setStart(1)  -> ABCDE
  * setStart(4)  -> DE
  * setStart(4) and then setStart(1) -> ABCDE
  * setStart(6)  -> F
  *
  * @param newStart [int]
  * @see setStop()
  */
void UngappedSubseq::setStart(int newStart)
{
    ASSERT_X(parentSeq_.isValidPosition(newStart), "newStart out of range");

    // Slight optimization
    if (newStart == start_)
        return;

    // Case 1
    if (newStart < start_)
    {
        BioString::prepend(parentSeq_.mid(ClosedIntRange(newStart, start_ - 1)));
    }
    // Case 2: newStart > start_ && newStart <= stop_
    else if (newStart <= stop_)
    {
        BioString::remove(1, newStart - start_);
    }
    // Case 3: newStart > stop_
    else
    {
        BioString::operator=(parentSeq_.at(newStart));

        stop_ = newStart;
    }

    // Update the start position to the new position
    start_ = newStart;
}

/**
  * Only updates the stop position if newStop references a valid index within the parent Seq BioString object. If
  * newStop is valid and less than start_, start_ is also updated.
  *
  * >>> Examples
  * 123456
  * ABCDEF     (parent Seq)
  *
  * CDE        (start = 3, stop = 5)
  *
  * setStop(6)  -> CDEF
  * setStop(1)  -> A
  * setStop(3)  -> C
  * setStop(3) and then setStop(6) -> CDEF
  *
  * @param newStop [int]
  * @see setStart()
  */
void UngappedSubseq::setStop(int newStop)
{
    ASSERT_X(parentSeq_.isValidPosition(newStop), "newStop out of range");

    // Slight optimization
    if (newStop == stop_)
        return;

    // Case 1
    if (newStop > stop_)
    {
        BioString::append(parentSeq_.mid(ClosedIntRange(stop_ + 1, newStop)));
    }
    // Case 2: newStop < stop_ && newStop >= start_
    else if (newStop >= start_)
    {
        BioString::chop(stop_ - newStop);
    }
    // Case 3: newStop < start_
    else
    {
        BioString::operator=(parentSeq_.at(newStop));
        start_ = newStop;
    }

    stop_ = newStop;
}

/**
  * @returns int
  */
int UngappedSubseq::start() const
{
    return start_;
}

/**
  * @returns int
  */
int UngappedSubseq::stop() const
{
    return stop_;
}
