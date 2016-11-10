/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "BioString.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Does not consider the sequence character at position.
  *
  * Examples:
  * 12345678
  * AB----CD
  *
  * gapsLeftOf(3) -> 0
  * gapsLeftOf(5) -> 2
  * gapsLeftOf(7) -> 4
  * gapsLeftOf(8) -> 0
  *
  * @param position [int] 1-based
  * @return int
  */
int BioString::gapsLeftOf(int position) const
{
    position = positiveIndex(position);

    if (position > 1 &&
        position <= sequence_.length())
    {
        const QChar *x = sequence_.constData();

        // Q: Why do we subtract 2 here?
        // A: position is 1-based, but QString is zero-based, so we subtract 1
        //    This function does not consider the character at position, so we subtract 1
        x += position - 2;

        int n_gaps = 0;
        while (BioString::isGap(*x))
        {
            --x;
            ++n_gaps;
        }

        return n_gaps;
    }

    return 0;
}

/**
  * Does not consider the sequence character at position.
  *
  * Examples:
  * 12345678
  * AB----CD
  *
  * gapsRightOf(1) -> 0
  * gapsRightOf(2) -> 4
  * gapsRightOf(5) -> 1
  * gapsRightOf(6) -> 0
  *
  * @param position [int] 1-based
  * @return int
  */
int BioString::gapsRightOf(int position) const
{
    position = positiveIndex(position);

    if (position >= 1 &&
        position < sequence_.length())
    {
        const QChar *x = sequence_.constData();

        // Q: Why do we subtract 2 here?
        // A: position is 1-based, but QString is zero-based, so we subtract 1
        //    This function does not consider the character at position, so we subtract 1
        x += position;

        int n_gaps = 0;
        while (BioString::isGap(*x))
        {
            ++x;
            ++n_gaps;
        }

        return n_gaps;
    }

    return 0;
}



/**
  * Searches for bioString starting at position from (1-based). Unlike QString, 0 is returned if bioString is empty or
  * bioString is not found. From by default begins at position 1 of the sequence. Gap positions regardless of the exact
  * gap symbol are considered equivalent.
  *
  * >>> Examples:
  * 12345678901
  * ABC-D.E-ABC
  *
  * indexOf(BioString(".D.E")) -> 4
  * indexOf(BioString("E-A")) -> 7
  * indexOf(BioString(".D-E.A")) -> 4
  * indexOf(BioString("E.E")) -> 0
  *
  * indexOf(BioString("ABC")) -> 1
  * indexOf(BioString("ABC"), 2) -> 9
  * indexOf(BioString("ABC"), -4) -> 9
  * indexOf(BioString("BC"), 4) -> 10
  * indexOf(BioString("BC"), 10) -> 10
  * indexOf(BioString("BC"), 11) -> 0
  *
  * @param bioString [const BioString &] bioString to find within this object
  * @param from [int] 1-based start RelAbs position to begin searching forward from
  * @return int; 0 if not found, otherwise absolute positive index where bioString starts
  */
int BioString::indexOf(const BioString &bioString, int from) const
{
    int len = length();

    if (bioString.length() == 0 ||
        from > len)
    {
        return 0;
    }

    if (from == 0)
        from = 1;
    else if (from < 0)
    {
        from = positiveIndex(from);

        // The QString indexOf function will return not found (-1) if it is passed a negative value for from. We deviate
        // from this approach and begin searching at the beginning even if the computed from is negative. To make this
        // work with QString's indexOf function, we clamp from to 1. For more info see the comments in
        // BioString::segment(...)
        if (from < 1)
            from = 1;
    }

    // To make search independent of the gap character, simply make a copy with all gaps represented as dashes
    // and compare the QStrings.
    QString subject = sequence_;
    QString query = bioString.sequence_;

    subject.replace(".", "-");
    query.replace(".", "-");

    return subject.indexOf(query, from-1) + 1;
}

