/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

/**
  * >>> Special notes <<<
  *
  * [24 March 2011]
  * -----------------------------------------------------------
  * @@ Private and public versions of setSubseqs{Start,Stop} @@
  *
  * -- Private methods of extendSubseqs{Left,Right} and trimSubseqs{Left,Right} were created to effectively handle the
  *    need to properly and timely emit the extendOrTrimFinished signal. This reorganization isrequired because the
  *    levelSubseqs{Left,Right} methods directly call the extend and trim methods, and one call to a level method
  *    should only result in one extendOrTrimFinished signal. Not 1-3 (one for extend, one for trim, one for level).
  *
  * -- A similar issue with the above occurs with the setSubseq{Start,Stop} methods. These methods originally were
  *    called by both external classes and internally from within the trim and extend methods. This is problematic
  *    because when called interally, the multiple setSubseq{Start,Stop} calls generally are part of a single,
  *    related extendOrTrimFinished signal (or possibly not at all). On the other hand, when called externally, each
  *    individual subseq modification comprises the entire extendOrTrimFinished signal.
  *
  *    To deal with these needs, private versions of these methods were created that may be called internally as part
  *    of a larger editing operation. The public versions also call their cognate private method, but afterwards emit
  *    the extendOrTrimFinished signal (if relevant).
  */

#include "Msa.h"

#include <QtCore/QPair>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param alphabet [Alphabet]
  * @param id [int]
  * @param parent [QObject *]
  */
