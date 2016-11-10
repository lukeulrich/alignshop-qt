/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Msa.h"
#include "global.h"
#include "macros.h"
#include "misc.h"

class SubseqLessThanHelperPrivate
{
public:
    SubseqLessThanHelperPrivate(const ISubseqLessThan *lessThanHelper) : lessThanHelper_(lessThanHelper) {}

    inline bool operator()(const Subseq *a, const Subseq *b) const
    {
        return lessThanHelper_->lessThan(a, b);
    }

private:
    const ISubseqLessThan *lessThanHelper_;
};

class SubseqGreaterThanHelperPrivate
{
public:
    SubseqGreaterThanHelperPrivate(const ISubseqLessThan *lessThanHelper) : lessThanHelper_(lessThanHelper) {}

    inline bool operator()(const Subseq *a, const Subseq *b) const
    {
        return lessThanHelper_->lessThan(b, a);
    }

private:
    const ISubseqLessThan *lessThanHelper_;
};


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param grammar [Grammar]
  */
Msa::Msa(Grammar grammar) : grammar_(grammar)
{
}

/**
  * This class takes ownership of all member subseqs and therefore is responsible for releasing their
  * memory allocation.
  */
Msa::~Msa()
{
    clear();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * Note: 1-based, not 0-based.
  *
  * @param i [int]
  * @returns const Subseq *
  */
const Subseq *Msa::operator[](int i) const
{
    return subseqs_.at(i-1);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * An exception will be thrown if these conditions are not satisfied.
  *
  * @param subseq [Subseq *]
  * @returns bool
  */
bool Msa::append(Subseq *subseq)
{
    if (isCompatibleSubseq(subseq))
    {
        subseqs_ << subseq;
        return true;
    }

    return false;
}

/**
  * Identical and duplicate to the operator() method. Code reproduced here for performance reasons.
  *
  * @param i [int]
  * @returns const Subseq *
  */
const Subseq *Msa::at(int i) const
{
    ASSERT_X(i >= 1 && i <= subseqCount(), "index out of range");

    return subseqs_[i-1];
}

/**
  * @param msaRect [const PosiRect &]
  * @returns bool
  */
bool Msa::canCollapseLeft(const PosiRect &msaRect) const
{
    if (msaRect.isNull())
        return false;

    ASSERT_X(isValidColumn(msaRect.left()) && isValidColumn(msaRect.right()), "msaRect columns out of range");
    ASSERT_X(isValidRowRange(msaRect.verticalRange()), "msaRect rows out of range");

    // Scan for the first gap, non-gap character pairing
    for (int i=msaRect.top(), z=msaRect.bottom(); i<=z; ++i)
    {
        const char *x = subseqFromRow(i)->constData() + msaRect.left() - 1;

        // Find the first gap character in this range
        int firstGap = -1;
        for (int j=0; j< msaRect.width(); ++j, ++x)
        {
            if (::isGapCharacter(*x))
            {
                firstGap = msaRect.left() + j;
                break;
            }
        }
        if (firstGap == -1)
            continue;

        // Now search for a non-gap character in the remainder
        ++x;
        for (int j=firstGap+1; j<= msaRect.right(); ++j, ++x)
            if (!::isGapCharacter(*x))
                return true;
    }

    return false;
}

/**
  * @param msaRect [const PosiRect &]
  * @returns bool
  */
bool Msa::canCollapseRight(const PosiRect &msaRect) const
{
    if (msaRect.isNull())
        return false;

    ASSERT_X(isValidColumn(msaRect.left()) && isValidColumn(msaRect.right()), "msaRect columns out of range");
    ASSERT_X(isValidRowRange(msaRect.verticalRange()), "msaRect rows out of range");
    ASSERT_X(isValidColumn(msaRect.left()) && isValidColumn(msaRect.right()), "msaRect columns out of range");
    ASSERT_X(isValidRowRange(msaRect.verticalRange()), "msaRect rows out of range");

    // Scan for the first gap, non-gap character pairing
    for (int i=msaRect.top(), z=msaRect.bottom(); i<=z; ++i)
    {
        const char *x = subseqFromRow(i)->constData() + msaRect.right() - 1;

        // Find the first gap character in this range
        int firstGap = -1;
        for (int j=0; j< msaRect.width(); ++j, --x)
        {
            if (::isGapCharacter(*x))
            {
                firstGap = msaRect.right() - j;
                break;
            }
        }
        if (firstGap == -1)
            continue;

        // Now search for a non-gap character in the remainder
        --x;
        for (int j=firstGap-1; j>= msaRect.left(); --j, --x)
            if (!::isGapCharacter(*x))
                return true;
    }

    return false;
}

/**
  * Only invalid value acceptable is 0.
  *
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canExtendLeft(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    for (int i=rows.begin_; i<= rows.end_; ++i)
        if (leftExtendableLength(msaColumn, i) > 0)
            return true;

    return false;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canExtendRight(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    for (int i=rows.begin_; i<= rows.end_; ++i)
        if (rightExtendableLength(msaColumn, i) > 0)
            return true;

    return false;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canLevelLeft(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    if (msaColumn > 1)
        return canTrimLeft(msaColumn - 1, rows) || canExtendLeft(msaColumn, rows);

    return canExtendLeft(msaColumn, rows);
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canLevelRight(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    if (msaColumn < length())
        return canTrimRight(msaColumn + 1, rows) || canExtendRight(msaColumn, rows);

    return canExtendRight(msaColumn, rows);
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canTrimLeft(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    for (int i=rows.begin_; i<= rows.end_; ++i)
        if (leftTrimmableLength(msaColumn, i) > 0)
            return true;

    return false;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::canTrimRight(int msaColumn, const ClosedIntRange &rows) const
{
    if (msaColumn == 0)
        return false;

    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    for (int i=rows.begin_; i<= rows.end_; ++i)
        if (rightTrimmableLength(msaColumn, i) > 0)
            return true;

    return false;
}

/**
  * Frees all subseqs. Nothing is done directly with any associated IEntity pointers contained by the subseqs. Any
  * necessary action relating to such entities should be dealt with before calling this method.
  */
void Msa::clear()
{
    qDeleteAll(subseqs_);
    subseqs_.clear();
}

/**
  * Maximally shifts all non-gap characters in msaRect to the left.
  *
  * @param msaRect [const PosiRect &]
  * @returns SubseqChangePodVector
  */
SubseqChangePodVector Msa::collapseLeft(const PosiRect &msaRect)
{
    PosiRect normalizedMsaRect = msaRect.normalized();

    ASSERT_X(normalizedMsaRect.isValid(), "msaRect is not valid");
    ASSERT_X(normalizedMsaRect.right() <= length(), "msaRect.right out of range");
    ASSERT_X(normalizedMsaRect.bottom() <= subseqCount(), "msaRect.bottom out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(msaRect.height());
    for (int row=normalizedMsaRect.top(), bottom = normalizedMsaRect.bottom(); row <= bottom; ++row)
    {
        Subseq *subseq = subseqFromRow(row);
        const ClosedIntRange &horizRange = normalizedMsaRect.horizontalRange();
        BioString difference = subseq->mid(horizRange);
        ClosedIntRange collapseRange = subseq->collapseLeft(horizRange);
        if (collapseRange.isEmpty() == false)
        {
            if (collapseRange.begin_ > horizRange.begin_)
                difference = difference.mid(collapseRange.begin_ - horizRange.begin_ + 1, collapseRange.length());
            else if (collapseRange.end_ < horizRange.end_)
                difference.chop(horizRange.end_ - collapseRange.end_);

            pods << SubseqChangePod(row, collapseRange, SubseqChangePod::eInternal, difference);
        }
    }

    pods.squeeze();
    return pods;
}

/**
  * @param msaRect [const PosiRect &]
  * @returns SubseqChangePodVector
  */
SubseqChangePodVector Msa::collapseRight(const PosiRect &msaRect)
{
    PosiRect normalizedMsaRect = msaRect.normalized();

    ASSERT_X(normalizedMsaRect.isValid(), "msaRect is not valid");
    ASSERT_X(normalizedMsaRect.right() <= length(), "msaRect.right out of range");
    ASSERT_X(normalizedMsaRect.bottom() <= subseqCount(), "msaRect.bottom out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(msaRect.height());
    for (int row=normalizedMsaRect.top(), bottom = normalizedMsaRect.bottom(); row <= bottom; ++row)
    {
        Subseq *subseq = subseqFromRow(row);
        const ClosedIntRange &horizRange = normalizedMsaRect.horizontalRange();
        BioString difference = subseq->mid(horizRange);
        ClosedIntRange collapseRange = subseq->collapseRight(horizRange);
        if (collapseRange.isEmpty() == false)
        {
            if (collapseRange.begin_ > horizRange.begin_)
                difference = difference.mid(collapseRange.begin_ - horizRange.begin_ + 1, collapseRange.length());
            else if (collapseRange.end_ < horizRange.end_)
                difference.chop(horizRange.end_ - collapseRange.end_);

            pods << SubseqChangePod(row, collapseRange, SubseqChangePod::eInternal, difference);
        }
    }

    pods.squeeze();
    return pods;
}

/**
  * Identical to length. Reimplemented directly here for performance reasons.
  *
  * @returns int
  * @see length()
  */
int Msa::columnCount() const
{
    return (subseqs_.size() > 0) ? subseqs_.at(0)->length() : 0;
}

/**
  * In contrast to its sister extendLeft methods, this method targets a group of subseqs and maximally extends (if any)
  * the subseqs to msaColumn.
  *
  * Each subseq may only be extended if:
  * 1) msaColumn references a valid column
  * 1) Zero or more contiguous gaps occur before msaColumn (the extension occurs on the terminus of the subseq)
  * 2) There is at least one gap at or downstream of msaColumn
  *
  * Example of how this method works:
  * 123456789
  * --C-DEF--
  * -XY-ZZZ-W
  *
  * msa.extendSubseqsLeft(2, ClosedIntRange(1, 2));
  * -BC-DEF--
  * -XY-ZZZ-W
  *
  * msa.extendSubseqsLeft(1, ClosedIntRange(1, 2));
  * ABC-DEF--
  * WXY-ZZZ-W
  *
  * Only can add characters to the alignment by decreasing the start position of individual subseqs. Obviously extending
  * the subseqs is constrained by the actual subseqs length and current position. In other words, if a subseq begins at
  * position 1, it cannot be extended more at its leftmost terminus.
  *
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  * @see setSubseqStart(), extendRight()
  */
QVector<SubseqChangePod> Msa::extendLeft(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(rows.length());
    for (int i=rows.begin_; i<= rows.end_; ++i)
    {
        int nNewCharacters = leftExtendableLength(msaColumn, i);
        if (nNewCharacters > 0)
            pods << extendLeft(i, nNewCharacters);
    }

    // Very likely that not all sequences were extended; release the unused memory
    pods.squeeze();

    return pods;
}

/**
  * @param row [int]
  * @param nCharsToExtend [int]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::extendLeft(int row, int nCharsToExtend)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(nCharsToExtend > 0, "nChars must be positive");
    ASSERT_X(subseqFromRow(row)->leftUnusedLength() >= nCharsToExtend, "lefUnusedSpace smaller than nChars");
    ASSERT_X(subseqFromRow(row)->headGaps() >= nCharsToExtend, "Not enough head gaps to accommodate extension");

    Subseq *subseq = subseqFromRow(row);
    int column = subseq->headGaps() - nCharsToExtend + 1;
    subseqFromRow(row)->extendLeft(column, ClosedIntRange(subseq->start() - nCharsToExtend, subseq->start() - 1));
    ClosedIntRange extensionRange = ClosedIntRange(column, column + nCharsToExtend - 1);
    return SubseqChangePod(row,
                           extensionRange,
                           SubseqChangePod::eExtendLeft,
                           subseq->mid(extensionRange));
}

/**
  * Most likely the inverse of a trim operation. Necessary to make extensions that involve gaps. Does not perform any
  * optimizations with respect to extension. If there are gaps, they are not stripped.
  *
  * Requires: at least one non-gap in extension; msaColumn + extension - 1 will fit in the headgaps
  *
  * @param msaColumn [int]
  * @param row [int]
  * @param extension [BioString]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::extendLeft(int msaColumn, int row, const BioString &extension)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT(extension.isEmpty() == false);
    ASSERT(extension.hasNonGaps());
    ASSERT_X(msaColumn + extension.length() - 1 <= at(row)->headGaps(), "extension must fit in head gaps");

    subseqFromRow(row)->extendLeft(msaColumn, extension);

    return SubseqChangePod(row,
                           ClosedIntRange(msaColumn, msaColumn + extension.length() - 1),
                           SubseqChangePod::eExtendLeft,
                           extension);
}

/**
  * Performs similarly to extendLeft except applies to the right boundary of the alignment.
  *
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> Msa::extendRight(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(rows.length());
    for (int i=rows.begin_; i<= rows.end_; ++i)
    {
        int nNewCharacters = rightExtendableLength(msaColumn, i);
        if (nNewCharacters > 0)
            pods << extendRight(i, nNewCharacters);
    }

    // Very likely that not all sequences were extended; release the unused memory
    pods.squeeze();

    return pods;
}

/**
  * @param row [int]
  * @param nCharsToExtend [int]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::extendRight(int row, int nCharsToExtend)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(nCharsToExtend > 0, "nChars must be positive");
    ASSERT_X(subseqFromRow(row)->rightUnusedLength() >= nCharsToExtend, "rightUnusedSpace smaller than nChars");
    ASSERT_X(subseqFromRow(row)->tailGaps() >= nCharsToExtend, "Not enough tail gaps to accommodate extension");

    Subseq *subseq = subseqFromRow(row);
    int column = subseq->length() - subseq->tailGaps() + 1;
    subseqFromRow(row)->extendRight(column, ClosedIntRange(subseq->stop() + 1, subseq->stop() + nCharsToExtend));
    ClosedIntRange extensionRange = ClosedIntRange(column, column + nCharsToExtend - 1);
    return SubseqChangePod(row,
                           extensionRange,
                           SubseqChangePod::eExtendRight,
                           subseq->mid(extensionRange));
}

/**
  * Most likely the inverse of a trim operation. Necessary to make extensions that involve gaps. Does not perform any
  * optimizations with respect to extension. If there are gaps, they are not stripped.
  *
  * Requires: at least one non-gap in extension; msaColumn + extension - 1 will fit in the tailgaps
  *
  * @param msaColumn [int]
  * @param row [int]
  * @param extension [BioString]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::extendRight(int msaColumn, int row, const BioString &extension)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT(extension.isEmpty() == false);
    ASSERT(extension.hasNonGaps());
    ASSERT_X(msaColumn >= length() - at(row)->tailGaps() + 1, "msaColumn must start within the tail gaps");
    ASSERT_X(msaColumn + extension.length() - 1 <= length(), "extension does not fit in tail gaps");

    subseqFromRow(row)->extendRight(msaColumn, extension);

    return SubseqChangePod(row,
                           ClosedIntRange(msaColumn, msaColumn + extension.length() - 1),
                           SubseqChangePod::eExtendRight,
                           extension);
}

/**
  * @returns Grammar
  */
Grammar Msa::grammar() const
{
    return grammar_;
}

/**
  * @param abstractSeq [AbstractSeq *]
  * @returns int
  */
int Msa::indexOfAbstractSeq(const AbstractSeqSPtr &abstractSeq) const
{
    if (abstractSeq)
    {
        QVector<Subseq *>::ConstIterator it = subseqs_.constBegin();
        for (; it != subseqs_.constEnd(); ++it)
            if ((*it)->seqEntity_ == abstractSeq)
                return it - subseqs_.constBegin() + 1;
    }

    return 0;
}

/**
  * To successfully insert the subseq, it must be of equal length to all the other sequences in the Msa. This operation
  * is 1-based.
  *
  * @param i [int]
  * @param subseq [Subseq *]
  * @return bool
  */
bool Msa::insert(int i, Subseq *subseq)
{
    ASSERT_X(i >= 1 && i <= subseqCount() + 1, "index out of range");
    if (!isCompatibleSubseq(subseq))
        return false;

    subseqs_.insert(i-1, subseq);

    return true;
}

/**
  * Column must reference a valid index which includes the alignment length + 1 for the addition of gap columns at the
  * rightmost side of the alignment.
  *
  * @param column [int]
  * @param count [int]
  * @param gapCharacter [char]
  * @see removeGapColumns()
  */
void Msa::insertGapColumns(int column, int count, char gapCharacter)
{
    ASSERT_X(count >= 0, "count must be greater or equal to zero");
    if (count == 0)
        return;

    // Alignment must have at least one sequence to insert gap columns
    ASSERT_X(isEmpty() == false, "At least one sequence is required");

    // Step through each subseq and add the gaps
    for (int i=0, z=rowCount(); i<z; ++i)
        subseqs_[i]->insertGaps(column, count, gapCharacter);
}

/**
  * @param row [int]
  * @param subseqs [const QVector<Subseq *>]
  * @returns bool
  */
bool Msa::insertRows(int row, const QVector<Subseq *> subseqs)
{
    ASSERT_X(isValidRow(row) || row == subseqs_.size() + 1, "row out of range");
    if (!isCompatibleSubseqVector(subseqs))
        return false;

    // Expand the vector with null pointers
    subseqs_.insert(row - 1, subseqs.size(), nullptr);

    // Copy subseqs into vector
    memcpy(subseqs_.data() + row - 1, subseqs.constData(), subseqs.size() * sizeof(Subseq *));

    return true;
}

/**
  * A compatible subseq has the following traits:
  * 1) Have the same grammar as the Msa
  * 2) Contain at least one non-gap character
  * 3) Have an equal length to all other sequences
  *
  * @param subseq [const Subseq *]
  * @returns bool
  */
bool Msa::isCompatibleSubseq(const Subseq *subseq) const
{
    ASSERT_X(subseq != nullptr, "subseq pointer must not be null");

    return subseq->grammar() == grammar_ &&
           subseq->hasNonGaps() &&
           (subseqCount() == 0 || subseq->length() == length());
}

/**
  * Convenience method for testing if all subseqs are compatible.
  *
  * @param subseq [const QVector<Subseq *>]
  * @returns bool
  * @see isCompatibleSubseq()
  */
bool Msa::isCompatibleSubseqVector(const QVector<Subseq *> subseqs) const
{
    foreach (const Subseq *subseq, subseqs)
        if (!isCompatibleSubseq(subseq))
            return false;

    return true;
}

/**
  * @returns bool
  */
bool Msa::isEmpty() const
{
    return subseqs_.isEmpty();
}

/**
  * @param column [int]
  * @returns bool
  */
bool Msa::isValidColumn(int column) const
{
    return column > 0 && column <= columnCount();
}

/**
  * @param msaRect [const PosiRect &]
  * @returns bool
  */
bool Msa::isValidRect(const PosiRect &msaRect) const
{
    return isValidColumn(msaRect.left()) &&
           isValidColumn(msaRect.right()) &&
           isValidRow(msaRect.top()) &&
           isValidRow(msaRect.bottom());
}

/**
  * @param row [int]
  * @returns bool
  */
bool Msa::isValidRow(int row) const
{
    return row > 0 && row <= rowCount();
}

/**
  * @param rows [const ClosedIntRange &]
  * @returns bool
  */
bool Msa::isValidRowRange(const ClosedIntRange &rows) const
{
    return isValidRow(rows.begin_) &&
            rows.begin_ <= rows.end_ &&
            isValidRow(rows.end_);
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @returns int
  */
int Msa::leftExtendableLength(int msaColumn, int row) const
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");

    Subseq *subseq = subseqFromRow(row);
    int nFillableGaps = subseq->headGaps() - msaColumn + 1;
    if (nFillableGaps < 1)
        return 0;

    int oldStart = subseq->start();
    int newStart = qMax(1, oldStart - nFillableGaps);
    return oldStart - newStart;
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @returns int
  */
int Msa::leftTrimmableLength(int msaColumn, int row) const
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");

    Subseq *subseq = subseqFromRow(row);
    int nHeadGaps = subseq->headGaps();
    if (msaColumn <= nHeadGaps)
        return 0;

    int nTrimmableChars = subseq->nonGapsBetween(ClosedIntRange(nHeadGaps + 1, msaColumn));
    if (subseq->ungappedLength() - nTrimmableChars < 1)
        --nTrimmableChars;

    return nTrimmableChars;
}

/**
  * @returns int
  */
int Msa::length() const
{
    return (subseqs_.size() > 0) ? subseqs_.at(0)->length() : 0;
}

/**
  * @param msaColumn [int]
  * @param rows [ClosedIntRange]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> Msa::levelLeft(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    if (msaColumn > 1)
    {
        // Combination of a trim and extend
        QVector<SubseqChangePod> pods;
        pods.reserve(rows.length() * 2);
        pods = trimLeft(msaColumn - 1, rows);
        pods << extendLeft(msaColumn, rows);
        pods.squeeze();

        return pods;
    }

    // Special case: msaColumn == 1
    return extendLeft(msaColumn, rows);
}

/**
  * @param msaColumn [int]
  * @param rows [ClosedIntRange]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> Msa::levelRight(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    if (msaColumn < length())
    {
        // Otherwise: this process is the combination of a trim and extend
        QVector<SubseqChangePod> pods;
        pods.reserve(rows.length() * 2);
        pods = trimRight(msaColumn + 1, rows);
        pods << extendRight(msaColumn, rows);
        pods.squeeze();

        return pods;
    }

    // Special case: msaColumn == length()
    return extendRight(msaColumn, rows);
}

/**
  * @returns QVector<const Subseq *>
  */
QVector<const Subseq *> Msa::members() const
{
    QVector<const Subseq *> constSubseqs;
    constSubseqs.reserve(subseqs_.size());
    for (int i=0, z=subseqs_.size(); i<z; ++i)
        constSubseqs << subseqs_.at(i);
    return constSubseqs;
}

/**
  * Simply call QVector::move() after trnaslating from and to to 0-based indices.
  *
  * @param from [int]
  * @param to [int]
  * @see moveRowRelative(), moveRowRange(), moveRowRangeRelative()
  */
void Msa::moveRow(int from, int to)
{
    // Do nothing if they are the same values
    if (from == to)
        return;

    Subseq *fromSubseq = subseqs_.at(from - 1);
    subseqs_.remove(from - 1);
    subseqs_.insert(to - 1, fromSubseq);
}

/**
  * Simultaneously moves the sequences in rows to the position to.
  *
  * @param rows [const ClosedIntRange &]
  * @param to [int]
  */
void Msa::moveRowRange(const ClosedIntRange &rows, int to)
{
    if (to == rows.begin_)
        return;

    QVector<Subseq *> subseqs = subseqs_.mid(rows.begin_ - 1, rows.length());
    subseqs_.remove(rows.begin_ - 1, rows.length());
    subseqs_.insert(to - 1, rows.length(), nullptr);

    Subseq **dst = subseqs_.data() + to - 1;
    ::memcpy(dst, subseqs.constData(), rows.length() * sizeof(Subseq *));
}

/**
  * Functions similar to moveRow(), except that the subseqs in rows are moved delta positions relative to their current
  * location.
  *
  * @param rows [const ClosedIntRange &]
  * @param delta [int]
  */
void Msa::moveRowRangeRelative(const ClosedIntRange &rows, int delta)
{
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    moveRowRange(rows, rows.begin_ + delta);

    /*
    if (delta == 0)
        return 0;

    int actual_delta = 0;
    int to = 0;
    if (delta < 0)  // Move up
    {
        if (rows.begin_ > 1)
        {
            actual_delta = qMin(rows.begin_ - 1, qAbs(delta));
            to = rows.begin_ - actual_delta;

            actual_delta = -actual_delta;
        }
    }
    else
    {
        int nSubseqs = subseqCount();
        if (rows.end_ < nSubseqs)
        {
            actual_delta = qMin(nSubseqs - rows.end_, delta);
            to = rows.begin_ + actual_delta;
        }
    }

    if (actual_delta)
        moveRowRange(rows, to);

    return actual_delta;
    */
}

/**
  * @param from [int]
  * @param delta [int]
  */
void Msa::moveRowRelative(int from, int delta)
{
    ASSERT_X(from >= 1 && from <= subseqCount(), "from out of range");

    moveRow(from, from + delta);
    /*
    if (delta == 0)
        return 0;

    // Undefined state of to = 0
    int to = 0;
    int actual_delta = 0;

    if (delta < 0)  // Move up
    {
        if (from > 1)
        {
            actual_delta = qMin(from - 1, qAbs(delta));
            to = from - actual_delta;

            actual_delta = -actual_delta;
        }
    }
    else // (delta > 0) Move down
    {
        int nSubseqs = subseqCount();
        if (from < nSubseqs)
        {
            actual_delta = qMin(nSubseqs - from, delta);
            to = from + actual_delta;
        }
    }

    // No possible change, simply return
    if (actual_delta)
    {
        moveRow(from, to);
    }

    return actual_delta;
    */
}

/**
  * To successfully add new sequences to a Msa, the new sequence must be of equal length to the other sequences in the
  * Msa and contain only valid characters (which is implicitly the case because subseq is based on an AnonSeq which
  * masks all invalid characters.
  *
  * @param subseq [Subseq *]
  * @return bool
  */
bool Msa::prepend(Subseq *subseq)
{
    if (!isCompatibleSubseq(subseq))
        return false;

    subseqs_.prepend(subseq);
    return true;
}

/**
  * Simply call QVector::remove().
  *
  * @param i [int]
  * @see removeFirst(), removeLast(), clear()
  */
void Msa::removeAt(int i)
{
    delete subseqs_.at(i-1);
    subseqs_.remove(i-1);
}

/**
  * Simply call QVector::pop_front()
  */
void Msa::removeFirst()
{
    delete subseqs_.at(0);
    subseqs_.pop_front();
}

/**
  * @param rows [const ClosedIntRange &]
  */
void Msa::removeRows(const ClosedIntRange &rows)
{
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    for (int i=rows.begin_; i<= rows.end_; ++i)
        delete subseqs_.at(i-1);

    subseqs_.remove(rows.begin_-1, rows.length());
}

/**
  * Remove those columns that consist entirely of gaps regardless of the gap character. Returns a vector of the ranges
  * that were removed.
  *
  * @return QVector<ClosedIntRange>
  * @see insertGapColumns()
  */
QVector<ClosedIntRange> Msa::removeGapColumns()
{
    if (isEmpty())
        return QVector<ClosedIntRange>();

    return removeGapColumns(ClosedIntRange(1, columnCount()));
}

/**
  * @param columnRange [const ClosedIntRange &]
  * @returns QVector<ClosedIntRange>
  */
QVector<ClosedIntRange> Msa::removeGapColumns(const ClosedIntRange &columnRange)
{
    QVector<ClosedIntRange> contiguousGapRanges = findGapColumns_iteratorRowBased(columnRange);

    for (int i=contiguousGapRanges.count()-1; i >= 0; --i)
    {
        int first = contiguousGapRanges.at(i).begin_;
        int contiguousGapSize = contiguousGapRanges.at(i).end_ - first + 1;
        for (int j=0, z= rowCount(); j<z; ++j)
            subseqs_.at(j)->removeGaps(first, contiguousGapSize);
    }

    return contiguousGapRanges;
}

/**
  * Simply call QVector::pop_back()
  */
void Msa::removeLast()
{
    delete subseqs_.last();
    subseqs_.pop_back();
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @returns int
  */
int Msa::rightExtendableLength(int msaColumn, int row) const
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");

    Subseq *subseq = subseqFromRow(row);
    int nFillableGaps = msaColumn - (length() - subseq->tailGaps());
    if (nFillableGaps < 1)
        return 0;

    int oldStop = subseq->stop();
    int newStop = qMin(subseq->parentSeq_.length(), oldStop + nFillableGaps);
    return newStop - oldStop;
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @returns int
  */
int Msa::rightTrimmableLength(int msaColumn, int row) const
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRow(row), "row out of range");

    Subseq *subseq = subseqFromRow(row);
    int firstTailGap = length() - subseq->tailGaps() + 1;
    if (msaColumn >= firstTailGap)
        return 0;

    int nTrimmableChars = subseq->nonGapsBetween(ClosedIntRange(msaColumn, firstTailGap - 1));

    // Prevent trim operations from removing all characters so we reduce the number of trimmable characters by one
    if (subseq->ungappedLength() - nTrimmableChars < 1)
        --nTrimmableChars;

    return nTrimmableChars;
}

/**
  * @returns int
  * @see subseqCount()
  */
int Msa::rowCount() const
{
    return subseqs_.size();
}

/**
  * Changes are simply extensions (replacing gaps with characters) or trims (replacing characters with gaps).
  * For example,
  * ABC-- >>> --C-- (trim)
  * --C-- >>> ABC-- (extension)
  *
  * Setting the start position beyond the current stop position is not allowed.
  *
  * @param row [int]
  * @param newStart [int]
  * @returns SubseqChangePod
  * @see setSubseqStop()
  */
SubseqChangePod Msa::setSubseqStart(int row, int newStart)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(newStart <= subseqFromRow(row)->stop(), "Moving start beyond current stop is not permitted from Msa");

    Subseq *subseq = subseqFromRow(row);
    if (newStart < subseq->start())
        return extendLeft(row, subseq->start() - newStart);
    else if (newStart > subseq->start())
        return trimLeft(row, newStart - subseq->start());

    return SubseqChangePod();
}

/**
  * Changes are simply extensions (replacing gaps with characters) or trims (replacing characters with gaps).
  * For example,
  * --CDE >>> --C-- (trim)
  * --C-- >>> --CDE (extension)
  *
  * Setting the stop position before the current start position is not allowed.
  *
  * @param row [int]
  * @param newStop [int]
  * @returns SubseqChangePod
  * @see setSubseqStart()
  */
SubseqChangePod Msa::setSubseqStop(int row, int newStop)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(newStop >= subseqFromRow(row)->start(), "Moving stop before the current start is not permitted from Msa");

    Subseq *subseq = subseqFromRow(row);
    if (newStop < subseq->stop())
        return trimRight(row, subseq->stop() - newStop);
    else if (newStop > subseq->stop())
        return extendRight(row, newStop - subseq->stop());

    return SubseqChangePod();
}

/**
  * Core editing routine for manipulating the characters within a Msa. In essence, all editing operations boil down
  * to horizontally sliding a block of characters within the limits of the alignment. Left/top+ right/bottom denotes a
  * rectangular set of coordinates which may be in any order and positive or negative given that each point is within
  * the Msa boundaries. delta may not be zero and is the number of positions to slide left (negative delta) or right
  * (positive delta).
  *
  * If the rectangular region contains non-gap characters, will horizontally slide the characters until the region is
  * immediately adjacent to other non-gap characters at any point along the vertical edge of region. On the other hand,
  * if region is solely comprised of gap characters, this may be moved to any extent up to the alignment bounds.
  *
  * The delta emitted corresponds to the actual delta and not the requested delta.
  *
  * @param left [int]
  * @param top [int]
  * @param right [int]
  * @param bottom [int]
  * @param delta [int]
  * @return int - the actual direction and number of positions slid
  */
int Msa::slideRect(const PosiRect &msaRect, int delta)
{
    ASSERT_X(isEmpty() == false, "There are no sequences in the Msa");
    if (delta == 0)
        return 0;

    ASSERT_X(msaRect.isValid(), "msaRect is not valid");

    PosiRect normalizedRect = msaRect.normalized();

#ifdef QT_DEBUG
    int left = normalizedRect.left();
    int right = normalizedRect.right();
    ASSERT_X(left <= length(), "left out of range");
    ASSERT_X(right <= length(), "right out of range");
#endif

    int top = normalizedRect.top();
    int bottom = normalizedRect.bottom();
    ASSERT_X(top <= subseqCount(), "top out of range");
    ASSERT_X(bottom <= subseqCount(), "bottom out of range");

    ClosedIntRange sourceHorizontalRange = normalizedRect.horizontalRange();

    int actualDelta = 0;
    if (delta < 0)  // Slide to the left
    {
        actualDelta = subseqs_[top-1]->leftSlidablePositions(sourceHorizontalRange);
        for (int i=top+1; actualDelta > 0 && i<= bottom; ++i)
        {
            int tmp = subseqs_[i-1]->leftSlidablePositions(sourceHorizontalRange);
            if (tmp < actualDelta)
                actualDelta = tmp;
        }

        // Negate actualDelta because we are sliding to the left
        actualDelta = -actualDelta;

        // If actualDelta can slide more spots than requested, limit to the number of spots requested
        if (actualDelta < delta)
            actualDelta = delta;
    }
    else    // (delta > 0) slide to the right
    {
        actualDelta = subseqs_[top-1]->rightSlidablePositions(sourceHorizontalRange);
        for (int i=top+1; actualDelta > 0 && i<= bottom; ++i)
        {
            int tmp = subseqs_[i-1]->rightSlidablePositions(sourceHorizontalRange);
            if (tmp < actualDelta)
                actualDelta = tmp;
        }

        // If actualDelta can slide more spots than requested, limit to the number of spots requested
        if (actualDelta > delta)
            actualDelta = delta;
    }

    if (actualDelta)
        for (int i=top-1; i< bottom; ++i)
            subseqs_[i]->slide(sourceHorizontalRange, actualDelta);

    return actualDelta;
}

/**
  * @param lessThan [bool (*)(const Subseq *, const Subseq *)]
  */
void Msa::sort(bool (*lessThan)(const Subseq *a, const Subseq *b))
{
    ASSERT(lessThan != nullptr);

    qStableSort(subseqs_.begin(), subseqs_.end(), lessThan);
}

/**
  * @param subseqLessThan [const ISubseqLessThan &]
  * @param sortOrder [Qt::SortOrder]
  */
void Msa::sort(const ISubseqLessThan &subseqLessThan, Qt::SortOrder sortOrder)
{
    if (sortOrder == Qt::AscendingOrder)
        qStableSort(subseqs_.begin(), subseqs_.end(), SubseqLessThanHelperPrivate(&subseqLessThan));
    else
        qStableSort(subseqs_.begin(), subseqs_.end(), SubseqGreaterThanHelperPrivate(&subseqLessThan));
}

/**
  * @returns int
  * @see rowCount()
  */
int Msa::subseqCount() const
{
    return subseqs_.size();
}

/**
  * Simply swaps the subseqs at positions i and j (1-based).
  *
  * @param i [int]
  * @param j [int]
  */
void Msa::swap(int i, int j)
{
    if (i == j)
        return;

    Subseq *tmp = subseqs_.at(i-1);
    subseqs_[i-1] = subseqs_[j-1];
    subseqs_[j-1] = tmp;
}

/**
  * @param rows [const ClosedIntRange &]
  */
QVector<Subseq *> Msa::takeRows(const ClosedIntRange &rows)
{
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    QVector<Subseq *> extraction = subseqs_.mid(rows.begin_ - 1, rows.length());
    subseqs_.remove(rows.begin_-1, rows.length());
    return extraction;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> Msa::trimLeft(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(rows.length());
    for (int i=rows.begin_; i<= rows.end_; ++i)
    {
        Subseq *subseq = subseqFromRow(i);
        int nHeadGaps = subseq->headGaps();
        if (msaColumn <= nHeadGaps)
            continue;

        ClosedIntRange trimRange(nHeadGaps + 1, msaColumn);
        int nTrimmableChars = subseq->nonGapsBetween(trimRange);

        // Prevent trim operations from removing all characters so we reduce the number of trimmable characters by one
        // Additionally, adjust the trim range at the same time
        if (subseq->ungappedLength() - nTrimmableChars < 1)
        {
            const char *x = subseq->constData() + trimRange.end_ - 1;   // - 1 to map to 0-based indices
            forever
            {
                --trimRange.end_;
                if (!::isGapCharacter(*x))
                    break;

                --x;
            }
            --nTrimmableChars;
        }
        if (nTrimmableChars == 0)
            continue;

        // Could simply call the trimLeft(i, nCharsToTrim) method; however, that would require looping through the
        // sequence again looking for the exact range to trim. This would duplicate effort, since we already know
        // the exact range to trim. The only complication is that the trimRange.end_ might have trailing gap characters.
        // Thus, we remove these in this loop.
        const char *x = subseq->constData() + trimRange.end_ - 1;
        while (::isGapCharacter(*x))
        {
            --x;
            --trimRange.end_;
        }

        BioString difference = subseq->mid(trimRange);
        subseq->trimLeft(trimRange, nTrimmableChars);
        pods << SubseqChangePod(i,
                                trimRange,
                                SubseqChangePod::eTrimLeft,
                                difference);
    }

    return pods;
}

/**
  * At least one non-gap character must remain after the trim operation.
  *
  * Why not simply have this functionaliy in Subseq?
  * 1) Because it would have to return the relevant changes in some new custom structure
  * 2) Given #1, capture the difference *before* the trim operation takes place
  * 3) the trimLeft operation is actually quite fast because it simply utilizes memcpy with a gap buffer
  *
  * @param row [int]
  * @param nCharsToRemove [int]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::trimLeft(int row, int nCharsToRemove)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(nCharsToRemove > 0, "nChars must be positive");
    ASSERT_X(subseqFromRow(row)->ungappedLength() > nCharsToRemove, "Removing all non-gap characters is not allowed");

    Subseq *subseq = subseqFromRow(row);
    int nHeadGaps = subseq->headGaps();
    ClosedIntRange range(nHeadGaps + 1, nHeadGaps + 1);
    int charsFound = 1;

    const char *x = subseq->constData() + range.begin_;
    while (charsFound != nCharsToRemove)
    {
        if (!::isGapCharacter(*x))
            ++charsFound;

        ++range.end_;
        ++x;
    }

    BioString difference = subseq->mid(range);
    subseq->trimLeft(range, nCharsToRemove);
    return SubseqChangePod(row,
                           range,
                           SubseqChangePod::eTrimLeft,
                           difference);
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> Msa::trimRight(int msaColumn, const ClosedIntRange &rows)
{
    ASSERT_X(isValidColumn(msaColumn), "msaColumn out of range");
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    QVector<SubseqChangePod> pods;
    pods.reserve(rows.length());
    for (int i=rows.begin_; i<= rows.end_; ++i)
    {
        Subseq *subseq = subseqFromRow(i);
        int firstTailGap = length() - subseq->tailGaps() + 1;
        if (msaColumn >= firstTailGap)
            continue;

        ClosedIntRange trimRange(msaColumn, firstTailGap - 1);
        int nTrimmableChars = subseq->nonGapsBetween(trimRange);

        // Prevent trim operations from removing all characters so we reduce the number of trimmable characters by one
        // Additionally, adjust the trim range at the same time
        if (subseq->ungappedLength() - nTrimmableChars < 1)
        {
            const char *x = subseq->constData() + trimRange.begin_ - 1; // - 1 to map to 0-based indices
            forever
            {
                ++trimRange.begin_;
                if (!::isGapCharacter(*x))
                    break;

                ++x;
            }
            --nTrimmableChars;
        }
        if (nTrimmableChars == 0)
            continue;

        // Could simply call the trimRight(i, nCharsToTrim) method; however, that would require looping through the
        // sequence again looking for the exact range to trim. This would duplicate effort, since we already know
        // the exact range to trim. The only complication is that the trimRange.begin_ might have leading gap characters
        // Thus, we remove these in this loop.
        const char *x = subseq->constData() + trimRange.begin_ - 1;
        while (::isGapCharacter(*x))
        {
            ++x;
            ++trimRange.begin_;
        }

        BioString difference = subseq->mid(trimRange);
        subseq->trimRight(trimRange, nTrimmableChars);
        pods << SubseqChangePod(i,
                                trimRange,
                                SubseqChangePod::eTrimRight,
                                difference);
    }

    return pods;
}

/**
  * At least one non-gap character must remain after the trim operation.
  *
  * Why not simply have this functionaliy in Subseq?
  * 1) Because it would have to return the relevant changes in some new custom structure
  * 2) Given #1, capture the difference *before* the trim operation takes place
  * 3) the trimRight operation is actually quite fast because it simply utilizes memcpy with a gap buffer
  *
  * @param row [int]
  * @param nCharsToRemove [int]
  * @returns SubseqChangePod
  */
SubseqChangePod Msa::trimRight(int row, int nCharsToRemove)
{
    ASSERT_X(isValidRow(row), "row out of range");
    ASSERT_X(nCharsToRemove > 0, "nChars must be positive");
    ASSERT_X(subseqFromRow(row)->ungappedLength() > nCharsToRemove, "Removing all non-gap characters is not allowed");

    Subseq *subseq = subseqFromRow(row);
    int nTailGaps = subseq->tailGaps();
    ClosedIntRange range(length() - nTailGaps, length() - nTailGaps);
    int charsFound = 1;

    const char *x = subseq->constData() + range.end_ - 2;
    while (charsFound != nCharsToRemove)
    {
        if (!::isGapCharacter(*x))
            ++charsFound;

        --range.begin_;
        --x;
    }

    BioString difference = subseq->mid(range);
    subseq->trimRight(range, nCharsToRemove);
    return SubseqChangePod(row,
                           range,
                           SubseqChangePod::eTrimRight,
                           difference);
}

/**
  * If the subseqChangePodVector only contains one change per subseq, then the order in which the changes are undone
  * would not matter; however, sometimes there will be multiple changes for a given subseq (e.g. from levelLeft or
  * levelRight commands) in which their order of application is significant. Therefore, it is vital to treat this
  * vector of changes like a stack and undo them in the reverse order.
  *
  * @param subseqChangePodVector [const SubseqChangePodVector &]
  * @returns SubseqChangePodVector
  */
SubseqChangePodVector Msa::undo(const SubseqChangePodVector &subseqChangePodVector)
{
    SubseqChangePodVector undoneChangePods;
    undoneChangePods.reserve(subseqChangePodVector.size());
    for (int i=subseqChangePodVector.size()-1; i >= 0; --i)
    {
        const SubseqChangePod &pod = subseqChangePodVector.at(i);
        if (pod.isNull())
            continue;

        // This is the operation that we are "undo'ing"
        switch(pod.operation_)
        {
        case SubseqChangePod::eExtendLeft:
            subseqFromRow(pod.row_)->trimLeft(pod.columns_);
            undoneChangePods << pod;
            undoneChangePods.last().operation_ = SubseqChangePod::eTrimLeft;
            break;
        case SubseqChangePod::eExtendRight:
            subseqFromRow(pod.row_)->trimRight(pod.columns_);
            undoneChangePods << pod;
            undoneChangePods.last().operation_ = SubseqChangePod::eTrimRight;
            break;
        case SubseqChangePod::eTrimLeft:
            undoneChangePods << extendLeft(pod.columns_.begin_, pod.row_, pod.difference_);
            break;
        case SubseqChangePod::eTrimRight:
            undoneChangePods << extendRight(pod.columns_.begin_, pod.row_, pod.difference_);
            break;
        case SubseqChangePod::eInternal:
            {
                Subseq *subseq = subseqFromRow(pod.row_);
                undoneChangePods << pod;
                BioString old = subseq->mid(pod.columns_);
                subseq->rearrange(pod.columns_, undoneChangePods.last().difference_);
                undoneChangePods.last().difference_ = old;
            }
            break;

        default:
            ASSERT_X(0, "Unimplemented switch condition");
            break;
        }
    }

    return undoneChangePods;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Iterator, row-based strategy for finding columns containing purely gaps. The approach follows the given rules (X
  * indicates columns currently recognized as all-gaps, and * indicates a column that has just been identified with a
  * non-gap character):
  *
  * ___X___ | X___ | ___X
  *    *      *         *
  * Result: remove this gap range
  *
  * __XXX__
  *    *
  * Result: split the one gap range (3 -> 5) into two separate gap ranges: (3 -> 3) and (5 -> 5)
  *
  * __XX__ | __XX__
  *   *         *
  * Result: shrink the range by increasing the beginning by one or decreasing the end by one, respectively.
  *
  * The above should capture all possible situations and the algorithm begins with one gap range that spans the entire
  * alignment length. This one gap range may be reduced to nothing as non-gap characters are located.
  *
  * By iterating over the rows instead of the columns, we more efficiently access the memory which should result in
  * increased performance because the sequence will likely remain in the cache. In contrast, moving column by column
  * requires jumping to a new memory location with each character lookup.
  *
  * @returns QVector<ClosedIntRange>
  */
QVector<ClosedIntRange> Msa::findGapColumns_iteratorRowBased(const ClosedIntRange &columnRange) const
{
    ASSERT(columnRange.begin_ >= 0 && columnRange.begin_ <= columnRange.end_);
    ASSERT(columnRange.end_ <= columnCount());

    int rows = rowCount();
    QVector<ClosedIntRange> contiguousGapRanges;
    contiguousGapRanges << columnRange;
    for (int i=0; i< rows; ++i)
    {
        Subseq *subseq = subseqs_.at(i);

        QVector<ClosedIntRange>::Iterator it = contiguousGapRanges.begin();
        while (it != contiguousGapRanges.end())
        {
            bool erasedGapRange = false;
            const char *x = subseq->constData() + it->begin_ - 1;
            for (int j=it->begin_; j<=it->end_; ++j, ++x)
            {
                if (!::isGapCharacter(*x))
                {
                    // Case A: We found a non-gap character at the very beginning of this range
                    if (j == it->begin_)
                    {
                        ++it->begin_;
                        if (it->begin_ > it->end_)
                        {
                            it = contiguousGapRanges.erase(it);
                            erasedGapRange = true;
                            break;
                        }
                    }
                    // Case B: Non-gap character at the end of the range
                    else if (j == it->end_)
                    {
                        --it->end_;
                        ASSERT(it->end_ >= it->begin_);
                    }
                    // Case C: Non-gap character in the middle of the range => split this group
                    else
                    {
                        // !! Note: QVector::insert inserts *before* the current iterator position
                        it = contiguousGapRanges.insert(it, ClosedIntRange(it->begin_, j - 1));
                        ++it;
                        it->begin_ = j + 1;
                    }
                }
            }

            if (!erasedGapRange)
                ++it;
        }
    }

    return contiguousGapRanges;
}

/**
  * Uses for loops and direct character access column-by-column to find all gap columns. Preserved here for historical
  * sake and later benchmarking should that be desired. This method was replaced by a presumably superior iterator, row-
  * based version, findGapColumns_iteratorRowBased()
  *
  * @returns QVector<ClosedIntRange>
  * @see findGapColumns_iteratorRowBased()
  */
QVector<ClosedIntRange> Msa::findGapColumns_nonIteratorColumnBased() const
{
    if (isEmpty())
        return QVector<ClosedIntRange>();

    int columns = columnCount();

    QVector<ClosedIntRange> contiguousGapRanges;
    for (int i=1, z=length(); i <= z; ++i)
    {
        bool all_gaps = true;
        for (int j=0; j< columns; ++j)
        {
            if (::isGapCharacter(subseqs_.at(j)->at(i)) == false)
            {
                all_gaps = false;
                break;
            }
        }

        if (!all_gaps)
            continue;

        if (contiguousGapRanges.isEmpty() || contiguousGapRanges.last().end_ != i-1)
            contiguousGapRanges << ClosedIntRange(i, i);
        else // This gap column is contiguous with the previous all gap column. Increment the last all gap range column.
            ++contiguousGapRanges.last().end_;
    }

    return contiguousGapRanges;
}




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Defunct methods
/**
  * @param row [int]
  * @param newStart [int]
  * @returns SubseqChangePod
  */
/*
SubseqChangePod Msa::setSubseqStartHelper(int row, int newStart)
{
    ASSERT_X(row > 0 && row <= subseqCount(), "row out of range");
    ASSERT_X(newStart > 0 && newStart <= subseqs_.at(row-1)->parentSeq_.length(), "newStart out of range");
    ASSERT_X(newStart > 0, "newStart out of range");

    // Do nothing if the start is not different
    Subseq *subseq = subseqs_.at(row - 1);
    int oldStart = subseq->start();
    if (newStart == oldStart)
        return SubseqChangePod();

    // Two cases:
    // 1) newStart < start :: adding characters
    if (newStart < oldStart)
    {
        int nNewChars = oldStart - newStart;
        int nHeadGaps = subseq->headGaps();

        // Insert any new gap columns as needed to accommodate the new sequence characters
        int nNewGapColumns = qMax(0, nNewChars - nHeadGaps);
        insertGapColumns(1, nNewGapColumns);
        nHeadGaps += nNewGapColumns;

        // Update the start position
        subseq->setStart(newStart);

        int msaColumn = nHeadGaps - nNewChars + 1;
        return SubseqChangePod(row,
                               ClosedIntRange(msaColumn, msaColumn + nNewChars - 1),
                               SubseqChangePod::eExtendLeft,
                               subseq->mid(msaColumn, nNewChars));
    }
    // 2) newStart > start :: removing characters
    //    More complicated case because this operation may also involve moving the stop coordinate
    else
    {
        if (newStart <= subseq->stop())
        {
            int nCharsToRemove = newStart - oldStart;
            int nHeadGaps = subseq->headGaps();
            int startMsaColumn = nHeadGaps + 1;

            int charsFound = 1;
            int endMsaColumn = startMsaColumn;

            const char *x = subseq->constData();
            x += startMsaColumn;
            while (charsFound != nCharsToRemove) // ASSERT && *x != '\0')
            {
                if (!::isGapCharacter(*x))
                    ++charsFound;

                ++endMsaColumn;
                ++x;
            }

            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            BioString difference = subseq->mid(startMsaColumn, nMsaColumns);

            subseq->setStart(newStart);
            return SubseqChangePod(row,
                                   ClosedIntRange(startMsaColumn, endMsaColumn),
                                   SubseqChangePod::eTrimLeft,
                                   difference);
        }
        // newStart > subseq->stop()
        else
        {
            // Remember this is at least a two-step process:
            // 1) Extend the sequence by increasing the stop
            // 2) Trim the sequence by inreasing the start to the stop

            // Save the old stop position and old alignment
            int oldStop = subseq->stop();
            int newStop = newStart;     // Because the newStart is larger than the old stop
            int nNewChars = newStop - oldStop;

            // Insert any new gap columns (for entire Msa) as needed to accommodate the new sequence characters
            int nTailGaps = subseq->tailGaps();
            int nNewGapColumns = qMax(0, nNewChars - nTailGaps);
            insertGapColumns(length() + 1, nNewGapColumns);
            nTailGaps += nNewGapColumns;

            int trimStartColumn = subseq->headGaps() + 1;   // Important that we know the number of headgaps *before*
                                                            // updating the start position and gather alignment data
            int extendStartColumn = length() - nTailGaps + 1; // 1-based

            BioString difference = subseq->mid(ClosedIntRange(trimStartColumn, extendStartColumn - 1));
            subseq->setStart(newStart);

            return SubseqChangePod(row,
                                   ClosedIntRange(trimStartColumn, extendStartColumn + nNewChars - 1),
                                   SubseqChangePod::eExtendRightTrimLeft,
                                   difference);
        }
    }

    ASSERT_X(0, "Impossible location to reach!");
}
*/

/**
  * @param row [int]
  * @param newStart [int]
  * @returns SubseqChangePod
  */
/*
SubseqChangePod Msa::setSubseqStopHelper(int row, int newStop)
{
    ASSERT_X(row > 0 && row <= subseqCount(), "subseqIndex out of range");
    ASSERT_X(newStop > 0 && newStop <= subseqs_.at(row-1)->parentSeq_.length(), "newStop out of range");
    ASSERT_X(newStop > 0, "newStop out of range");

    // Do nothing if the stop is not different
    Subseq *subseq = subseqs_.at(row - 1);
    int oldStop = subseq->stop();
    if (newStop == oldStop)
        return SubseqChangePod();

    int oldLength = length();

    // Two cases:
    // 1) newStop > stop :: adding characters
    if (newStop > oldStop)
    {
        int nNewChars = newStop - oldStop;
        int nTailGaps = subseq->tailGaps();

        // Insert any new gap columns as needed to accommodate the new sequence characters
        insertGapColumns(oldLength + 1, qMax(0, nNewChars - nTailGaps));

        subseq->setStop(newStop);
        int msaColumn = oldLength - nTailGaps + 1;

        return SubseqChangePod(row,
                               ClosedIntRange(msaColumn, msaColumn + nNewChars - 1),
                               SubseqChangePod::eExtendRight,
                               subseq->mid(msaColumn, nNewChars));
    }
    // 2) newStop < stop :: removing characters
    //    More complicated case because this operation may also involve moving the start coordinate
    else
    {
        if (newStop >= subseq->start())
        {
            int nCharsToRemove = oldStop - newStop;
            int nTailGaps = subseq->tailGaps();
            int endMsaColumn = oldLength - nTailGaps;

            int charsFound = 1;                 // We can assume that the very last character is already found
            int startMsaColumn = endMsaColumn;

            const char *x = subseq->constData() + endMsaColumn - 2;   // Minus 2 = -1 because going from 1-based to
                                                                      // zero based and -1 because we are ignoring
                                                                      // character already found
            while (charsFound != nCharsToRemove)
            {
                if (!::isGapCharacter(*x))
                    ++charsFound;

                --startMsaColumn;
                --x;
            }

            // Save the old alignment
            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            BioString difference = subseq->mid(startMsaColumn, nMsaColumns);

            subseq->setStop(newStop);
            return SubseqChangePod(row,
                                   ClosedIntRange(startMsaColumn, endMsaColumn),
                                   SubseqChangePod::eTrimRight,
                                   difference);
        }
        // newStop < subseq->start()
        else
        {
            // Remember this is at least a two-step process:
            // 1) Extend the sequence by increasing the stop
            // 2) Trim the sequence by inreasing the start to the stop

            // Save the old start position and old alignment
            int oldStart = subseq->start();
            int newStart = newStop;
            int nNewChars = oldStart - newStart;

            // Insert any new gap columns as needed to accommodate the new sequence characters
            int nHeadGaps = subseq->headGaps();
            int nNewGapColumns = qMax(0, subseq->start() - newStop - nHeadGaps);
            insertGapColumns(1, nNewGapColumns);
            nHeadGaps += nNewGapColumns;


            int trimStopColumn = length() - subseq->tailGaps();
            BioString difference = subseq->mid(ClosedIntRange(nHeadGaps + 1, trimStopColumn));
            subseq->setStop(newStop);
            return SubseqChangePod(row,
                                   ClosedIntRange(nHeadGaps - nNewChars + 1, trimStopColumn),
                                   SubseqChangePod::eExtendLeftTrimRight,
                                   difference);
        }
    }

    ASSERT_X(0, "Impossible location to reach!");
}
*/