/**
  * Position may be expressed in RelAbs coordinates. bioString is only inserted if position references a valid index
  * inside the sequence boundaries (either using normal or negative indices) or following the last sequence character.
  * Specifically, the valid range is 1..n+1 and -1 .. -n, where n = length of BioString. This slightly differs from
  * other methods in which 0 (the default value) implies a particular sequence terminus.
  *
  * Insertion is done at the given index, not after the character as this index.
  *
  * ABCDEF
  * insert(1, "XYZ") = insert(-6, "XYZ") -> XYZABCDEF
  * insert(-1, "XYZ") -> ABCDEXYZF
  * insert(6, "---") -> ABCDE---F
  * insert(7, "..")  -> ABCDEF..
  * insert(-3, "ST") -> ABCSTDEF
  *
  * Insertions outside the allowed range do not change the sequence
  * insert(0, "xyz") -> ABCDEF
  * insert(-7, "me") -> ABCDEF
  * insert(-32, "them") -> ABCDEF
  * insert(8, "you") -> ABCDEF
  *
  * @param position [int]
  * @param bioString [const BioString &]
  * @return BioString &
  * @see remove()
  */
BioString &BioString::insert(int position, const BioString &bioString)
{
    if (position < 0)
        position = positiveIndex(position);

    if (!bioString.sequence_.isEmpty() &&
        position >= 1 &&
        position <= sequence_.length() + 1)
    {
        sequence_.insert(position-1, bioString.sequence_);
    }

    return *this;
}

/**
  * Position may be expressed in RelAbs coordinates. bioString is only inserted if position references a valid index
  * inside the sequence boundaries (either using normal or negative indices) or following the last sequence character.
  * Specifically, the valid range is 1..len+1 and -1 .. -len, where len = length of BioString. This slightly differs
  * from other methods in which 0 (the default value) implies a particular sequence terminus. Insertion is done at the
  * given index, not after the character as this index.
  *
  * Note: if n < 1, no gaps are inserted.
  *
  * Examples:
  * ABCDEF
  * insertGaps(1, 2) -> --ABCDEF
  * insertGaps(-1, 2) -> ABCDE--F
  * insertGaps(0, 3) -> ABCDEF
  * insertGaps(3, 0) -> ABCDEF
  * insertGaps(-7, 3) -> ABCDEF
  *
  * @param position [int]
  * @param n [int]
  * @param gapCharacter [char]
  * @return BioString &
  */
BioString &BioString::insertGaps(int position, int n, char gapCharacter)
{
    if (position < 0)
        position = positiveIndex(position);

    if (n > 0 &&
        position >= 1 &&
        position <= sequence_.length() + 1)
    {
        sequence_.insert(position-1, QString(gapCharacter).repeated(n));
    }

    return *this;
}

/**
  * Test validity directly on BioString object
  * @return bool
  * @see isValid()
  */
bool BioString::isValid() const
{
    QByteArray string = sequence_.toAscii();
    for (int i=0, z= string.size(); i< z; ++i)
        if (!isValid(string.at(i)))
            return false;

    return true;
}

/**
  * The number of positions that the segment (start..stop) may be slid depends on whether it contains non-gap characters.
  * A segment containing non-gap characters may only be slid the number of gap characters that immediately bound the
  * segment on the respective side. If it contains solely gap characters, these may be moved any amount up to the
  * sequence boundaries.
  *
  * Examples:
  *
  * 123456789
  * AB--C-D-E
  *
  * leftSlidablePositions(5, 7) = 2
  * leftSlidablePositions(6, 6) = 5
  * leftSlidablePositions(3, 5) = 0
  *
  * @param start [int]
  * @param stop [int]
  * @return int
  * @see rightSlidablePositions(), slideSegment()
  */
int BioString::leftSlidablePositions(int start, int stop) const
{
    int potential_delta = 0;        // The distance the segment may be successfully moved to the left
    start = positiveIndex(start);
    stop = positiveIndex(stop);

    Q_ASSERT_X(start >= 1, "BioString::leftSlidablePositions", "start must be >= 1");
    Q_ASSERT_X(start <= sequence_.length(), "BioString::leftSlidablePositions", "start must be <= length()");

    Q_ASSERT_X(stop >= 1, "BioString::leftSlidablePositions", "stop must be >= 1");
    Q_ASSERT_X(stop <= sequence_.length(), "BioString::leftSlidablePositions", "stop must be <= length()");

    Q_ASSERT_X(start <= stop, "BioString::leftSlidablePositions", "start must be <= stop");

    if (start >= 1 &&
        start <= stop &&
        stop <= sequence_.length())
    {
        // Does this segment have gaps
        BioString src_seg = segment(start, stop);

        if (src_seg.hasCharacters())
            potential_delta = gapsLeftOf(start);
        else
            potential_delta = start - 1;
    }

    return potential_delta;
}

