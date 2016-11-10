/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "Subseq.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Constructs a Subseq that spans the entire length of anonSeq.
  *
  * @param anonSeq [const AnonSeq &]
  * @param id [int]
  */
Subseq::Subseq(const AnonSeq &anonSeq, int id) : anonSeq_(anonSeq), id_(id)
{
    bioString_.reset(anonSeq.bioString().clone());
    start_ = 1;
    stop_ = bioString_->length();
    modified_ = false;
}

/**
  * Copy all member data including arbitrary user properties.
  *
  * @param other [const Subseq &]
  */
Subseq::Subseq(const Subseq &other)
    : modified_(other.modified_), anonSeq_(other.anonSeq_), id_(other.id_), label_(other.label_), start_(other.start_), stop_(other.stop_)
{
    // Clone copy of other bioString
    bioString_.reset(other.bioString_->clone());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * Assigns other to the current object
  * @param other reference to Subseq to be assigned
  */
Subseq &Subseq::operator=(const Subseq &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    bioString_.reset(other.bioString_->clone());

    anonSeq_ = other.anonSeq_;
    id_ = other.id_;
    label_ = other.label_;
    start_ = other.start_;
    stop_ = other.stop_;
    modified_ = other.modified_;

    return *this;
}

/**
  * Position i must reference a valid index (1-based). Simply passes the call onto BioString. Effectively
  * the same as at().
  *
  * @param i [int]
  * @return const QChar
  * @see at()
  */
const QChar Subseq::operator[](int i) const
{
    Q_ASSERT_X(i != 0, "Subseq::operator[]", "index must not equal 0");

    return (*bioString_)[i];
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns const AnonSeq &
  */
const AnonSeq &Subseq::anonSeq() const
{
    return anonSeq_;
}

/**
  * Position i must reference a valid index (1-based). Simply passes the call onto BioString. Effectively
  * the same as operator[]().
  *
  * @param i [int]
  * @return const QChar
  * @see operator[]()
  */
const QChar Subseq::at(int i) const
{
    Q_ASSERT_X(i != 0, "Subseq::at", "index must not equal 0");

    return (*bioString_)[i];
}

/**
  * Maximally shifts all characters in a given segment to the left.
  *
  * @param start [int]
  * @param stop [int]
  * @returns QPair<int, int>
  * @see collapseRight()
  */
QPair<int, int> Subseq::collapseLeft(int start, int stop)
{
    ASSERT_X(start >= 1 && start <= bioString_->length(), "start out of range");
    ASSERT_X(stop >= start && stop <= bioString_->length(), "stop out of range");

    QString tmpseq = bioString_->sequence();
    QChar *charPtr = tmpseq.data();

    // Find first gap
    int firstGap = -1;
    const QChar *x = charPtr + start - 1;
    for (int i=start; i <= stop; ++i)
    {
        if (isGapCharacter(*x))
        {
            firstGap = i;
            break;
        }
        ++x;
    }

    // Return if the first gap is not found
    QPair<int, int> affectedRange;
    if (firstGap == -1)
        return affectedRange;

    QChar swapVar;
    QChar *firstGapPtr = charPtr + firstGap - 1;
    QChar *y = firstGapPtr + 1; // There is no point in checking that the firstGap character is not a gap character since we already
                                // know this. Furthermore, that is why i is incremented by 1 in the initialization of the following
                                // for loop
    for (int i=firstGap + 1; i <= stop; ++i, ++y)
    {
        if (!isGapCharacter(*y))
        {
            swapVar = *firstGapPtr;
            *firstGapPtr = *y;
            *y = swapVar;

            ++firstGapPtr;

            if (affectedRange.first == 0)
                affectedRange.first = firstGap;
            affectedRange.second = i;
        }
    }

    *bioString_ = tmpseq;

    return affectedRange;
}

/**
  * Maximally shifts all characters in a given segment to the right.
  *
  * @param start [int]
  * @param stop [int]
  * @returns QPair<int, int>
  * @see collapseLeft()
  */
QPair<int, int> Subseq::collapseRight(int start, int stop)
{
    ASSERT_X(start >= 1 && start <= bioString_->length(), "start out of range");
    ASSERT_X(stop >= start && stop <= bioString_->length(), "stop out of range");

    QString tmpseq = bioString_->sequence();
    QChar *charPtr = tmpseq.data();

    // Find first gap from the right
    int firstGap = -1;
    const QChar *x = charPtr + stop - 1;
    for (int i=stop; i >= start; --i)
    {
        if (isGapCharacter(*x))
        {
            firstGap = i;
            break;
        }
        --x;
    }

    // Return if the first gap is not found
    QPair<int, int> affectedRange;
    if (firstGap == -1)
        return affectedRange;

    QChar swapVar;
    QChar *firstGapPtr = charPtr + firstGap - 1;
    QChar *y = firstGapPtr - 1; // There is no point in checking that the firstGap character is not a gap character since we already
                                // know this. Furthermore, that is why i is decremented by 1 in the initialization of the following
                                // for loop
    for (int i=firstGap - 1; i >= start; --i, --y)
    {
        if (!isGapCharacter(*y))
        {
            swapVar = *firstGapPtr;
            *firstGapPtr = *y;
            *y = swapVar;

            --firstGapPtr;

            if (affectedRange.second == 0)
                affectedRange.second = firstGap;
            affectedRange.first = i;
        }
    }

    *bioString_ = tmpseq;

    return affectedRange;
}

/**
  * @return int
  * @see tailGaps()
  */
int Subseq::headGaps() const
{
    int headGaps = 0;
    const QChar *x = bioString_->sequence().constData();
    while (BioString::isGap(*x) &&
           *x != '\0')
    {
        ++headGaps;
        ++x;
    }

    return headGaps;
}

/**
  * @returns int
  */
int Subseq::id() const
{
    return id_;
}

/**
  * Simply pass this call along to the BioString function
  *
  * @param position [int]
  * @param n [int]
  * @param gapCharacter [char]
  * @return Subseq &
  */
Subseq &Subseq::insertGaps(int position, int n, char gapCharacter)
{
    bioString_->insertGaps(position, n, gapCharacter);

    return *this;
}


/**
  * Start_ will be moved as many dstart characters as is possible. The final sequence will be similar to that of setStart() and
  * setStop() in terms of how characters are added and removed. Obviously, if dstart equals zero, nothing will be changed.
  *
  * >>> Examples:
  *
  * 123456
  * ABCDEF
  *
  * -C---D-  (start = 3, stop = 4)
  * moveStart(-1) -> BC---D- (start = 2, stop = 4)
  * moveStart(-2..-N) -> ABC---D-
  * moveStart(1) -> -----D-
  * moveStart(2) -> ------E  (since start now exceeds stop, also update stop)
  * moveStart(3..N) -> -------F
  * moveStart(0) -> -C---D-
  *
  * @param dstart [int]
  * @return int; number of characters start position was successfully moved
  * @see moveStop()
  */
int Subseq::moveStart(int dstart)
{
    int old_start = start_;
    int new_start = start_ + dstart;

    // Move as many characters as possible and clamp to sequence boundaries
    if (new_start < 1)
        new_start = 1;
    else
    {
        int x = anonSeq_.bioString().length();
        if (new_start > x)
            new_start = x;
    }

    setStart(new_start);

    return qAbs(start_ - old_start);
}

/**
  * Stop_ will be moved as many dstop characters as is possible. The final sequence will be similar to that of setStart() and
  * setStop() in terms of how characters are added and removed. Obviously, if dstop equals zero, nothing will be changed.
  *
  * >>> Examples:
  *
  * 123456
  * ABCDEF
  *
  * -C---D-  (start = 3, stop = 4)
  * moveStop(1) -> -C---DE (start = 3, stop = 5)
  * moveStop(2..N) -> -C---DEF
  * moveStop(-1) -> -C-----
  * moveStop(-2) -> B------  (since stop now exceeds start, also update start)
  * moveStop(-3..-N) -> A-------
  * moveStop(0) -> -C---D-
  *
  * @param dstop [int]
  * @return int; number of characters stop position was successfully moved
  * @see moveStart()
  */
int Subseq::moveStop(int dstop)
{
    int old_stop = stop_;
    int new_stop = stop_ + dstop;

    // Move as many characters as possible and clamp to sequence boundaries
    if (new_stop < 1)
        new_stop = 1;
    else
    {
        int x = anonSeq_.bioString().length();
        if (new_stop > x)
            new_stop = x;
    }

    setStop(new_stop);

    return qAbs(stop_ - old_stop);
}

/**
  * Simply remove gaps on the BioString object itself.
  *
  * @param position [int]
  * @param n [int]
  * @return Subseq &
  */
Subseq &Subseq::removeGaps(int position, int n)
{
    bioString_->removeGaps(position, n);

    return *this;
}

/**
  * If the ungapped version of bioString is located within the parent Seq/AnonSeq BioString sequence, update the local
  * bioString_ member and update start_, stop_, headGaps_, and tailGaps_.
  *
  * By update, we simply update its gapped sequence data - the actual alphabet/BioString type (determined by the underlying
  * AnonSeq) is not affected.
  *
  * @param bioString [const BioString &]
  * @return bool; if bioString was successfully set
  */
bool Subseq::setBioString(const BioString &bioString)
{
    QString ungapped = bioString.ungapped();
    int start = anonSeq_.bioString().indexOf(ungapped);
    if (start)
    {
        *bioString_ = bioString;
        start_ = start;
        stop_ = start + ungapped.length() - 1;

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
  * underlying bioString_ sequence. Assuming a valid newStart:
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
  * setStart(-3) -> -----DE--
  *
  * @param start [int] RelAbs coordinate
  * @return bool; whether start position was accepted
  * @see setStop()
  */
bool Subseq::setStart(int newStart)
{
    const BioString anonBioString = anonSeq_.bioString();

    // Must compute the absolute coordinate with respect to the parent Seq/AnonSeq
    newStart = anonBioString.positiveIndex(newStart);

    // Slight optimization
    if (newStart == start_)
        return true;

    if (newStart >= 1 &&
        newStart <= anonBioString.length())
    {
        int head_gaps = headGaps();

        // Case 1
        if (newStart < start_)
        {
            // Case 1.1
            int n_new_chars = start_ - newStart;
            if (n_new_chars <= head_gaps)
            {
                // Replace gapped region with new characters
                bioString_->replace(head_gaps - n_new_chars + 1, n_new_chars, anonBioString.segment(newStart, start_ - 1));
            }
            // Case 1.2
            else // n_new_chars > head_gaps
            {
                // Step A: replace any leading gaps with the equivalent number of characters from the anonBioString
                bioString_->replace(1, head_gaps, anonBioString.mid(start_ - head_gaps, head_gaps));

                // Step B: Prepend the remainder
                bioString_->prepend(anonBioString.mid(newStart, start_ - newStart - head_gaps));
            }
        }
        // Case 2: newStart > start_ && newStart <= stop_
        else if (newStart <= stop_)
        {
            // Simply need to replace non-gap characters with gaps until we reach the newStart
            QString tmpseq = bioString_->sequence();
            QChar *x = tmpseq.data();
            x += head_gaps;

            int n_chars_to_remove = newStart - start_;
            while (n_chars_to_remove)
            {
                // Check for non-gap character
                if (!BioString::isGap(*x))
                {
                    *x = constants::kDefaultGapCharacter;     // Defined in BioString.h
                    --n_chars_to_remove;
                }
                ++x;
            }

            *bioString_ = tmpseq;
        }
        // Case 3: newStart > stop_
        else
        {
            // Step A: Replace all non-gap characters with gaps until we reach the current stop_
            QString tmpseq = bioString_->sequence();
            QChar *x = tmpseq.data();
            x += head_gaps;

            int n_chars_to_remove = stop_ - start_ + 1;
            while (n_chars_to_remove)
            {
                Q_ASSERT(*x != '\0');

                // Check for non-gap character
                if (!BioString::isGap(*x))
                {
                    *x = constants::kDefaultGapCharacter;     // Defined in BioString.h
                    --n_chars_to_remove;
                }
                ++x;
            }

            // Step B: Skip over and add (if necessary) intermediate gaps and add newStart character
            int tail_gaps = tailGaps();  // Note that this does not reflect the changes we just in the previous while loop
                                         // Because those changes were performed on a temporary QString which has not been reassigned
                                         // to this classes' bioString_
            int intermediate_gaps = newStart - stop_ - 1;
            if (tail_gaps >= intermediate_gaps + 1)
            {
                x += intermediate_gaps;
                *x = anonBioString.sequence().at(newStart - 1);
                *bioString_ = tmpseq;
            }
            else
            {
                // Append any remainining gaps along with the appropriate new start character
                *bioString_ = tmpseq;
                bioString_->append(QString("-").repeated(intermediate_gaps - tail_gaps)).append(anonBioString.mid(newStart));
            }

            stop_ = newStart;
        }

        // Update the start position to the new position
        start_ = newStart;

        // Position was accepted
        return true;
    }

    // Position was invalid
    return false;
}

/**
  * Only updates the stop position if newStop references a valid index within the parent Seq BioString object. If newStop
  * is valid and less than start_, start_ is also updated. Both start and stop positions only relate to the actual
  * sequence characters. In other words, gap positions are not considered when updating the start and stop.
  *
  * Because a Subseq may contain gaps, it is important to understand how adjusting the stop position impacts the
  * underlying bioString_ sequence. Assuming a valid newStop:
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
  * @param start [int] RelAbs coordinate
  * @return bool; whether start position was accepted
  * @see setStart()
  */
bool Subseq::setStop(int newStop)
{
    const BioString anonBioString = anonSeq_.bioString();

    // Must compute the absolute coordinate with respect to the parent Seq/AnonSeq
    newStop = anonBioString.positiveIndex(newStop);

    // Slight optimization
    if (newStop == stop_)
        return true;

    if (newStop >= 1 &&
        newStop <= anonBioString.length())
    {
//        int head_gaps = headGaps();
        int tail_gaps = tailGaps();

        // Case 1
        if (newStop > stop_)
        {
            // Case 1.1
            int n_new_chars = newStop - stop_;
            if (n_new_chars <= tail_gaps)
            {
                // Replace gapped region with new characters
                bioString_->replace(-tail_gaps, n_new_chars, anonBioString.mid(stop_ + 1, n_new_chars));
            }
            // Case 1.2
            else // n_new_chars > tail_gaps
            {
                // Step A: replace any trailing gaps with the equivalent number of characters from the anonBioString
                bioString_->replace(-tail_gaps, tail_gaps, anonBioString.mid(stop_ + 1, tail_gaps));

                // Step B: Append the remainder
                bioString_->append(anonBioString.mid(stop_ + 1 + tail_gaps, n_new_chars - tail_gaps));
            }
        }
        // Case 2: newStop < stop_ && newStop >= start_
        else if (newStop >= start_)
        {
            // Simply need to replace non-gap characters with gaps until we reach the newStop
            QString tmpseq = bioString_->sequence();
            QChar *x = tmpseq.data();
            x += tmpseq.length() - 1 - tail_gaps;

            int n_chars_to_remove = stop_ - newStop;
            while (n_chars_to_remove)
            {
                // Check for non-gap character
                if (!BioString::isGap(*x))
                {
                    *x = constants::kDefaultGapCharacter;     // Defined in BioString.h
                    --n_chars_to_remove;
                }
                --x;
            }

            *bioString_ = tmpseq;
        }
        // Case 3: newStop < start_
        else
        {
            // Step A: Replace all non-gap characters with gaps until we reach the current start_
            QString tmpseq = bioString_->sequence();
            QChar *x = tmpseq.data();
            x += tmpseq.length() - 1 - tail_gaps;

            int n_chars_to_remove = stop_ - start_ + 1;
            while (n_chars_to_remove)
            {
                Q_ASSERT(*x != '\0');

                // Check for non-gap character
                if (!BioString::isGap(*x))
                {
                    *x = constants::kDefaultGapCharacter;     // Defined in BioString.h
                    --n_chars_to_remove;
                }
                --x;
            }

            // Step B: Skip over and add (if necessary) intermediate gaps and add newStop character
            int head_gaps = headGaps();  // Note that this does not reflect the changes we just in the previous while loop
                                         // Because those changes were performed on a temporary QString which has not been reassigned
                                         // to this classes' bioString_
            int intermediate_gaps = start_ - newStop - 1;
            if (head_gaps >= intermediate_gaps + 1)
            {
                x -= intermediate_gaps;
                *x = anonBioString.sequence().at(newStop - 1);
                *bioString_ = tmpseq;
            }
            else
            {
                // Append any remainining gaps along with the appropriate new start character
                *bioString_ = tmpseq;
                bioString_->prepend(QString("-").repeated(intermediate_gaps - head_gaps)).prepend(anonBioString.mid(newStop));
            }

            start_ = newStop;
        }

        // Update the stop position to the new position
        stop_ = newStop;

        // Position was accepted
        return true;
    }

    // Position was invalid
    return false;
}

/**
  * @return int
  * @see headGaps()
  */
int Subseq::tailGaps() const
{
    int tailGaps = 0;
    const QChar *y = bioString_->sequence().constData();
    int pos = bioString_->length() - 1;
    y += pos;
    while (BioString::isGap(*y) &&
           pos >= 0)
    {
        ++tailGaps;
        --y;
        --pos;
    }

    return tailGaps;
}

/**
  * @returns int
  */
int Subseq::ungappedLength() const
{
    return stop_ - start_ + 1;
}