Msa::Msa(Alphabet alphabet, int id, QObject *parent) : QObject(parent), alphabet_(alphabet), id_(id)
{
    name_ = "Undefined";
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * This class takes ownership of all member subseqs and therefore is responsible for releasing their
  * memory allocation.
  */
Msa::~Msa()
{
    clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * For convenience, it is possible to directly access a Subseq (e.g. row) via its index position in the Msa. i must be
  * a valid index position in the subseq list.
  *
  * @param i [int]
  * @returns const Subseq *
  * @see at()
  */
const Subseq *Msa::operator()(int i) const
{
    Q_ASSERT_X(i != 0, "Msa::operator()", "index cannot equal 0");
    i = positiveRowIndex(i);
    Q_ASSERT_X(i >= 1 && i <= subseqCount(), "Msa::operator()", "index out of range");

    return subseqs_[i-1];
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet Msa::alphabet() const
{
    return alphabet_;
}

/**
  * To successfully add new sequences to a Msa, the new sequence must be of equal length to the other sequences in the
  * Msa and contain only valid characters (which is implicitly the case because subseq is based on an AnonSeq which
  * masks all invalid characters. Additionally, subseq must have a non-zero length or an assertion will be thrown.
  *
  * TODO: Alphabet must also be equivalent
  *
  * @param subseq [Subseq *]
  * @return bool
  */
bool Msa::append(Subseq *subseq)
{
    Q_ASSERT_X(subseq, "Msa::append", "subseq pointer must not be null");
    ASSERT(subseq->bioString().length() > 0);

    if (subseq->bioString().alphabet() != alphabet_)
        return false;

    int nSubseqs = subseqCount();
    if (nSubseqs == 0 ||
        subseq->bioString().length() == length())
    {
        emit subseqsAboutToBeInserted(nSubseqs + 1, nSubseqs + 1);
        subseqs_.append(subseq);
        emit subseqsInserted(nSubseqs + 1, nSubseqs + 1);

        return true;
    }

    return false;
}

/**
  * Identical and duplicate to the operator() method. Code reproduced here for performance reasons.
  *
  * @param i [int]
  * @returns const Subseq *
  * @see operator()
  */
const Subseq *Msa::at(int i) const
{
    Q_ASSERT_X(i != 0, "Msa::at()", "index cannot equal 0");
    i = positiveRowIndex(i);
    Q_ASSERT_X(i >= 1 && i <= subseqCount(), "Msa::at", "index out of range");

    return subseqs_[i-1];
}

/**
  * Simply call QList::clear()
  */
void Msa::clear()
{
    qDeleteAll(subseqs_);
    subseqs_.clear();
    emit msaReset();
}

/**
  * Maximally shifts all characters in a given region to the left.
  *
  * Emits: collapsedLeft
  *
  * @param msaRect [const MsaRect &]
  * @see collapseRight()
  */
void Msa::collapseLeft(const MsaRect &msaRect)
{
    MsaRect normalizedMsaRect = msaRect.normalized();

    ASSERT_X(normalizedMsaRect.isValid(), "msaRect is not valid");
    ASSERT_X(normalizedMsaRect.right() <= length(), "msaRect.right out of range");
    ASSERT_X(normalizedMsaRect.bottom() <= subseqCount(), "msaRect.bottom out of range");

    int rightMostModifiedColumn = 0;
    for (int row=normalizedMsaRect.top(), bottom = normalizedMsaRect.bottom(); row <= bottom; ++row)
    {
        QString oldAlignment = subseqs_.at(row-1)->bioString().mid(normalizedMsaRect.left(), normalizedMsaRect.width());
        QPair<int, int> affectedRange = subseqs_.at(row-1)->collapseLeft(normalizedMsaRect.left(), normalizedMsaRect.right());
        if (affectedRange.first != 0)
        {
            // Determine the old and new alignment data
            QString finalAlignment = subseqs_.at(row-1)->bioString().mid(affectedRange.first, affectedRange.second - affectedRange.first + 1);

            emit subseqInternallyChanged(row, affectedRange.first, finalAlignment, oldAlignment.mid(affectedRange.first - normalizedMsaRect.left(), affectedRange.second - affectedRange.first + 1));

            // Capture right most modified column
            if (affectedRange.second > rightMostModifiedColumn)
                rightMostModifiedColumn = affectedRange.second;
        }
    }

    if (rightMostModifiedColumn)
        emit collapsedLeft(normalizedMsaRect, rightMostModifiedColumn);
}

/**
  * Emits: collapsedRight
  *
  * @param msaRect [const MsaRect &]
  * @see collapseLeft()
  */
void Msa::collapseRight(const MsaRect &msaRect)
{
    MsaRect normalizedMsaRect = msaRect.normalized();

    ASSERT_X(normalizedMsaRect.isValid(), "msaRect is not valid");
    ASSERT_X(normalizedMsaRect.right() <= length(), "msaRect.right out of range");
    ASSERT_X(normalizedMsaRect.bottom() <= subseqCount(), "msaRect.bottom out of range");

    int leftMostModifiedColumn = length() + 1;
    for (int row=normalizedMsaRect.top(), bottom = normalizedMsaRect.bottom(); row <= bottom; ++row)
    {
        QString oldAlignment = subseqs_.at(row-1)->bioString().mid(normalizedMsaRect.left(), normalizedMsaRect.width());
        QPair<int, int> affectedRange = subseqs_.at(row-1)->collapseRight(normalizedMsaRect.left(), normalizedMsaRect.right());
        if (affectedRange.first != 0)
        {
            // Determine the old and new alignment data
            QString finalAlignment = subseqs_.at(row-1)->bioString().mid(affectedRange.first, affectedRange.second - affectedRange.first + 1);

            emit subseqInternallyChanged(row, affectedRange.first, finalAlignment, oldAlignment.mid(affectedRange.first - normalizedMsaRect.left(), affectedRange.second - affectedRange.first + 1));

            // Capture left most modified column
            if (affectedRange.first < leftMostModifiedColumn)
                leftMostModifiedColumn = affectedRange.first;
        }
    }

    if (leftMostModifiedColumn < length() + 1)
        emit collapsedRight(normalizedMsaRect, leftMostModifiedColumn);
}

/**
  * @return const QString &
  */
const QString &Msa::description() const
{
    return description_;
}

/**
  * This operation takes place within the current Msa extents - no gap columns need to be inserted for this method to
  * execute properly. This is implied by the fact that msaColumn must reference a valid column.
  *
  * Each subseq may only be extended if:
  * 1) msaColumn
  * 1) Zero or more contiguous gaps occur before msaColumn
  * 2) There is at least one gap
  *
  * Example of how this method works:
  * 123456789
  * --C-DEF--
  * -XY-ZZZ-W
  *
  * msa.extendSubseqsLeft(1, 2, 2):
  * -BC-DEF--
  * -XY-ZZZ-W
  *
  * msa.extendSubseqsLeft(1, 2, 1):
  * ABC-DEF--
  * WXY-ZZZ-W
  *
  * Only can add characters to the alignment by decreasing the start position of individual subseqs. Obviously extending
  * the subseqs is constrained by the actual subseqs length and current position. In other words, if a subseq begins at
  * position 1, it cannot be extended more at its leftmost terminus.
  *
  * Emits: extendOrTrimFinished
  *
  * !! Note: The extendOrTrimFinished signal is only emitted if at least one column was affected. If this method is
  *          called, but nothing changed, this signal is not emitted.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @see setSubseqStart(), extendSubseqsRight()
  */
void Msa::extendSubseqsLeft(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    QPair<int, int> affectedColumns = __extendSubseqsLeft(top, bottom, msaColumn);

    // We know that if at least one column was affected, the second value of affectedColumns will be non-zero
    if (affectedColumns.second != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);
}

/**
  * Performs similarly to extendSubseqsLeft except applies to the right boundary of the alignment.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @see setSubseqStop(), extendSubseqsLeft()
  */
void Msa::extendSubseqsRight(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    QPair<int, int> affectedColumns = __extendSubseqsRight(top, bottom, msaColumn);

    // We know that if at least one column was affected, the second value of affectedColumns will be non-zero
    if (affectedColumns.second != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);
}

/**
  * @returns int
  */
int Msa::id() const
{
    return id_;
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
    Q_ASSERT_X(subseq, "Msa::insert", "subseq pointer must not be null");
    if (subseq->bioString().alphabet() != alphabet_)
        return false;

    i = positiveRowIndex(i);
    if (i == 0 && subseqCount() == 0)
        i = 1;

    Q_ASSERT_X(i >= 1 && i <= subseqCount() + 1, "Msa::insert", "index out of range");

    if (subseqs_.count() == 0 ||
        subseq->bioString().length() == length())
    {
        emit subseqsAboutToBeInserted(i, i);
        subseqs_.insert(i-1, subseq);
        emit subseqsInserted(i, i);

        return true;
    }

    return false;
}

/**
  * Position must reference a valid index using either positive or negative indices. The only exception is that position
  * may also be the alignment length + 1, which permits the addition of gap columns at the rightmost side of the alignment.
  *
  * Signals (only emitted if count is greater than zero):
  * >> gapColumnsAboutToBeInserted(column, count)
  * >> gapColumnsInserted(column, count)
  *
  * The signal variable, column, is 1-based and must be between 1 and length() inclusive.
  *
  * @param column [int]
  * @param count [int]
  * @param gapCharacter [char]
  * @see removeGapColumns()
  */
void Msa::insertGapColumns(int column, int count, char gapCharacter)
{
    Q_ASSERT_X(count >= 0, "Msa::insertGapColumns", "count must be greater or equal to zero");
    if (count == 0)
        return;

    // Alignment must have at least one sequence to insert gap columns
    int nSubseqs = subseqCount();
    Q_ASSERT_X(nSubseqs > 0, "Msa::insertGapColumns", "At least one sequence is required");
    if (nSubseqs == 0)  // Release mode guard
        return;

    // Convert column into a positive column index
    int positiveColumn = positiveColIndex(column);
    Q_ASSERT_X(positiveColumn >= 1 && positiveColumn <= length() + 1, "Msa::insertGapColumns", "column out of range");

    emit gapColumnsAboutToBeInserted(positiveColumn, count);

    // Step through each subseq and add the gaps
    for (int i=0; i< nSubseqs; ++i)
        subseqs_[i]->insertGaps(positiveColumn, count, gapCharacter);

    emit gapColumnsInserted(positiveColumn, count);
}

/**
  * Simply call QList::isEmpty()
  *
  * @return bool
  */
bool Msa::isEmpty() const
{
    return subseqs_.isEmpty();
}

/**
  * @return int
  */
int Msa::length() const
{
    return (subseqs_.count()) ? subseqs_.at(0)->bioString().length() : 0;
}

/**
  * Horizontally leveling an alignment involves performing various trim and extend operations to tweak the subseq
  * characters and minimize the number of gaps at msaColumn. In essence, extend or trim as much as possible to build a
  * uniform edge up to and including msaColumn.
  *
  * An example:
  * 123456789
  * --DEF
  * WXYZ-
  * -LMNO
  * A-BCD
  * ---BC
  *
  * msa.levelSubseqsLeft(1, 3, 2)
  * >>>
  * -CDEF  (add one character)
  * -XYZ-  (remove one character)
  * -LMNO  (do nothing)
  * -ABCD  (shift A over one spot)
  * --ABC  (add as much as possible)
  *
  * Emits: extendOrTrimFinished
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @see levelSubseqsRight()
  */
void Msa::levelSubseqsLeft(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    // Special case
    if (msaColumn == 1)
    {
        extendSubseqsLeft(top, bottom, msaColumn);
        return;
    }

    // Otherwise: this process is the intersection of a trim and extend
    QPair<int, int> trimAffectedColumns = __trimSubseqsLeft(top, bottom, msaColumn - 1);
    QPair<int, int> extendAffectedColumns = __extendSubseqsLeft(top, bottom, msaColumn);
    if (trimAffectedColumns.second != 0 || extendAffectedColumns.second != 0)
    {
        emit extendOrTrimFinished(qMin(trimAffectedColumns.first, extendAffectedColumns.first),
                                  qMax(trimAffectedColumns.second, extendAffectedColumns.second));
    }
}

/**
  * Performs similarly to levelSubseqsLeft except all operations are applied to the right alignment terminus.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @see levelSubseqsLeft()
  */
void Msa::levelSubseqsRight(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    // Special case
    if (msaColumn == length())
    {
        extendSubseqsRight(top, bottom, msaColumn);
        return;
    }

    // Otherwise: this process is the combination of a trim and extend
    QPair<int, int> trimAffectedColumns = __trimSubseqsRight(top, bottom, msaColumn + 1);
    QPair<int, int> extendAffectedColumns = __extendSubseqsRight(top, bottom, msaColumn);
    if (trimAffectedColumns.second != 0 || extendAffectedColumns.second != 0)
    {
        emit extendOrTrimFinished(qMin(trimAffectedColumns.first, extendAffectedColumns.first),
                                  qMax(trimAffectedColumns.second, extendAffectedColumns.second));
    }
}

/**
  * It is vital that none of the Subseq members is deleted or the program will crash when the Msa is
  * deallocated.
  *
  * @returns QList<const Subseq *>
  */
QList<const Subseq *> Msa::members() const
{
    QList<const Subseq *> constSubseqs;
    for (int i=0, z=subseqs_.size(); i<z; ++i)
        constSubseqs << subseqs_.at(i);
    return constSubseqs;
}

/**
  * Simply call QList::move() except accept negative indices in addition to positive indices (1-based)
  *
  * @param from [int]
  * @param to [int]
  * @see moveRowRelative(), moveRowRange(), moveRowRangeRelative()
  */
void Msa::moveRow(int from, int to)
{
    from = positiveRowIndex(from);
    to = positiveRowIndex(to);

    Q_ASSERT_X(from >= 1 && from <= subseqCount(), "Msa::move", "from index out of range");
    Q_ASSERT_X(to >= 1 && to <= subseqCount(), "Msa::move", "to index out of range");
    if (from < 1 || from > subseqCount()    // Release mode guard
        || to < 1 || to > subseqCount())
    {
        return;
    }

    // Do nothing if they are the same values
    if (from == to)
        return;

    emit subseqsAboutToBeMoved(from, from, to);
    subseqs_.move(from-1, to-1);
    emit subseqsMoved(from, from, to);
}

/**
  * Functions similar to moveRow(), except that the subseq at from is moved delta positions away relative to its current
  * list position. The subseq will be constrained to the list bounds even if from + delta extends beyond the valid list
  * range. Depending on the values of from and delta, will not always return delta.
  *
  * @param from [int]
  * @param delta [int]
  * @return int
  * @see moveRow(), moveRowRange(), moveRowRangeRelative()
  */
int Msa::moveRowRelative(int from, int delta)
{
    from = positiveRowIndex(from);
    Q_ASSERT_X(from >= 1 && from <= subseqCount(), "Msa::moveRelative", "from out of range");

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
        emit subseqsAboutToBeMoved(from, from, to);
        subseqs_.move(from-1, to-1);
        emit subseqsMoved(from, from, to);
    }

    return actual_delta;
}

/**
  * Simultaneously moves a block of sequences between start_index and stop_index to the position to. If to is between
  * start_index and stop_index, nothing is done. Negative indices may be used, but start_index must be less or equal to
  * stop_index.
  *
  * @param start_index [int]
  * @param stop_index [int]
  * @param to [int]
  * @see moveRowRangeRelative(), moveRow(), moveRowRelative()
  */
void Msa::moveRowRange(int start_index, int stop_index, int to)
{
    start_index = positiveRowIndex(start_index);
    stop_index = positiveRowIndex(stop_index);

    Q_ASSERT_X(start_index >= 1 && start_index <= subseqCount(), "Msa::moveRange", "start_index out of range");
    Q_ASSERT_X(stop_index >= 1 && stop_index <= subseqCount(), "Msa::moveRange", "stop_index out of range");
    Q_ASSERT_X(start_index <= stop_index, "Msa::moveRange", "start_index > stop_index");

    to = positiveRowIndex(to);
    Q_ASSERT_X(to >= 1, "Msa::moveRange", "to must be at >= 1");

    int range_size = stop_index - start_index + 1;
    Q_ASSERT_X(to + range_size - 1 <= subseqCount(), "Msa::moveRange", "to + range_size must be <= n");

    if (start_index == to)
        return;

    emit subseqsAboutToBeMoved(start_index, stop_index, to);

    if (start_index < to)   // Moving up in the list
    {
        for (int i=0; i< range_size; ++i)
            subseqs_.move(start_index-1, to + range_size - 2);  // Optimization (done): Move the subseqs here instead of in the move method
//          move(start_index, to+range_size-1);
    }
    else                    // Moving down in the list (start_index > to)
    {
        for (int i=0; i< range_size; ++i)
            subseqs_.move(start_index + i - 1, to + i - 1);     // Optimization (done): Move the subseqs here instead of in the move method
//          move(start_index+i, to+i);
    }

    emit subseqsMoved(start_index, stop_index, to);
}

/**
  * Functions similar to moveRowRange(), except that the set of subseqs between start_index and stop_index are moved delta
  * positions away relative to the start_index. The subseqs will be constrained to the list bounds even if start_index
  * + delta or stop_index + delta extends beyond the valid list range.
  *
  * @param start_index [int]
  * @param stop_index [int]
  * @param delta [int]
  * @return int
  * @see moveRowRange(), moveRow(), moveRowRelative()
  */
int Msa::moveRowRangeRelative(int start_index, int stop_index, int delta)
{
    start_index = positiveRowIndex(start_index);
    stop_index = positiveRowIndex(stop_index);

    Q_ASSERT_X(start_index >= 1 && start_index <= subseqCount(), "Msa::moveRange", "start_index out of range");
    Q_ASSERT_X(stop_index >= 1 && stop_index <= subseqCount(), "Msa::moveRange", "stop_index out of range");
    Q_ASSERT_X(start_index <= stop_index, "Msa::moveRange", "start_index > stop_index");

    if (delta == 0)
        return 0;

    int actual_delta = 0;
    int to = 0;
    if (delta < 0)  // Move up
    {
        if (start_index > 1)
        {
            actual_delta = qMin(start_index - 1, qAbs(delta));
            to = start_index - actual_delta;

            actual_delta = -actual_delta;
        }
    }
    else
    {
        int nSubseqs = subseqCount();
        if (stop_index < nSubseqs)
        {
            actual_delta = qMin(nSubseqs - stop_index, delta);
            to = start_index + actual_delta;
        }
    }

    if (actual_delta)
        moveRowRange(start_index, stop_index, to);

    return actual_delta;
}

/**
  * @return const QString &
  */
const QString &Msa::name() const
{
    return name_;
}

/**
  * Utility function strictly used for translating *column* indices to their negative position (relative to the right
  * alignment boundary). Returns 0 if 0 is passed in or there are no sequences in the alignment.
  *
  * @param i [int]
  * @return int
  * @see negativeRowIndex(), positiveColIndex(), positiveRowIndex()
  */
int Msa::negativeColIndex(int i) const
{
    if (i != 0 && subseqs_.count())
      return (i > 0) ? i - length() - 1 : i;

    return 0;
}

/**
  * Utility function strictly used for translating *row* indices to their negative equivalent (relative to the last
  * subseq). Returns 0 if 0 is passed in or there are no sequences in the alignment.
  *
  * @param i [int]
  * @return int
  * @see negativeColIndex(), positiveColIndex(), positiveRowIndex()
  */
int Msa::negativeRowIndex(int i) const
{
    if (i != 0 && subseqs_.count())
      return (i > 0) ? i - subseqs_.count() - 1 : i;

    return 0;
}

/**
  * Utility function strictly used for translating *column* indices to their positive position (relative to the left
  * alignment boundary). Returns 0 if 0 is passed in or there are no sequences in the alignment.
  *
  * @param i [int]
  * @return int
  * @see negativeColIndex(), negativeRowIndex(), positiveRowIndex()
  */
int Msa::positiveColIndex(int i) const
{
    if (i != 0 && subseqs_.count())
        return (i < 0) ? i + length() + 1 : i;

    return 0;
}

/**
  * Utility function strictly used for translating *row* indices to their positive equivalent (relative to the first
  * sequence). Returns 0 if 0 is passed in or there are no sequences in the alignment.
  *
  * @param i [int]
  * @return int
  * @see negativeColIndex(), negativeRowIndex(), positiveColIndex()
  */
int Msa::positiveRowIndex(int i) const
{
    if (i != 0 && subseqs_.count())
        return (i < 0) ? i + subseqs_.count() + 1 : i;

    return 0;
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
    Q_ASSERT_X(subseq, "Msa::prepend", "subseq pointer must not be null");
    if (subseq->bioString().alphabet() != alphabet_)
        return false;

    if (subseqs_.count() == 0 ||
        subseq->bioString().length() == length())
    {
        emit subseqsAboutToBeInserted(1, 1);
        subseqs_.prepend(subseq);
        emit subseqsInserted(1, 1);

        return true;
    }

    return false;
}

/**
  * Simply call QList::removeAt() except accept negative indices in addition to positive indices (1-based)
  *
  * @param i [int]
  * @see removeFirst(), removeLast(), clear()
  */
void Msa::removeAt(int i)
{
    i = positiveRowIndex(i);

    Q_ASSERT_X(i >= 1 && i <= subseqCount(), "Msa::removeAt", "index out of range");

    emit subseqsAboutToBeRemoved(i, i);

    Subseq *subseq = subseqs_.takeAt(i-1);
    delete subseq;
    subseq = 0;

    emit subseqsRemoved(i, i);
}

/**
  * Simply call QList::removeFirst()
  */
void Msa::removeFirst()
{
    if (subseqs_.count())
    {
        emit subseqsAboutToBeRemoved(1, 1);

        Subseq *firstSubseq = subseqs_.takeFirst();
        delete firstSubseq;
        firstSubseq = 0;

        emit subseqsRemoved(1, 1);
    }
}

/**
  * Remove those columns the consist entirely of gaps regardless of the gap character. Returns the number of gap columns
  * that were removed.
  *
  * Keep track of contiguous gapped columns and then removeGaps on this block as a whole. This approach is much more optimal
  * than calling removeGaps for each individual column.
  *
  * @return int
  * @see insertGapColumns()
  */
int Msa::removeGapColumns()
{
    int nSubseqs = subseqCount();
    if (nSubseqs == 0)
        return 0;

    // Walk through the entire alignment column by column and identify those columns that solely contain
    // gap characters.
    int nColsToRemove = 0;
    QList<QPair<int, int> > contiguousGapColumns;
    for (int i=1, z=length(); i <= z; ++i)
    {
        bool all_gaps = true;
        for (int j=0; j< nSubseqs; ++j)
        {
            //                if (BioString::isGap(subseqs_.at(j)->bioString().sequence().at(i-1)) == false)
            if (BioString::isGap(subseqs_.at(j)->at(i)) == false)
            {
                all_gaps = false;
                break;
            }
        }

        if (!all_gaps)
            continue;

        if (contiguousGapColumns.isEmpty() || contiguousGapColumns.last().second != i-1)
            contiguousGapColumns.append(qMakePair(i, i));
        else // This gap column is contiguous with the previous all gap column. Increment the last all gap range column.
            ++contiguousGapColumns.last().second;

        ++nColsToRemove;
    }

    if (nColsToRemove == 0)
        return 0;

    // There is at least one gap column to remove
    emit gapColumnsAboutToBeRemoved(nColsToRemove);

    for (int i=contiguousGapColumns.count()-1; i >= 0; --i)
    {
        int first = contiguousGapColumns.at(i).first;
        int contiguousGapSize = contiguousGapColumns.at(i).second - first + 1;
        for (int j=0; j< nSubseqs; ++j)
            subseqs_[j]->removeGaps(first, contiguousGapSize);
    }

    emit gapColumnsRemoved(nColsToRemove);

    return nColsToRemove;
}

/**
  * Simply call QList::removeFirst()
  */
void Msa::removeLast()
{
    int nSubseqs = subseqCount();
    if (nSubseqs)
    {
        emit subseqsAboutToBeRemoved(nSubseqs, nSubseqs);

        Subseq *lastSubseq = subseqs_.takeLast();
        delete lastSubseq;
        lastSubseq = 0;

        emit subseqsRemoved(nSubseqs, nSubseqs);
    }
}

/**
  * @param description [const QString &]
  */
void Msa::setDescription(const QString &description)
{
    description_ = description;
}

/**
  * @param description [const QString &]
  */
void Msa::setName(const QString &name)
{
    name_ = name;
}

/**
  * If necessary to accommodate the newStart position, the alignment left border is extended by inserting the
  * appropriate number of gap columns.
  *
  * Most operations are simply extensions (replacing gaps with characters) or trims (replacing characters with gaps).
  * For example,
  * ABC-- >>> --C-- (trim)
  * --C-- >>> ABC-- (extension)
  *
  * It is easy to see from the above that the only information that needs to be transmitted is the difference relative
  * to non-gap characters. In other words, we can deduce the gap information. We know that an extension replaces N gaps
  * with N non-gap sequence characters. Conversely, a trim operation replaces N sequence characters (possibly containing
  * gaps) with N gap characters. It is not explicitly necessary to transmit an old and new alignment, but rather only
  * the leftmost msa column in this operation, and the string with the relevant non-gap data (trim operation may
  * contain gaps but it must have at least one non-gap character).
  *
  * A special case that must be handled occurs when the start position is moved beyond the current stop position:
  * --C
  * setStart(4)
  * ---D
  *
  * Or a slightly more complicated version:
  * --C
  * setStart(7)
  * -----F
  *
  * Technically, --C should be extended to --CDEF and then finally trimmed to -----F; however, Subseq::setStart called
  * in this method does this transformation in one step. Two issues must be addressed/recognized here:
  * 1) There are two separate Msa signals - subseqExtended and subseqTrimmed - corresponding to this change
  *    - The problem here is that Subseq is in its final state, yet two signals must be emitted before connected
  *      components will be once again syncrhonized. More importantly, it is impossible for connected components to
  *      trust the state of the Subseq relative to the signal information.
  * 2) What information should be transmitted for the extended and trimmed operations?
  *    Options:
  *    A) subseqExtended(7, F), subseqTrimmed(3, C)
  *    B) subseqExtended(4, DEF), subseqTrimmed(3, CDE)
  *    - Also, should we send the extended first and then trimmed signal, or vice versa?
  *
  * Solution: first extend the Subseq by calling Subseq::setStop, emit the subseqExtended signal, call Subseq::setStart,
  * and finally emit the subseqTrimmed signal. This approach, while less optimized, keeps the state of the Subseq in
  * sync with the Msa signals and permits all connected components to always trust both the Subseq state and the data
  * distributed with the signal about the Msa. This also addresses the second issue - subseqExtended should be called
  * prior to subseqTrimmed.
  *
  * Emits: subseqStartChanged, subseqExtended, subseqTrimmed [subseqStopChanged, gapColumnsInserted]
  *
  * @param subseqIndex [int]
  * @param newStart [int]
  * @see setSubseqStop()
  */
void Msa::setSubseqStart(int subseqIndex, int newStart)
{
    ASSERT_X(subseqIndex > 0 && subseqIndex <= subseqCount(), "subseqIndex out of range");
    ASSERT_X(newStart > 0 && newStart <= subseqs_.at(subseqIndex-1)->anonSeq().bioString().length(), "newStart out of range");
    ASSERT_X(newStart > 0, "newStart out of range");

    QPair<int, int> affectedColumns = __setSubseqStart(subseqIndex, newStart);
    if (affectedColumns.first != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);
}

/**
  * If necessary to accommodate the newStop position, the alignment right border is extended by inserting the
  * appropriate number of gap columns. For detailed information, see setSubseqStart.
  *
  * Emits: subseqStopChanged, subseqExtended, subseqTrimmed [subseqStartChanged, gapColumnsInserted]
  *
  * @param subseqIndex [int]
  * @param newStop [int]
  * @see setSubseqStart()
  */
void Msa::setSubseqStop(int subseqIndex, int newStop)
{
    ASSERT_X(subseqIndex > 0 && subseqIndex <= subseqCount(), "subseqIndex out of range");
    ASSERT_X(newStop > 0 && newStop <= subseqs_.at(subseqIndex-1)->anonSeq().bioString().length(), "newStop out of range");
    ASSERT_X(newStop > 0, "newStop out of range");

    QPair<int, int> affectedColumns = __setSubseqStop(subseqIndex, newStop);
    if (affectedColumns.first != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);
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
  * Emits the regionSlid signal if successfully slid region at least one position. The delta emitted corresponds to the
  * the actual delta and not the requested delta.
  *
  * @param left [int]
  * @param top [int]
  * @param right [int]
  * @param bottom [int]
  * @param delta [int]
  * @return int - the actual direction and number of positions slid
  */
int Msa::slideRegion(int left, int top, int right, int bottom, int delta)
{
    Q_ASSERT_X(subseqCount() != 0, "Msa::slideRegion", "There are no sequences in the Msa");
    if (subseqCount() == 0) // Release mode guard
        return 0;

    // Do nothing if the region is not to be slid in any direction
    if (delta == 0)
        return 0;

    Q_ASSERT_X(left != 0, "Msa::slideRegion", "left may not be 0");
    Q_ASSERT_X(top != 0, "Msa::slideRegion", "top may not be 0");
    Q_ASSERT_X(right != 0, "Msa::slideRegion", "right may not be 0");
    Q_ASSERT_X(bottom != 0, "Msa::slideRegion", "bottom may not be 0");

    // Convert to positive coordinates
    left = positiveColIndex(left);
    right = positiveColIndex(right);
    if (left > right)
        qSwap(left, right);

    top = positiveRowIndex(top);
    bottom = positiveRowIndex(bottom);
    if (top > bottom)
        qSwap(top, bottom);

    Q_ASSERT_X(left > 0 && left <= length(), "Msa::slideRegion", "left out of range");
    Q_ASSERT_X(top > 0 && top <= subseqCount(), "Msa::slideRegion", "top out of range");
    Q_ASSERT_X(right > 0 && right <= length(), "Msa::slideRegion", "right out of range");
    Q_ASSERT_X(bottom > 0 && bottom <= subseqCount(), "Msa::slideRegion", "bottom out of range");
    Q_ASSERT_X(left <= right, "Msa::slideRegion", "left must be <= right");
    Q_ASSERT_X(top <= bottom, "Msa::slideRegion", "top must be <= bottom");

    // Release mode guards for out of invalid coordinates
    if (left <= 0
        || right > length()
        || top <= 0
        || bottom > subseqCount())
    {
        return 0;
    }

    int actualDelta = 0;
    if (delta < 0)  // Slide to the left
    {
        actualDelta = subseqs_[top-1]->bioString().leftSlidablePositions(left, right);
        for (int i=top+1; actualDelta > 0 && i<= bottom; ++i)
        {
            int tmp = subseqs_[i-1]->bioString().leftSlidablePositions(left, right);
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
        actualDelta = subseqs_[top-1]->bioString().rightSlidablePositions(left, right);
        for (int i=top+1; actualDelta > 0 && i<= bottom; ++i)
        {
            int tmp = subseqs_[i-1]->bioString().rightSlidablePositions(left, right);
            if (tmp < actualDelta)
                actualDelta = tmp;
        }

        // If actualDelta can slide more spots than requested, limit to the number of spots requested
        if (actualDelta > delta)
            actualDelta = delta;
    }

    if (actualDelta)
    {
        for (int i=top-1; i< bottom; ++i)
            subseqs_[i]->slideSegment(left, right, actualDelta);

        emit regionSlid(left, top, right, bottom, actualDelta, left + actualDelta, right + actualDelta);
    }

    return actualDelta;
}

/**
  * If there are no subseq members, do nothing.
  *
  * @param greaterThan [const SubseqGreaterThan &]
  * @see sort(SubseqLessThan)
  */
void Msa::sort(const SubseqGreaterThan &greaterThan)
{
    if (subseqs_.isEmpty())
        return;

    emit subseqsAboutToBeSorted();
    qStableSort(subseqs_.begin(), subseqs_.end(), greaterThan);
    emit subseqsSorted();
}

/**
  * If there are no subseq members, do nothing.
  *
  * @param greaterThan [const SubseqLessThan &]
  * @see sort(SubseqGreaterThan)
  */
void Msa::sort(const SubseqLessThan &lessThan)
{
    if (subseqs_.isEmpty())
        return;

    emit subseqsAboutToBeSorted();
    qStableSort(subseqs_.begin(), subseqs_.end(), lessThan);
    emit subseqsSorted();
}

/**
  * Simply call QList::count()
  *
  * @return int
  */
int Msa::subseqCount() const
{
    return subseqs_.count();
}

/**
  * Convenience routine.
  *
  * @returns QList<int>
  */
QList<int> Msa::subseqIds() const
{
    QList<int> subseqIds;
    foreach (Subseq *subseq, subseqs_)
        subseqIds << subseq->id();

    return subseqIds;
}

/**
  * Simply call QList::swap() except accept negative indices in addition to positive indices (1-based)
  *
  * @param i [int]
  * @param j [int]
  */
void Msa::swap(int i, int j)
{
    i = positiveRowIndex(i);
    j = positiveRowIndex(j);

    Q_ASSERT_X(i >= 1 && i <= subseqCount(), "Msa::swap", "index i out of range");
    Q_ASSERT_X(j >= 1 && j <= subseqCount(), "Msa::swap", "index j out of range");

    if (i == j)
        return;

    emit subseqAboutToBeSwapped(i, j);

    // Convert 1-based i and j to 0-based QList operations
    subseqs_.swap(i-1, j-1);

    emit subseqSwapped(i, j);
}

/**
  * Will potentially remove all characters from the left boundary to msaColumn. The one exception to this rule is if
  * the trim operation would remove the last non-gap character from this subseq. In this case, all but the last
  * character will be removed.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  */
void Msa::trimSubseqsLeft(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    QPair<int, int> affectedColumns = __trimSubseqsLeft(top, bottom, msaColumn);

    // We know that if at least one column was affected, the second value of affectedColumns will be non-zero
    if (affectedColumns.second != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);
}

/**
  * Performs similarly to trimSubseqsLeft except applies to the right terminus of the alignment.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @see trimSubseqsLeft()
  */
void Msa::trimSubseqsRight(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    QPair<int, int> affectedColumns = __trimSubseqsRight(top, bottom, msaColumn);

    // We know that if at least one column was affected, the second value of affectedColumns will be non-zero
    if (affectedColumns.second != 0)
        emit extendOrTrimFinished(affectedColumns.first, affectedColumns.second);

    /*
    for (int i=top; i<= bottom; ++i)
    {
        Subseq *subseq = subseqs_.at(i-1);
        int nTrimmableChars = subseq->bioString().nonGapCharsBetween(msaColumn, length());

        if (subseq->ungappedLength() - nTrimmableChars < 1)
            --nTrimmableChars;

        setSubseqStop(i, subseq->stop() - nTrimmableChars);
    }
    */
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * All changes to the actual sequence data are performed by calling setSubseqStart and setSubseqStop.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @returns QPair<int, int>
  * @see extendSubseqsLeft()
  */
QPair<int, int> Msa::__extendSubseqsLeft(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    // Track extents of columns affected by this operation; initialize to maximal variables.
    // Callers can test if any change occurred by checking for a non-zero second value in the
    // returned QPair<int, int>.
    QPair<int, int> columnRangeAffected(length() + 1, 0);

    for (int i=top; i<= bottom; ++i)
    {
        Subseq *subseq = subseqs_.at(i-1);
        int nFillableGaps = subseq->headGaps() - msaColumn + 1;
        if (nFillableGaps < 1)
            continue;

        int oldStart = subseq->start();
        int newStart = qMax(1, oldStart - nFillableGaps);
        if (oldStart - newStart == 0)   // No new characters
            continue;

        // Update the start position
        QPair<int, int> affectedColumns = __setSubseqStart(i, newStart);

        // Update the leftmost/rightmost affected columns
        if (affectedColumns.first < columnRangeAffected.first)
            columnRangeAffected.first = affectedColumns.first;
        if (affectedColumns.second > columnRangeAffected.second)
            columnRangeAffected.second = affectedColumns.second;
    }

    return columnRangeAffected;
}

/**
  * Performs similarly to __extendSubseqsLeft except applies to the right boundary of the alignment.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @returns QPair<int, int>
  * @see extendSubseqsRight()
  */
QPair<int, int> Msa::__extendSubseqsRight(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    int msaLength = length();

    // Track extents of columns affected by this operation; initialize to maximal variables.
    // Callers can test if any change occurred by checking for a non-zero second value in the
    // returned QPair<int, int>.
    QPair<int, int> columnRangeAffected(msaLength + 1, 0);

    for (int i=top; i<= bottom; ++i)
    {
        Subseq *subseq = subseqs_.at(i-1);
        int nFillableGaps = msaColumn - (msaLength - subseq->tailGaps());
        if (nFillableGaps < 1)
            continue;

        int oldStop = subseq->stop();
        int newStop = qMin(subseq->anonSeq().bioString().length(), oldStop + nFillableGaps);
        if (newStop - oldStop == 0)     // No new chars
            continue;

        // Update the stop position
        QPair<int, int> affectedColumns = __setSubseqStop(i, newStop);

        // Update the leftmost/rightmost affected columns
        if (affectedColumns.first < columnRangeAffected.first)
            columnRangeAffected.first = affectedColumns.first;
        if (affectedColumns.second > columnRangeAffected.second)
            columnRangeAffected.second = affectedColumns.second;
    }

    return columnRangeAffected;
}

/**
  * All changes to the actual sequence data are performed by calling setSubseqStart and setSubseqStop.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @returns QPair<int, int>
  * @see trimSubseqsLeft()
  */
QPair<int, int> Msa::__trimSubseqsLeft(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    // Track extents of columns affected by this operation; initialize to maximal variables.
    // Callers can test if any change occurred by checking for a non-zero second value in the
    // returned QPair<int, int>.
    QPair<int, int> columnRangeAffected(length() + 1, 0);

    for (int i=top; i<= bottom; ++i)
    {
        Subseq *subseq = subseqs_.at(i-1);
        int nTrimmableChars = subseq->bioString().nonGapCharsBetween(1, msaColumn);

        // Prevent trim operations from removing all characters
        if (subseq->ungappedLength() - nTrimmableChars < 1)
            --nTrimmableChars;

        if (nTrimmableChars == 0)
            continue;

        int oldStart = subseq->start();
        int newStart = oldStart + nTrimmableChars;
        if (newStart - oldStart == 0)
            continue;

        // Update the start position
        QPair<int, int> affectedColumns = __setSubseqStart(i, newStart);

        // Update the leftmost/rightmost affected columns
        if (affectedColumns.first < columnRangeAffected.first)
            columnRangeAffected.first = affectedColumns.first;
        if (affectedColumns.second > columnRangeAffected.second)
            columnRangeAffected.second = affectedColumns.second;
    }

    return columnRangeAffected;
}

/**
  * All changes to the actual sequence data are performed by calling setSubseqStart and setSubseqStop.
  *
  * @param top [int]
  * @param bottom [int]
  * @param msaColumn [int]
  * @returns QPair<int, int>
  * @see trimSubseqsRight()
  */
QPair<int, int> Msa::__trimSubseqsRight(int top, int bottom, int msaColumn)
{
    ASSERT_X(top > 0 && top <= subseqCount(), "top out of range");
    ASSERT_X(bottom >= top && bottom <= subseqCount(), "bottom out of range");
    ASSERT_X(msaColumn > 0 && msaColumn <= length(), "msaColumn out of range");

    // Track extents of columns affected by this operation; initialize to maximal variables.
    // Callers can test if any change occurred by checking for a non-zero second value in the
    // returned QPair<int, int>.
    QPair<int, int> columnRangeAffected(length() + 1, 0);

    for (int i=top; i<= bottom; ++i)
    {
        Subseq *subseq = subseqs_.at(i-1);
        int nTrimmableChars = subseq->bioString().nonGapCharsBetween(msaColumn, length());

        if (subseq->ungappedLength() - nTrimmableChars < 1)
            --nTrimmableChars;

        if (nTrimmableChars == 0)
            continue;

        int oldStop = subseq->stop();
        int newStop = oldStop - nTrimmableChars;
        if (oldStop - newStop == 0)
            continue;

        // Update the stop position
        QPair<int, int> affectedColumns = __setSubseqStop(i, newStop);

        // Update the leftmost/rightmost affected columns
        if (affectedColumns.first < columnRangeAffected.first)
            columnRangeAffected.first = affectedColumns.first;
        if (affectedColumns.second > columnRangeAffected.second)
            columnRangeAffected.second = affectedColumns.second;
    }

    return columnRangeAffected;
}

/**
  * The pair of integers returned indicate the leftmost and rightmost columns that were affected and are 1-based.
  *
  * @param subseqIndex [int]
  * @param newStart [int]
  * @returns QPair<int, int>
  */
QPair<int, int> Msa::__setSubseqStart(int subseqIndex, int newStart)
{
    ASSERT_X(subseqIndex > 0 && subseqIndex <= subseqCount(), "subseqIndex out of range");
    ASSERT_X(newStart > 0 && newStart <= subseqs_.at(subseqIndex-1)->anonSeq().bioString().length(), "newStart out of range");
    ASSERT_X(newStart > 0, "newStart out of range");

    // Do nothing if the start is not different
    Subseq *subseq = subseqs_.at(subseqIndex - 1);
    int oldStart = subseq->start();
    if (newStart == oldStart)
        return QPair<int, int>(0, 0);

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

        // Emit the startChanged signal
        emit subseqStartChanged(subseqIndex, newStart, oldStart);

        // Emit the extended signal
        int msaColumn = nHeadGaps - nNewChars + 1;
        emit subseqExtended(subseqIndex, msaColumn, subseq->bioString().mid(msaColumn, nNewChars));

        return QPair<int, int>(msaColumn, msaColumn + nNewChars - 1);

        // ---------------------------------------------
        // ---------------------------------------------
        // Emit the border changed signal
        /*
        emit subseqBorderChanged(subseqIndex,
                                 msaColumn,
                                 subseq->bioString().mid(msaColumn, nNewChars),
                                 QString("-").repeated(nNewChars));
        */
        // ---------------------------------------------
        // ---------------------------------------------
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

            const QString &sequence = subseq->bioString().sequence();
            const QChar *x = sequence.constData();
            x += startMsaColumn;
            while (charsFound != nCharsToRemove) // ASSERT && *x != '\0')
            {
                if (!BioString::isGap(*x))
                    ++charsFound;

                ++endMsaColumn;
                ++x;
            }

            // Save the old alignment
            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            QString oldAlignment = sequence.mid(startMsaColumn - 1, nMsaColumns);

            // Update the start position
            subseq->setStart(newStart);

            emit subseqStartChanged(subseqIndex, newStart, oldStart);

            // Emit the trimmed signal
            emit subseqTrimmed(subseqIndex, startMsaColumn, oldAlignment);

            return QPair<int, int>(startMsaColumn, endMsaColumn);

            // ---------------------------------------------
            // ---------------------------------------------
            // Emit the border changed signal
            /*
            emit subseqBorderChanged(subseqIndex,
                                     startMsaColumn,
                                     QString("-").repeated(nMsaColumns),
                                     oldAlignment);
                                     */
            // ---------------------------------------------
            // ---------------------------------------------
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

            // Update the stop position
            subseq->setStop(newStop);

            // Emit the subseq stop changed signal
            emit subseqStopChanged(subseqIndex, newStop, oldStop);

            // Emit the extended signal
            int extendStartColumn = length() - nTailGaps + 1; // 1-based
            emit subseqExtended(subseqIndex, extendStartColumn, subseq->bioString().mid(extendStartColumn, nNewChars));

            // Update the start position
            int trimStartColumn = subseq->headGaps() + 1;   // Important that we know the number of headgaps *before*
                                                            // updating the start position and gather alignment data
            int trimStopColumn = extendStartColumn + nNewChars - 2;
            int nMsaColumns = trimStopColumn - trimStartColumn + 1;
            QString trimmings = subseq->bioString().mid(trimStartColumn, nMsaColumns);
            subseq->setStart(newStart);

            // Emit the subseq start changed signal
            emit subseqStartChanged(subseqIndex, newStart, oldStart);

            // Emit the trimmed signal
            emit subseqTrimmed(subseqIndex, trimStartColumn, trimmings);

            return QPair<int, int>(trimStartColumn, trimStopColumn + 1);

            // ---------------------------------------------
            // ---------------------------------------------
            // Subseq border changed signal - no longer used
            /*
            // Obsolete!!
            int startMsaColumn = subseq->headGaps() + 1;
            int endMsaColumn = length() - nTailGaps + (newStart - oldStop);
            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            QString oldAlignment = subseq->bioString().mid(startMsaColumn, nMsaColumns);

            // Emit the border changed signal
            emit subseqBorderChanged(subseqIndex,
                                     startMsaColumn,
                                     subseq->bioString().mid(startMsaColumn, nMsaColumns),
                                     oldAlignment);
            */
            // ---------------------------------------------
            // ---------------------------------------------
        }
    }

    ASSERT_X(0, "Impossible location to reach!");
}

/**
  * The pair of integers returned indicate the leftmost and rightmost columns that were affected and are 1-based.
  *
  * @param subseqIndex [int]
  * @param newStop [int]
  * @returns QPair<int, int>
  */
QPair<int, int> Msa::__setSubseqStop(int subseqIndex, int newStop)
{
    ASSERT_X(subseqIndex > 0 && subseqIndex <= subseqCount(), "subseqIndex out of range");
    ASSERT_X(newStop > 0 && newStop <= subseqs_.at(subseqIndex-1)->anonSeq().bioString().length(), "newStop out of range");
    ASSERT_X(newStop > 0, "newStop out of range");

    // Do nothing if the stop is not different
    Subseq *subseq = subseqs_.at(subseqIndex - 1);
    int oldStop = subseq->stop();
    if (newStop == oldStop)
        return QPair<int, int>(0, 0);

    int oldLength = length();

    // Two cases:
    // 1) newStop > stop :: adding characters
    if (newStop > oldStop)
    {
        int nNewChars = newStop - oldStop;
        int nTailGaps = subseq->tailGaps();

        // Insert any new gap columns as needed to accommodate the new sequence characters
        insertGapColumns(oldLength + 1, qMax(0, nNewChars - nTailGaps));

        // Update the stop position
        subseq->setStop(newStop);

        // Emit the startChanged signal
        emit subseqStopChanged(subseqIndex, newStop, oldStop);

        // Emit the extended signal
        int msaColumn = oldLength - nTailGaps + 1;
        emit subseqExtended(subseqIndex, msaColumn, subseq->bioString().mid(msaColumn, nNewChars));

        return QPair<int, int>(msaColumn, msaColumn + nNewChars - 1);

        // ---------------------------------------------
        // ---------------------------------------------
        // Emit the border changed signal
        /*
        emit subseqBorderChanged(subseqIndex,
                                 msaColumn,
                                 subseq->bioString().mid(msaColumn, nNewChars),
                                 QString("-").repeated(nNewChars));
        */
        // ---------------------------------------------
        // ---------------------------------------------
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

            const QString &sequence = subseq->bioString().sequence();
            const QChar *x = sequence.constData() + endMsaColumn - 2;   // Minus 2 = -1 because going from 1-based to
                                                                        // zero based and -1 because we are ignoring
                                                                        // character already found
            while (charsFound != nCharsToRemove)
            {
                if (!BioString::isGap(*x))
                    ++charsFound;

                --startMsaColumn;
                --x;
            }

            // Save the old alignment
            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            QString oldAlignment = sequence.mid(startMsaColumn - 1, nMsaColumns);

            // Update the stop position
            subseq->setStop(newStop);

            emit subseqStopChanged(subseqIndex, newStop, oldStop);

            // Emit the trimmed signal
            emit subseqTrimmed(subseqIndex, startMsaColumn, oldAlignment);

            return QPair<int, int>(startMsaColumn, endMsaColumn);

            // ---------------------------------------------
            // ---------------------------------------------
            // Emit the border changed signal
            /*
            emit subseqBorderChanged(subseqIndex,
                                     startMsaColumn,
                                     QString("-").repeated(nMsaColumns),
                                     oldAlignment);
            */
            // ---------------------------------------------
            // ---------------------------------------------
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

            // Update the start position
            subseq->setStart(newStart);

            // Emit the subseq start changed signal
            emit subseqStartChanged(subseqIndex, newStart, oldStart);

            // Emit the extended signal
            int extendStartColumn = nHeadGaps - nNewChars + 1;
            emit subseqExtended(subseqIndex, extendStartColumn, subseq->bioString().mid(extendStartColumn, nNewChars));

            // Update the stop position
            int trimStartColumn = extendStartColumn + 1;
            int trimStopColumn = length() - subseq->tailGaps();
            int nMsaColumns = trimStopColumn - trimStartColumn + 1;
            QString trimmings = subseq->bioString().mid(trimStartColumn, nMsaColumns);
            subseq->setStop(newStop);

            // Emit the subseq stop changed signal
            emit subseqStopChanged(subseqIndex, newStop, oldStop);

            // Emit the trimmed signal
            emit subseqTrimmed(subseqIndex, trimStartColumn, trimmings);

            return QPair<int, int>(extendStartColumn, trimStopColumn);

            // ---------------------------------------------
            // ---------------------------------------------
            // Emit the border changed signal
            /*
            // Obsolete!
            int endMsaColumn = length() - subseq->tailGaps();
            int startMsaColumn = nHeadGaps - (oldStart - newStop) + 1;
            int nMsaColumns = endMsaColumn - startMsaColumn + 1;
            QString oldAlignment = subseq->bioString().mid(startMsaColumn, nMsaColumns);
            emit subseqBorderChanged(subseqIndex,
                                     startMsaColumn,
                                     subseq->bioString().mid(startMsaColumn, nMsaColumns),
                                     oldAlignment);
            */
            // ---------------------------------------------
            // ---------------------------------------------
        }
    }

    ASSERT_X(0, "Impossible location to reach!");
}