/**
  * Return a copy of the sequence with all invalid characters replaced with maskCharacter.
  * @param maskCharacter [const char] the character to replace invalid characters with
  * @return QString the masked sequence
  */
QString BioString::masked(const char &maskCharacter) const
{
    QByteArray string = sequence_.toAscii();
    for (int i=0, z= string.size(); i< z; ++i)
        if (!isValid(string.at(i)))
            string[i] = maskCharacter;

    return string;
}

/**
  * Start may be expressed using either positive or negative indices. Returns a NULL string if n is less
  * than 1 or start is greater than the sequence length. If n is excluded, it defaults to 1 character. In essence,
  * this method returns the substring that overlaps a window of length n, beginning at the start index (1-based).
  *
  * >>> Examples
  *
  *  1  2  3  4  5  6  7  8
  *  A  B  C  D  E  F  G  H
  * -8 -7 -6 -5 -4 -3 -2 -1
  *
  * mid(3)      -> C
  * mid(6, 3)   -> FGH
  * mid(-7)     -> B
  * mid(-8, 5)  -> ABCDE
  * mid(-12, 3) -> null
  * mid(9)      -> null
  *
  * @param start [int] 1-based start position to begin the substring
  * @param n [int] number of characters to return
  * @return QString
  * @see segment()
  */
QString BioString::mid(int start, int n) const
{
    int len = sequence_.length();
    if (start > len || n < 1)
        return "";

    if (start == 0)
        start = 1;
    else if (start < 0)
        start = positiveIndex(start);

    return sequence_.mid(start - 1, n);
}

/**
  * @param start [int]
  * @param stop [int]
  * @returns int
  */
int BioString::nonGapCharsBetween(int start, int stop) const
{
    ASSERT_X(start > 0 && start <= sequence_.length(), "start out of range");
    ASSERT_X(stop >= start && stop <= sequence_.length(), "stop out of range");

    const QChar *ch = sequence_.constData() + start - 1;

    int nNonGapChars = 0;

    int i = start - 1;
    while (i != stop)
    {
        ASSERT_X(*ch != '\0', "Unexpected zero terminated character");

        if (!isGap(*ch))
            ++nNonGapChars;

        ++i;
        ++ch;
    }

    return nNonGapChars;
}

/**
  * Return a copy of the sequence with all gaps removed and invalid characters replaced with maskCharacter.
  * @param maskCharacter [const char] the character to replace invalid characters with
  * @return QString the reduced sequence
  */
QString BioString::reduced(const char &maskCharacter) const
{
    static BioString reduced;
    reduced = masked(maskCharacter);
    return reduced.ungapped();
}

/**
  * Return a copy of the sequence with all gaps removed and invalid characters replaced with BioString::defaultMaskCharacter_.
  * This function is also declared so that derived classes may perform masking using the appropriate defaultMaskCharacter
  * @return QString the reduced sequence
  */
QString BioString::reduced() const
{
    static BioString reduced;
    reduced = masked();
    return reduced.ungapped();
}

/**
  * Removing is done at the given index, not afterwards. The sequence is only modified if position points to a valid
  * index and the number of characters to remove, n, is greater than 0. If position + n extends beyond the sequence
  * terminus.
  *
  * ABCDEF
  * remove(0, 1) -> ABCDEF    // Calls with position of 0 always return an unmodified sequence
  * remove(0, 34) -> ABCDEF
  * remove(1, 1) -> BCDEF
  * remove(3, 1) -> ABDEF
  * remove(3, 3) -> ABF
  * remove(3, 20) -> AB       // If position + n >= length, all characters from position to the end of the string are removed
  * remove(-1, 1) -> ABCDE
  * remove(-6, 2) -> CDEF
  * remove(-7, 3) -> ABCDEF
  *
  * @param position [int] a RelAbs coordinate
  * @param n [int]
  * @return BioString &
  * @see insert()
  */
BioString &BioString::remove(int position, int n)
{
    if (position < 0)
        position = positiveIndex(position);

    if (n > 0 &&
        position >= 1 &&
        position <= sequence_.length())
    {
        sequence_.remove(position-1, n);
    }

    return *this;
}

