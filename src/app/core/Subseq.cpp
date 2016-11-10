/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Subseq.h"
#include "constants.h"
#include "global.h"
#include "macros.h"
#include "misc.h"

static const int gapBufferSize = 1;
static ::QByteArray initializeGapBuffer()
{
    char buffer[2];
    buffer[1] = '\0';
    buffer[0] = constants::kDefaultGapCharacter;
    return ::QByteArray(buffer).repeated(gapBufferSize);
}
const ::QByteArray Subseq::gapBuffer_(initializeGapBuffer());

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parentSeq [const Seq &]
  */
Subseq::Subseq(const Seq &parentSeq) : UngappedSubseq(parentSeq)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * Specifically for this method to succeed, the Subseqs must have the same parent and subsequence characters.
  *
  * @param other [const Subseq &]
  * @returns bool
  */
bool Subseq::operator==(const Subseq &other) const
{
    if (this == &other)
        return true;

    return UngappedSubseq::operator==(other) && BioString::operator==(static_cast<BioString>(other));
}

/**
  * @param other [const Subseq &]
  * @returns bool
  */
bool Subseq::operator!=(const Subseq &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Convenience method for calling extendLeft(simpleExtension.subseqPosition_, simpleExtension.seqRange)
  *
  * @param simpleExtension [const SimpleExtension &]
  * @see extendLeft(int, ClosedIntRange)
  */
void Subseq::extendLeft(const SimpleExtension &simpleExtension)
{
    extendLeft(simpleExtension.subseqPosition_, simpleExtension.seqRange_);
}

/**
  * This is a slightly optimized and specialized version of setStart and/or the replace operation. This operation
  * requires the following:
  * o bioString must contain at least one non-gap character
  * o The non-gap characters in bioString must be an exact match of those characters preceding the start position
  * o bioString must be able to replace those characters beginning at position without overlapping the first non-gap
  *   character (current start)
  *
  * @param position [int]
  * @param bioString [const BioString &]
  * @see extendRight()
  */
void Subseq::extendLeft(int position, const BioString &bioString)
{
    int ul = bioString.ungappedLength();

    ASSERT_X(position >= 1 && position <= length(), "position out of range");
    ASSERT_X(ul > 0, "bioString does not contain any non-gap characters");
    ASSERT_X(position + bioString.length() - 1 <= headGaps(), "bioString does not fit when placed at position");
    ASSERT_X(parentSeq_.isExactMatch(start_ - ul, bioString.ungapped()), "bioString does not match parent Seq");

    char *x = data() + position - 1;
    const char *src = bioString.constData();
    memcpy(x, src, bioString.length());

    start_ -= ul;
}

/**
  * This is a slightly optimized and specialized version of setStart and/or the replace operation. Specifically, the
  * characters beginning at position are replaced with the parent sequence characters spanning parentSeqRange. This
  * operation requires the following:
  * o parentSeqRange_ must not be empty
  * o parentSeqRange must end immediately before but not overlap the current start value
  * o The replacement characters from the parent Seq must be able to replace those characters beginning at position
  *   without overlapping the first non-gap character (current start)
  *
  * @param position [int]
  * @param parentSeqRange [const ClosedIntRange &]
  */
void Subseq::extendLeft(int position, const ClosedIntRange &parentSeqRange)
{
    ASSERT_X(parentSeqRange.isEmpty() == false, "parentSeqRange is empty");
    ASSERT_X(parentSeqRange.begin_ > 0 && parentSeqRange.begin_ < parentSeq_.length(),
             "parentSeqRange.begin_ out of range");
    ASSERT_X(parentSeqRange.end_ >= parentSeqRange.begin_ &&
             parentSeqRange.end_ < parentSeq_.length(), "parentSeqRange.end out of range");
    ASSERT_X(parentSeqRange.end_ == start_ - 1, "parentSeqRange.end does not equal start - 1");
    ASSERT_X(position <= headGaps() - parentSeqRange.length() + 1,
             "parent Seq bioString does not fit when placed at position");

    char *x = data() + position - 1;
    const char *src = parentSeq_.constData() + parentSeqRange.begin_ - 1;
    memcpy(x, src, parentSeqRange.length());

    start_ = parentSeqRange.begin_;
}

/**
  * Convenience method for calling extendRight(simpleExtension.subseqPosition_, simpleExtension.seqRange)
  *
  * @param simpleExtension [const SimpleExtension &]
  * @see extendRight(int, ClosedIntRange)
  */
void Subseq::extendRight(const SimpleExtension &simpleExtension)
{
    extendRight(simpleExtension.subseqPosition_, simpleExtension.seqRange_);
}

/**
  * This is a slightly optimized and specialized version of setStop and/or the replace operation. This operation
  * requires the following:
  * o bioString must contain at least one non-gap character
  * o The non-gap characters in bioString must be an exact match of those characters following the stop position
  * o bioString must be able to replace those characters beginning at position without overlapping the last non-gap
  *   character (current stop)
  *
  * @param position [int]
  * @param bioString [const BioString &]
  * @see extendLeft()
  */
void Subseq::extendRight(int position, const BioString &bioString)
{
    int ul = bioString.ungappedLength();

    ASSERT_X(position >= 1 && position <= length(), "position out of range");
    ASSERT_X(ul > 0, "bioString does not contain any non-gap characters");
    ASSERT_X(position >= length() - tailGaps() + 1, "position overlaps non-gap character");
    ASSERT_X(position + bioString.length() - 1 <= length(), "bioString does not fit when placed at position");
    ASSERT_X(parentSeq_.isExactMatch(stop_ + 1, bioString.ungapped()), "bioString does not match parent Seq");

    char *x = data() + position - 1;
    const char *src = bioString.constData();
    memcpy(x, src, bioString.length());

    stop_ += ul;
}

/**
  * This is a slightly optimized and specialized version of setStop and/or the replace operation. Specifically, the
  * characters beginning at position are replaced with the parent sequence characters spanning
  * parentSeqRange. This operation requires the following:
  * o parentSeqRange must not be empty
  * o parentSeqRange must begin but not overlap the current stop value
  * o The replacement characters from the parent Seq must be able to replace those characters beginning at position
  *   without overlapping the last non-gap character (current stop)
  *
  * @param position [int]
  * @param parentSeqRange [const ClosedIntRange &]
  */
void Subseq::extendRight(int position, const ClosedIntRange &parentSeqRange)
{
    ASSERT_X(parentSeqRange.isEmpty() == false, "parentSeqRange is empty");
    ASSERT_X(parentSeqRange.begin_ > 1 && parentSeqRange.begin_ <= parentSeq_.length(),
             "parentSeqRange.begin_ out of range");
    ASSERT_X(parentSeqRange.end_ >= parentSeqRange.begin_ &&
             parentSeqRange.end_ <= parentSeq_.length(), "parentSeqRange.end out of range");
    ASSERT_X(parentSeqRange.begin_ == stop_ + 1, "parentSeqRange.end does not equal stop + 1");
    ASSERT_X(position >= length() - tailGaps() + 1, "position overlaps non-gap character");
    ASSERT_X(position + parentSeqRange.length() - 1 <= length(),
             "parent Seq bioString does not fit when placed at position");

    char *x = data() + position - 1;
    const char *src = parentSeq_.constData() + parentSeqRange.begin_ - 1;
    memcpy(x, src, parentSeqRange.length());

    stop_ = parentSeqRange.end_;
}

/**
  * Because a Subseq must always have at least one non-gap character, this method will not return a range that includes
  * all non-gap characters regardless of position. Note the returned ClosedIntRange is relative to the Subseq
  * coordinates.
  *
  * Examples:
  * 1234567890
  * --A-B-CD--
  * leftTrimRange(1) -> empty
  * leftTrimRange(2) -> empty
  * leftTrimRange(3) -> [3, 3]
  * leftTrimRange(4) -> [3, 3]
  * leftTrimRange(5) -> [3, 5]
  * leftTrimRange(6) -> [3, 5]
  * leftTrimRange(7) -> [3, 7]
  * leftTrimRange(8 -> 10) -> [3, 7] :: Note, that even though position 8 is a non-gap character, trimming it would
  *                                     remove the last non-gap character from the subseq, which is not allowed.
  *
  * @param position [int]
  * @returns ClosedIntRange
  * @see rightTrimRange()
  */
ClosedIntRange Subseq::leftTrimRange(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    ClosedIntRange range;
    int lastNonGapPos = 0;             // In case position includes the last non-gap character, we can easily
                                       // move the range back to this position
    const char *x = constData();
    for (int i=1; i<= position; ++i, ++x)
    {
        if (!::isGapCharacter(*x))
        {
            if (range.begin_ == 0)
                range.begin_ = i;
            else
                lastNonGapPos = range.end_;

            range.end_ = i;
        }
    }

    // If we have not found at least one non-gap by position, then return an empty range
    if (range.isEmpty())
        return range;

    // Now check that there is at least one more non-gap beyond position
    bool hasExtraNonGap = false;
    for (int i=position+1; *x; ++i, ++x)
    {
        if (!::isGapCharacter(*x))
        {
            hasExtraNonGap = true;
            break;
        }
    }

    if (!hasExtraNonGap)
    {
        if (lastNonGapPos == 0)
            return ClosedIntRange();

        range.end_ = lastNonGapPos;
    }

    return range;
}

/**
  * @param position [int]
  * @returns int
  */
int Subseq::mapToSeq(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    if (hasGapAt(position))
        return -1;

    return start_ + nonGapsBetween(ClosedIntRange(1, position)) - 1;
}

/**
  * @param range [const ClosedIntRange &]
  * @param bioString [const BioString &]
  */
void Subseq::rearrange(const ClosedIntRange &range, const BioString &bioString)
{
    ASSERT_X(range.begin_ > 0 && range.begin_ <= range.end_ && range.end_ <= length(), "range out of range");
    ASSERT_X(range.length() == bioString.length(), "range and bioString lengths must be equal");
    ASSERT_X(mid(range).ungapped() == bioString.ungapped(), "different ungapped values between subseq range and bioString");

    BioString::replace(range, bioString);
}

/**
  * @param position [int]
  * @param amount [int]
  * @param bioString [const BioString &]
  * @returns bool
  */
bool Subseq::replace(int position, int amount, const BioString &replacement)
{
    ASSERT_X(position >= 1 && position <= length() + 1, "position out of range");
    ASSERT_X(position + amount - 1 <= length(), "position + amount out of range");
    ASSERT_X(amount >= 0, "amount must be >= 0");

    ClosedIntRange subseqRange(position, position + amount - 1);

    // --------------------------------
    // Derive the number of non-gaps left of position
    const char *x = constData();
    int nNonGapsLeft = 0;
    int nNonGapsRight = 0;
    int nNonGapsInside = 0;
    for (int i=1, z=length(); i<=z; ++i, ++x)
    {
        if (!::isGapCharacter(*x))
        {
            if (i < position)
                ++nNonGapsLeft;
            else if (i <= subseqRange.end_)
                ++nNonGapsInside;
            else
                ++nNonGapsRight;
        }
    }

    // --------------------------------
    // Carry out the replacement if possible
    BioString ungappedReplacement = replacement.ungapped();
    int ungappedReplacementLength = ungappedReplacement.length();

    // Case 1: Replacing non-gaps
    if (nNonGapsInside)
    {
        // Case 1.1: there is at least one left and one right non-gap, therefore, the ungapped replacement must
        //           be identical
        if (nNonGapsLeft && nNonGapsRight)
        {
            // Replacement must be exact substring
            if (!parentSeq_.isExactMatch(start_ + nNonGapsLeft, ungappedReplacement))
                return false;
        }
        // Case 1.2: At least one non-gap on left and zero on the right
        else if (nNonGapsLeft && !nNonGapsRight)
        {
            if (ungappedReplacementLength && !parentSeq_.isExactMatch(start_ + nNonGapsLeft, ungappedReplacement))
                return false;

            stop_ = start_ + nNonGapsLeft + ungappedReplacementLength - 1;
        }
        // Case 1.3: Zero non-gaps on the left and at least one non-gap on the right
        else if (!nNonGapsLeft && nNonGapsRight)
        {
            // Compare the sequences
            int seqIndex = stop_ - nNonGapsRight - ungappedReplacementLength + 1;
            if (seqIndex < 1)
                return false;

            if (ungappedReplacementLength && !parentSeq_.isExactMatch(seqIndex, ungappedReplacement))
                return false;

            start_ = stop_ - nNonGapsRight - ungappedReplacementLength + 1;
        }
        // Case 1.4: There are 0 non-gaps on the left and 0 non-gaps on the right; identical to setBioString
        else    // if (!nNonGapsLeft && !nNonGapsRight)
        {
            BioString tmp = mid(1, position - 1) + replacement;
            if (subseqRange.end_ + 1 <= length())
                tmp.append(mid(ClosedIntRange(subseqRange.end_ + 1, length())));
            return setBioString(tmp);
        }
    }
    // Case 2: zero non-gaps being replaced
    else    // if (nNonGapsInside == 0)
    {
        // Case 2.1
        if (nNonGapsLeft && nNonGapsRight)
        {
            if (ungappedReplacementLength)
                return false;
        }
        else if (nNonGapsLeft && !nNonGapsRight)
        {
            int seqIndex = start_ + nNonGapsLeft;
            if (ungappedReplacementLength &&
                    ( seqIndex > parentSeq_.length() ||
                      !parentSeq_.isExactMatch(seqIndex, ungappedReplacement) ) )
            {
                return false;
            }

            stop_ = start_ + nNonGapsLeft + ungappedReplacementLength - 1;
        }
        else if (!nNonGapsLeft && nNonGapsRight)
        {
            // Compare the sequences
            int seqIndex = stop_ - nNonGapsRight - ungappedReplacementLength + 1;
            if (seqIndex < 1)
                return false;

            if (ungappedReplacementLength && !parentSeq_.isExactMatch(seqIndex, ungappedReplacement))
                return false;

            start_ = stop_ - ungappedReplacementLength;
        }
        else    // !nNonGapsLeft && !nNonGapsRight
        {
            ASSERT_X(0, "impossible to have completely empty subseq: nNonGapsLeft == nNonGapsRight == 0");
            return false;
        }
    }

    BioString::replace(position, amount, replacement);
    return true;
}

/**
  * Behaves slightly differently than its sister replace(int, int, BioString) method. Specifically, it is not possible
  * to specify a range outside the subseq or an empty range. Thus, pure insertions are not possible with this method.
  *
  * @param range [const ClosedIntRange &]
  * @param bioString [const BioString &]
  * @returns bool
  * @see replace(int, int, BioString)
  */
bool Subseq::replace(const ClosedIntRange &range, const BioString &bioString)
{
    ASSERT_X(range.begin_ >= 1 && range.begin_ <= length(), "range.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(), "range.end_ out of range");

    return replace(range.begin_, range.length(), bioString);
}

/**
  * Because a Subseq must always have at least one non-gap character, this method will not return a range that includes
  * all non-gap characters regardless of position. Note the returned ClosedIntRange is relative to the Subseq
  * coordinates.
  *
  * Examples:
  * 1234567890
  * --A-B-CD--
  * rightTrimRange(10) -> empty
  * rightTrimRange(9) -> empty
  * rightTrimRange(8) -> [8, 8]
  * rightTrimRange(7) -> [7, 8]
  * rightTrimRange(6) -> [7, 8]
  * rightTrimRange(5) -> [5, 8]
  * rightTrimRange(4) -> [5, 8]
  * rightTrimRange(3 -> 1) -> [5, 8] :: Note, that even though position 3 is a non-gap character, trimming it would
  *                                     remove the last non-gap character from the subseq, which is not allowed.
  *
  * @param position [int]
  * @returns ClosedIntRange
  * @see leftTrimRange()
  */
ClosedIntRange Subseq::rightTrimRange(int position) const
{
    ASSERT_X(position >= 1 && position <= length(), "position out of range");

    ClosedIntRange range;
    int lastNonGapPos = 0;             // In case position includes the last non-gap character, we can easily
                                       // move the range back to this position
    const char *x = constData() + length() - 1;
    for (int i=length(); i>= position; --i, --x)
    {
        if (!::isGapCharacter(*x))
        {
            if (range.end_ == -1)
                range.end_ = i;
            else
                lastNonGapPos = range.begin_;

            range.begin_ = i;
        }
    }

    // If we have not found at least one non-gap by position, then return an empty range
    if (range.isEmpty())
        return range;

    // Now check that there is at least one more non-gap beyond position
    bool hasExtraNonGap = false;
    for (int i=position-1; i>0; --i, --x)
    {
        if (!::isGapCharacter(*x))
        {
            hasExtraNonGap = true;
            break;
        }
    }

    if (!hasExtraNonGap)
    {
        if (lastNonGapPos == 0)
            return ClosedIntRange();

        range.begin_ = lastNonGapPos;
    }

    return range;
}

/**
  * If the ungapped version of bioString is located within the parent Seq/AnonSeq BioString sequence, set this object's
  * underlying BioString to bioString and update its start and stop coordinates. If the ungapped bioString is empty,
  * then nothing is done and false is returned.
  *
  * Grammar of bioString is ignored, although a warning will be emitted in debug mode if they have different grammars.
  *
  * @param bioString [const BioString &]
  * @returns bool
  */
bool Subseq::setBioString(const BioString &bioString)
{
#ifdef QT_DEBUG
#ifndef TESTING
    if (grammar_ != bioString.grammar())
        qWarning("%s - setting biostring with unequal grammars", __FUNCTION__);
#endif
#endif

    return setBioString(bioString.constData());
}

/**
  * @param byteArray [const QByteArray &]
  * @returns bool
  */
bool Subseq::setBioString(const ::QByteArray &byteArray)
{
    return setBioString(byteArray.constData());
}

/**
  * @param str [const char *]
  * @returns bool
  */
bool Subseq::setBioString(const char *str)
{
    BioString gapless(str);
    gapless.removeGaps();
    if (gapless.isEmpty())
        return false;

    int start = parentSeq_.indexOf(gapless);
    if (start != -1)
    {
        BioString::operator=(str);
        start_ = start;
        stop_ = start_ + gapless.length() - 1;

        return true;
    }

    return false;
}

/**
  * Only updates the start position if it references a valid index within the parent Seq BioString object. If start
  * is valid and greater than stop, the stop position is also updated. Both start and stop positions only relate to
  * the actual sequence characters. In other words, gap positions are not considered when updating the start and stop.
  *
  * Because a Subseq may contain gaps, it is important to understand how adjusting the start_ position impacts the
  * underlying sequence. Assuming a valid newStart:
  *
  * o If newStart > start_, then all sequence characters in bioString_ < newStart will be replaced with gap characters
  * o If newStart < start_, then start_ - newStart next ungapped, characters will be immediately prepended to bioString_
  *   replacing any gap characters that may precede start_
  * o If newStart > stop_, then stop_ is set to newStart
  *
  * >>> Examples
  * 123456
  * ABCDEF     (parent Seq)
  *
  * -C---DE--  (Subseq's bioString_, start = 3, stop = 5)
  *
  * setStart(2)  -> BC---DE--
  * setStart(1)  -> ABC---DE--   **Note: the bioString_ was extended by one character to fit A
  * setStart(4)  -> -----DE--    **Note: after operating on original subseq bioString_ (-C---DE--)
  * setStart(4) and then setStart(1) -> --ABCDE--
  * setStart(6)  -> -------F-
  *
  * @param newStart [int]
  * @see setStop()
  */
void Subseq::setStart(int newStart)
{
    ASSERT_X(newStart >= 1 && newStart <= parentSeq_.length(), "newStart out of range");

    // Slight optimization
    if (newStart == start_)
        return;

    int nHeadGaps = headGaps();

    // Case 1
    if (newStart < start_)
    {
        int nNewChars = start_ - newStart;
        BioString::replace(qMax(1, nHeadGaps - nNewChars + 1),
                           qMin(nHeadGaps, nNewChars),
                           parentSeq_.mid(ClosedIntRange(newStart, start_ - 1)));
    }
    // Case 2: newStart > start_ && newStart <= stop_
    else if (newStart <= stop_)
    {
        char *x = BioString::data() + nHeadGaps;

        int nCharsToRemove = newStart - start_;
        while (nCharsToRemove)
        {
            // Check for non-gap character
            if (!::isGapCharacter(*x))
            {
                *x = constants::kDefaultGapCharacter;
                --nCharsToRemove;
            }
            ++x;
        }
    }
    // Case 3: newStart > stop_
    else
    {
        int nTailGaps = tailGaps(); // Capture number of tail gaps *before* removing characters

        // Step A: Replace all non-gap characters with gaps until we reach the current stop_
        char *x = data() + nHeadGaps;

        int nCharsToRemove = stop_ - start_ + 1;
        while (nCharsToRemove)
        {
            // Check for non-gap character
            if (!isGapCharacter(*x))
            {
                *x = constants::kDefaultGapCharacter;
                --nCharsToRemove;
            }
            ++x;
        }

        // Step B: Skip over and add (if necessary) intermediate gaps and add newStart character
        int nIntermediateGaps = newStart - stop_ - 1;
        if (nTailGaps >= nIntermediateGaps + 1)
        {
            x += nIntermediateGaps;
            *x = parentSeq_.at(newStart);
        }
        else
        {
            // Append any remainining gaps along with the appropriate new start character
            ::QByteArray gapString = ::QByteArray("-").repeated(nIntermediateGaps - nTailGaps);
            append(gapString.constData());
            append(parentSeq_.at(newStart));
        }

        stop_ = newStart;
    }

    // Update the start position to the new position
    start_ = newStart;
}

/**
  * Only updates the stop position if newStop references a valid index within the parent Seq BioString object. If newStop
  * is valid and less than start_, start_ is also updated. Both start and stop positions only relate to the actual
  * sequence characters. In other words, gap positions are not considered when updating the start and stop.
  *
  * Because a Subseq may contain gaps, it is important to understand how adjusting the stop position impacts the
  * underlying sequence. Assuming a valid newStop:
  *
  * o If newStop < stop_, then all sequence characters in bioString_ > stop_ will be replaced with the default gap character
  * o If newStop > stop_, then the next stop_ - newStop ungapped characters will be immediately appended to bioString_
  *   replacing any gap characters that may succeed stop_
  * o If newStop < start_, then start_ is set to newStop
  *
  * >>> Examples
  * 123456
  * ABCDEF     (parent Seq)
  *
  * -C---DE--  (Subseq's bioString_, start = 3, stop = 5)
  *
  * setStop(6)  -> -C---DEF-
  * setStop(1)  -> A---------   **Note: the bioString_ was extended by one character to fit A
  * setStop(3)  -> -C-------
  * setStop(3) and then setStop(6) -> -CDEF----
  *
  * @param newStop [int]
  * @see setStart()
  */
void Subseq::setStop(int newStop)
{
    ASSERT_X(newStop >= 1 && newStop <= parentSeq_.length(), "newStop out of range");

    // Slight optimization
    if (newStop == stop_)
        return;

    int nTailGaps = tailGaps();

    // Case 1
    if (newStop > stop_)
    {
        int nNewChars = newStop - stop_;
        BioString::replace(length() - nTailGaps + 1,
                           qMin(nNewChars, nTailGaps),
                           parentSeq_.mid(stop_ + 1, nNewChars));
    }
    // Case 2: newStop < stop_ && newStop >= start_
    else if (newStop >= start_)
    {
        // Simply need to replace non-gap characters with gaps until we reach the newStop
        char *x = data() + length() - 1 - nTailGaps;

        int nCharsToRemove = stop_ - newStop;
        while (nCharsToRemove)
        {
            // Check for non-gap character
            if (!::isGapCharacter(*x))
            {
                *x = constants::kDefaultGapCharacter;
                --nCharsToRemove;
            }
            --x;
        }
    }
    // Case 3: newStop < start_
    else
    {
        int nHeadGaps = headGaps();     // Note this amount is captured *before* we remove characters

        // Step A: Replace all non-gap characters with gaps until we reach the current start_
        char *x = data() + length() - 1 - nTailGaps;

        int nCharsToRemove = stop_ - start_ + 1;
        while (nCharsToRemove)
        {
            // Check for non-gap character
            if (!::isGapCharacter(*x))
            {
                *x = constants::kDefaultGapCharacter;
                --nCharsToRemove;
            }
            --x;
        }

        // Step B: Skip over and add (if necessary) intermediate gaps and add newStop character
        int nIntermediateGaps = start_ - newStop - 1;
        if (nHeadGaps >= nIntermediateGaps + 1)
        {
            x -= nIntermediateGaps;
            *x = parentSeq_.at(newStop);
        }
        else
        {
            // Append any remainining gaps along with the appropriate new start character
            ::QByteArray gapString = ::QByteArray("-").repeated(nIntermediateGaps - nHeadGaps);
            prepend(gapString.constData());
            prepend(parentSeq_.at(newStop));
        }

        start_ = newStop;
    }

    // Update the stop position to the new position
    stop_ = newStop;
}

/**
  * Convenience method that calls trimLeft(trim.subseqRange_, trim.nNonGaps_).
  *
  * @param trim [const Trim &]
  * @see trimRight()
  */
void Subseq::trimLeft(const Trim &trim)
{
    trimLeft(trim.subseqRange_, trim.nNonGaps_);
}

/**
  * This is a slightly optimized and specialized version of setStart and/or the replace operation. Specifically, the
  * characters between trim.subseqRange are replaced with a gap string (using default gap character). If trim.nNonGaps
  * is positive, it is assumed that there are this many non-gap characters in this subseq range. Otherwise, they are
  * determined before the trim operation. This value is necessary to properly update the start member. This operation
  * requires the following:
  * o trim.subseqRange must not be empty
  * o The characters in trim.subseqRange must contain at least one non-gap character
  * o There must be no non-gap characters before trim.subseqRange.begin
  * o There must be at least one non-gap character after trim.subseqRange.end
  *
  * @param range [const ClosedIntRange &]
  * @param nNonGaps [int]
  */
void Subseq::trimLeft(const ClosedIntRange &range, int nNonGaps)
{
    ASSERT_X(range.begin_ > 0 && range.begin_ < length(),
             "trim.subseqRange.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ < length(),
             "trim.subseqRange.end_ out of range");
    ASSERT_X(nNonGaps == 0 || nNonGaps == nonGapsBetween(range),
             "trim non-gap count does not equal number of non-gap characters in subseqRange");
    ASSERT_X(range.begin_ == 1 || nonGapsBetween(ClosedIntRange(1, range.begin_ - 1)) == 0,
            "Non-gap present left of range.begin_");
    ASSERT_X(nonGapsBetween(ClosedIntRange(range.end_ + 1, length())) > 0,
             "No non-gap characters located after range.end_");

    // Must update the start value before the actual trim operation in case it is necessary to count the number
    // of non-gap characters
    if (nNonGaps > 0)
        start_ += nNonGaps;
    else
        start_ += nonGapsBetween(range);

    // Now overwrite the trim region with gaps
    int gapsWritten = 0;
    char *x = data() + range.begin_ - 1;
    const char *src = gapBuffer_.constData();
    for (int i=0, z= range.length() - gapBufferSize; i< z; i += gapBufferSize)
    {
        memcpy(x, src, gapBufferSize);
        gapsWritten += gapBufferSize;
        x += gapBufferSize;
    }

    // Finally any remainder
    memcpy(x, src, range.length() - gapsWritten);
}

/**
  * Convenience method that calls trimRight(trim.subseqRange_, trim.nNonGaps_).
  *
  * @param trim [const Trim &]
  * @see trimleft()
  */
void Subseq::trimRight(const Trim &trim)
{
    trimRight(trim.subseqRange_, trim.nNonGaps_);
}

/**
  * This is a slightly optimized and specialized version of setStop and/or the replace operation. Specifically, the
  * characters between trim.subseqRange are replaced with a gap string (using default gap character). If trim.nNonGaps
  * is positive, it is assumed that there are this many non-gap characters in this subseq range. Otherwise, they are
  * determined before the trim operation. This value is necessary to properly update the stop member. This operation
  * requires the following:
  * o trim.subseqRange must not be empty
  * o The characters in trim.subseqRange must contain at least one non-gap character
  * o There must be no non-gap characters after trim.subseqRange.end
  * o There must be at least one non-gap character after trim.subseqRange.begin
  *
  * @param trim [const Trim &]
  * @see trimleft()
  */
void Subseq::trimRight(const ClosedIntRange &range, int nNonGaps)
{
    ASSERT_X(range.begin_ > 1 && range.begin_ <= length(),
             "trim.subseqRange.begin_ out of range");
    ASSERT_X(range.end_ >= range.begin_ && range.end_ <= length(),
             "trim.subseqRange.end_ out of range");
    ASSERT_X(nNonGaps == 0 || nNonGaps == nonGapsBetween(range),
             "trim non-gap count does not equal number of non-gap characters in subseqRange");
    ASSERT_X(range.end_ == length() ||
             nonGapsBetween(ClosedIntRange(range.end_ + 1, length())) == 0,
            "Non-gap present right of range.end_");
    ASSERT_X(nonGapsBetween(ClosedIntRange(1, range.begin_ - 1)) > 0,
             "No non-gap characters located before range.begin_");

    // Must update the start value before the actual trim operation in case it is necessary to count the number
    // of non-gap characters
    if (nNonGaps > 0)
        stop_ -= nNonGaps;
    else
        stop_ -= nonGapsBetween(range);

    // Now overwrite the trim region with gaps
    int gapsWritten = 0;
    char *x = data() + range.begin_ - 1;
    const char *src = gapBuffer_.constData();
    for (int i=0, z= range.length() - gapBufferSize; i< z; i += gapBufferSize)
    {
        memcpy(x, src, gapBufferSize);
        gapsWritten += gapBufferSize;
        x += gapBufferSize;
    }

    // Finally any remainder
    memcpy(x, src, range.length() - gapsWritten);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param subseqs [const QVector<Subseq *>]
  * @returns QVector<AbstractSeq *>
  */
/*
QVector<AbstractSeq *> Subseq::seqEntityVector(const QVector<Subseq *> subseqs)
{
    QVector<AbstractSeq *> abstractSeqVector(subseqs.size(), nullptr);
    for (int i=0, z=subseqs.size(); i<z; ++i)
        abstractSeqVector[i] = subseqs.at(i)->seqEntity_;

    return abstractSeqVector;
}
*/
