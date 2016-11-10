/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef UNGAPPEDSUBSEQ_H
#define UNGAPPEDSUBSEQ_H

#include <QtCore/QByteArray>

#include "BioString.h"
#include "Seq.h"

/**
  * UngappedSubseq simply references a closed int range of a parent Seq object.
  *
  * It is a memory efficient subsequence representation because it only requires a parent Seq object and a range
  * indicating where this subseq is located.
  */
class UngappedSubseq : protected BioString
{
public:
    UngappedSubseq(const Seq &parentSeq);                               //!< Construct a Subseq from parentSeq

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const UngappedSubseq &other) const;                 //!< Returns true if other is equivalent to this Subseq; false otherwise
    bool operator!=(const UngappedSubseq &other) const;                 //!< Returns true if other is not identical to this Subseq; false otherwise


    // ------------------------------------------------------------------------------------------------
    // Re-exposed BioString operators
    bool operator<(const UngappedSubseq &other) const                        {   return BioString::operator<(other);       }
    using BioString::operator<;
    using BioString::operator[];
    using BioString::operator<=;
    using BioString::operator==;
    using BioString::operator>;
    using BioString::operator>=;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int inverseStart() const;                                           //!< Returns a negative value indicating the number of characters start is from the rightmost parentSeq position
    int inverseStop() const;                                            //!< Returns a negative value indicating the number of characters stop is from the rightmost parentSeq position
    int moveStart(int dstart);                                          //!< Moves start_ dstart characters and returns the number of characters the start position was successfully moved
    int moveStop(int dstop);                                            //!< Moves stop_ dstop characters and returns the number of characters the stop position was successfully moved
    int relativeStart() const;                                          //!< Returns the 1-based start position relative to relativeStart
    int relativeStop() const;                                           //!< Returns the 1-based stop position relative to relativeStart
    void setRelativeStart(int relativeStart);                           //!< Sets the relative start value to relativeStart
    virtual void setStart(int newStart);                                //!< Sets the start position to newStart
    virtual void setStop(int newStop);                                  //!< Sets the stop position to stop
    int start() const;                                                  //!< Returns the start position (1-based)
    int stop() const;                                                   //!< Returns the stop position (1-based)


    // ------------------------------------------------------------------------------------------------
    // Re-exposed public methods
    using BioString::asByteArray;
    using BioString::at;
    using BioString::backTranscribe;
    using BioString::capacity;
    using BioString::constData;
    using BioString::contains;
    using BioString::count;
    using BioString::endsWith;
    using BioString::gapsBetween;
    using BioString::gapsLeftOf;
    using BioString::gapsRightOf;
    using BioString::grammar;
    using BioString::indexOf;
    using BioString::isEmpty;
    using BioString::isEquivalentTo;
    using BioString::isExactMatch;
    using BioString::isNull;
    using BioString::lastIndexOf;
    using BioString::left;
    using BioString::length;
    using BioString::mid;
    using BioString::reserve;
    using BioString::reverseComplement;
    using BioString::right;
    using BioString::split;
    using BioString::squeeze;
    using BioString::startsWith;
    using BioString::transcribe;


    // ------------------------------------------------------------------------------------------------
    // Public members
    const Seq parentSeq_;

protected:
    using BioString::collapseLeft;
    using BioString::collapseRight;
    using BioString::data;
    using BioString::hasGaps;
    using BioString::hasNonGaps;
    using BioString::headGaps;
    using BioString::insertGaps;
    using BioString::leftSlidablePositions;
    using BioString::removeGaps;
    using BioString::rightSlidablePositions;
    using BioString::slide;
    using BioString::tailGaps;
    using BioString::translateGaps;
    using BioString::ungapped;
    using BioString::ungappedLength;

    // ------------------------------------------------------------------------------------------------
    // Protected members
    int start_;
    int stop_;

    int relativeStart_;
};

#endif // UNGAPPEDSUBSEQ_H