/**
  * Convenience function for removing one or more contiguous gaps starting at position. Position must reside within the
  * sequence bounds and reference a gap character, otherwise the BioString will be returned unchanged.
  *
  * @param position [int]
  * @param n [int]
  * @return BioString &
  * @see insertGaps()
  */
BioString &BioString::removeGaps(int position, int n)
{
    if (position < 0)
        position = positiveIndex(position);

    if (n > 0 &&
        position >= 1 &&
        position <= sequence_.length() &&
        isGap(sequence_.at(position-1)))
    {
        const QChar *x = sequence_.constData();
        x += position;
        int n_gaps_to_remove = 1;
        while (n_gaps_to_remove < n &&
               isGap(*x))
        {
            ++x;
            ++n_gaps_to_remove;
        }

        sequence_.remove(position-1, n_gaps_to_remove);
    }

    return *this;
}


/**
  * Similar to replaceSegment; however, instead of specifying both endpoints, this function takes a starting position,
  * and number of characters to replace, n. Position may be expressed using either positive or negative indices. Sequence
  * is not modified if n is less than 1 or position does not reference valid 1-based, index.
  *
  * @param position [int]
  * @param n [int]
  * @param replacement [const BioString &]
  * @return const BioString &
  * @see replaceSegment()
  */
BioString &BioString::replace(int position, int n, const BioString &replacement)
{
    if (position < 0)
        position = positiveIndex(position);

    if (position >= 1 &&
        position <= sequence_.length() &&
        n >= 1)
    {
        sequence_.replace(position - 1, n, replacement.sequence_);
    }

    return *this;
}

/**
  * Invalid indices (end < start or both end and start are outside the 1-based bounds) do not modify the sequence.
  *
  * If start is inside the bounds and the end is outside of the bounds, clamp end to the right terminus of the sequence
  * and vice versus. This again assumes that absolute start <= absolute end.
  *
  * @param start [int]
  * @param stop [int]
  * @param replacement [const BioString &]
  * @return BioString &
  * @see segment()
  */
BioString &BioString::replaceSegment(int start, int stop, const BioString &replacement)
{
    int len = sequence_.length();

    // Handle the zero case and convert to absolute coordinates with respect to sequence
    if (start == 0)
        start = 1;
    else if (start < 0)
    {
        start = positiveIndex(start);

        if (start < 1)
            start = 1;
    }

    if (stop == 0)
        stop = len;
    else if (stop < 0)
        stop = positiveIndex(stop);

    if (start <= len &&
        stop >= 1 &&
        start <= stop)
    {
        sequence_.replace(start-1, stop-start+1, replacement.sequence_);
    }

    return *this;
}

/**
  * The number of positions that the segment (start..stop) may be slid depends on whether it contains non-gap characters.
  * A segment containing non-gap characters may only be slid the number of gap characters that immediately bound the
  * segment on the respective side. If it contains solely gap characters, these may be moved any amount up to the
  * sequence boundaries.
  *
  * Examples:
  *
  * 123456789
  * AB--C-D-E
  *
  * rightSlidablePositions(5, 7) = 1
  * rightSlidablePositions(6, 6) = 3
  * rightSlidablePositions(2, 4) = 0
  *
  * @param start [int]
  * @param stop [int]
  * @return int
  * @see leftSlidablePositions(), slideSegment()
  */
int BioString::rightSlidablePositions(int start, int stop) const
{
    int potential_delta = 0;        // The distance the segment may be successfully moved to the right
    start = positiveIndex(start);
    stop = positiveIndex(stop);

    Q_ASSERT_X(start >= 1, "BioString::rightSlidablePositions", "start must be >= 1");
    Q_ASSERT_X(start <= sequence_.length(), "BioString::rightSlidablePositions", "start must be <= length()");

    Q_ASSERT_X(stop >= 1, "BioString::rightSlidablePositions", "stop must be >= 1");
    Q_ASSERT_X(stop <= sequence_.length(), "BioString::rightSlidablePositions", "stop must be <= length()");

    Q_ASSERT_X(start <= stop, "BioString::rightSlidablePositions", "start must be <= stop");

    if (start >= 1 &&
        start <= stop &&
        stop <= sequence_.length())
    {
        // Does this segment have gaps
        BioString src_seg = segment(start, stop);

        if (src_seg.hasCharacters())
            potential_delta = gapsRightOf(stop);
        else
            potential_delta = sequence_.length() - stop;
    }

    return potential_delta;
}


/**
  * Return the raw sequence between the provided start and stop coordinates (e.g. region between 45 and 100, inclusive).
  * Both start and stop are 1-based for convenience and may be expressed as RelAbs coordinates. RelAbs provide
  * for conveniently expressing absolute positions or positions relative to a terminus of the sequence. By default,
  * any positive number is relative to the left bound (or N-terminus). In this case, relative and absolute coordinates
  * will be identical. On the other hand, negative numbers are relative to the right bound or (C-terminus). A zero
  * start indicates the left bound, and a zero end indicates the right bound.
  *
  * Invalid RelAbs coordinate cases are shown below and return a NULL QString
  * a) end < start
  * b) both end and start are outside the 1-based bounds
  *
  * If start is inside the bounds and the end is outside of the bounds, clamp end to the right terminus of the sequence
  * and vice versus. This again assumes that absolute start <= absolute end.
  *
  * Examples:
  *
  *  1  2  3  4  5  6  7  8
  *  A  B  C  D  E  F  G  H
  * -8 -7 -6 -5 -4 -3 -2 -1
  *
  * Position: 6 = F
  * Position: -6 = C
  * Position: start of 0 -> 1 = A
  * Position: end of 0 -> 8 = H
  *
  * >>> Normal ranges:
  * Range: 0..0 -> 1..8 = ABCDEFGH
  * Range: 0..3 -> 1..3 = ABC
  * Range: 5..5 -> E
  * Range: -6..-4 -> 3..5 = CDE
  * Range: 1..-2 -> 1..7 = ABCDEFG
  * Range: -7..4 -> 2..4 = BCD
  *
  * >>> Clamped ranges:
  * Range: -10..5 -> 1..5 = ABCDE
  * Range: 6..30 -> 6..8 = FGH
  * Range: -10..-5 -> 1..4 = ABCD
  *
  * >>> Ranges which return null:
  * Range: 5..3 -> null
  * Range: 9..15 -> null
  * Range: -9..-9 -> -1..-1 = null
  *
  * @param start [int] 1-based RelAbs coordinate
  * @param stop [int] 1-based RelAbs coordinate
  * @return QString
  */
QString BioString::segment(int start, int stop) const
{
    int len = sequence_.length();

    // Handle the zero case and convert to absolute coordinates with respect to sequence
    if (start == 0)
        start = 1;
    else if (start < 0)
    {
        start = positiveIndex(start);

        // Clamp the start to 1 for any negative start whose absolute value is greater than len. If
        // we did not do this, start would be off by one during the mid operation which is used to return
        // the substring. For example:
        // ABC
        // segment(-4, 1) is translated into a 1-based start of 0
        // - This would result in a mid operation: mid(-1, 1)
        // segment(-6, 1) is translated into a 1-based start of -2
        // - This would result in a mid operation: mid(-3, 1)
        // Both of these mid operations would return the empty string even though the segment extends
        // to a valid stop position within the sequence.
        //
        // Solution: map all instances of a translated 1-based start < 0 to 1
        if (start < 1)
            start = 1;

        // This logic is asymmetrically applied to the start input because technically it is essential for the
        // QString.mid method to return the expected/proper result and the stop position is not constrained in the
        // same manner.
    }

    if (stop == 0)
        stop = len;
    else if (stop < 0)
        stop = positiveIndex(stop);

    if (start <= len &&
        stop >= 1 &&
        start <= stop)
    {
        // QString functions are 0-based, so we must subtract 1 from the 1-based start position
        return sequence_.mid(start-1, stop-start+1);
    }

    return "";
}

/**
  * A frequent operation while editing multiple sequence alignments is horizontally sliding a set of characters which
  * "displace" or exchange places with gap positions but not any non-gap character data. Thus, during this method the
  * order of sequence characters is not modified - only the gap positions. The delta parameter specifies the maximum
  * number of positions to slide within.
  *
  * A negative delta indicates to slide the characters to the left and vice versa.
  *
  * Both start_pos and stop_pos must point to valid 1-based character positions and the absolute stop_pos must be greater
  * than the absolute start_pos or 0 will be returned and the sequence will remain unchanged.
  *
  * While sliding is most often considered in light of moving actual sequence characters, it is also possible
  * to slide a set of gap characters which behave slightly differently. Because swapping positions of a gap does not
  * alter the actual ungapped sequence, it is possibly to move a gap anywhere within the sequence bounds.
  *
  * Example:
  * 1234567890123
  * ABC--D-EF--GH
  *
  * slide(6, 9, -1) -> 1, sequence = ABC-D-EF---GH
  * slide(6, 9, -2) -> 2, sequence = ABCD-EF----GH
  * slide(6, 9, -5) -> same thing as above
  *
  * slide(9, 11, 2) -> 0, unchanged sequence
  * slide(9, 10, 2) -> 1, sequence = ABC--D-E-F-GH
  *
  * @param start_pos [int]
  * @param stop_pos [int]
  * @param delta [int]
  * @return int - the number of positions that the segment was successfully moved (0..qAbs(delta))
  */
int BioString::slideSegment(int start_pos, int stop_pos, int delta)
{
    int actual_delta = 0;                  // Stores the distance (in characters) segment was successfully moved
    start_pos = positiveIndex(start_pos);
    stop_pos = positiveIndex(stop_pos);

    Q_ASSERT_X(start_pos >= 1, "BioString::slideSegment", "start_pos must be >= 1");
    Q_ASSERT_X(start_pos <= sequence_.length(), "BioString::slideSegment", "start_pos must be <= length()");

    Q_ASSERT_X(stop_pos >= 1, "BioString::slideSegment", "stop_pos must be >= 1");
    Q_ASSERT_X(stop_pos <= sequence_.length(), "BioString::slideSegment", "stop_pos must be <= length()");

    Q_ASSERT_X(start_pos <= stop_pos, "BioString::slideSegment", "start_pos must be <= stop_pos");

    if (start_pos >= 1 &&
        start_pos <= stop_pos &&
        stop_pos <= sequence_.length() &&
        delta != 0)
    {
        // Does this segment have gaps
        BioString src_seg = segment(start_pos, stop_pos);

        if (delta < 0)  // Slide to the left
        {
            actual_delta = qMin(-delta, leftSlidablePositions(start_pos, stop_pos));

            if (actual_delta)
            {
                // A. Get the exact gap representation to the left of the tmpseg
                BioString left_seg = mid(start_pos - actual_delta, actual_delta);

                // B. Swap places
                replaceSegment(start_pos - actual_delta, stop_pos, src_seg + left_seg);
            }
        }
        else // (delta > 0) -> Slide to the right
        {
            actual_delta = qMin(delta, rightSlidablePositions(start_pos, stop_pos));

            if (actual_delta)
            {
                // A. Get the exact gap representation to the left of the tmpseg
                BioString right_seg = mid(stop_pos + 1, actual_delta);

                // B. Swap places
                replaceSegment(start_pos, stop_pos + actual_delta, right_seg + src_seg);
            }
        }
    }

    return actual_delta;
}

/**
  * @param ch [char]
  * @returns QString
  */
QString BioString::substituteGapsWith(char ch) const
{
    QString substituted = sequence_;

    QChar *x = substituted.data();
    while (*x != '\0')
    {
        if (isGapCharacter(*x))
            *x = ch;
        ++x;
    }

    return substituted;
}

/**
  * Return a copy of the sequence with all gap characters (- and/or .) removed.
  * @see removeGaps()
  * @return QString ungapped sequence
  */
QString BioString::ungapped() const
{
    QString ungapped = sequence_;

    ungapped.remove('-');
    ungapped.remove('.');

    return ungapped;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static public methods
/**
  * Removes all whitespace characters (specifically: [space] \t \n \v \f \r) from sequence and uppercases all characters.
  *
  * OPTIMIZATION: Track all contiguous spans of whitespace and then remove those in a subsequent step
  *
  * @param sequence QString representation of an arbitrary sequence
  * @return QString
  */
QString BioString::normalize(QString sequence)
{
    // Based on benchmark tests, removal of individual characters in the following manner is about 10x
    // faster than using a RegExp. Of course, this was using a debug build so it might be different when
    // the comparison is performed with optimized code
    sequence.remove(Qt::Key_Space);
    sequence.remove('\t');
    sequence.remove('\n');
    sequence.remove('\v');
    sequence.remove('\f');
    sequence.remove('\r');

    return sequence.toUpper();
}
