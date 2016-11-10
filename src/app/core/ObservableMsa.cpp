/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param grammar [Grammar]
  * @param parent [QObject *]
  */
ObservableMsa::ObservableMsa(Grammar grammar, QObject *parent)
    : QObject(parent), Msa(grammar), modified_(false)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool ObservableMsa::isModified() const
{
    return modified_;
}

/**
  * @param lessThan [bool (*)(const Subseq *a, const Subseq *b)]
  */
void ObservableMsa::sort(bool (*lessThan)(const Subseq *a, const Subseq *b))
{
    emit rowsAboutToBeSorted();
    Msa::sort(lessThan);
    emit rowsSorted();
}

/**
  * @param subseqLessThan [const ISubseqLessThan &]
  * @param sortOrder [Qt::SortOrder]
  */
void ObservableMsa::sort(const ISubseqLessThan &subseqLessThan, Qt::SortOrder sortOrder)
{
    emit rowsAboutToBeSorted();
    Msa::sort(subseqLessThan, sortOrder);
    emit rowsSorted();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param subseq [Subseq *]
  * @returns bool
  */
bool ObservableMsa::append(Subseq *subseq)
{
    if (isCompatibleSubseq(subseq))
    {
        int row = rowCount() + 1;
        ClosedIntRange insertRange(row, row);

        emit rowsAboutToBeInserted(insertRange);
        subseqs_ << subseq;
        emit rowsInserted(insertRange);

        return true;
    }

    return false;
}

/**
  */
void ObservableMsa::clear()
{
    emit msaAboutToBeReset();
    Msa::clear();
    emit msaReset();
}

/**
  * @param msaRect [const PosiRect &]
  * @returns QVector<SubseqChangePod>
  */
SubseqChangePodVector ObservableMsa::collapseLeft(const PosiRect &msaRect)
{
    PosiRect normalizedRect = msaRect.normalized();

    emit aboutToBeCollapsedLeft(normalizedRect);
    SubseqChangePodVector pods = Msa::collapseLeft(normalizedRect);
    emit collapsedLeft(normalizedRect);
    emit subseqsChanged(pods);

    return pods;
}

/**
  * @param msaRect [const PosiRect &]
  * @returns QVector<SubseqChangePod>
  */
SubseqChangePodVector ObservableMsa::collapseRight(const PosiRect &msaRect)
{
    PosiRect normalizedRect = msaRect.normalized();

    emit aboutToBeCollapsedRight(normalizedRect);
    SubseqChangePodVector pods = Msa::collapseRight(normalizedRect);
    emit collapsedRight(normalizedRect);
    emit subseqsChanged(pods);

    return pods;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::extendLeft(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::extendLeft(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param row [int]
  * @param nCharsToExtend [int]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::extendLeft(int row, int nCharsToExtend)
{
    static SubseqChangePodVector podVector(1);

    SubseqChangePod pod = Msa::extendLeft(row, nCharsToExtend);
    if (pod.isNull() == false)
    {
        podVector[0] = pod;
        emit subseqsChanged(podVector);
    }

    return pod;
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @param extension [BioString]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::extendLeft(int msaColumn, int row, const BioString &extension)
{
    static SubseqChangePodVector podVector(1);

    SubseqChangePod pod = Msa::extendLeft(msaColumn, row, extension);
    if (pod.isNull() == false)
    {
        podVector[0] = pod;
        emit subseqsChanged(podVector);
    }

    return pod;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::extendRight(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::extendRight(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param row [int]
  * @param nCharsToExtend [int]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::extendRight(int row, int nCharsToExtend)
{
    static SubseqChangePodVector podVector(1);

    SubseqChangePod pod = Msa::extendRight(row, nCharsToExtend);
    if (pod.isNull() == false)
    {
        podVector[0] = pod;
        emit subseqsChanged(podVector);
    }

    return pod;
}

/**
  * @param msaColumn [int]
  * @param row [int]
  * @param extension [BioString]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::extendRight(int msaColumn, int row, const BioString &extension)
{
    static SubseqChangePodVector podVector(1);

    SubseqChangePod pod = Msa::extendRight(msaColumn, row, extension);
    if (pod.isNull() == false)
    {
        podVector[0] = pod;
        emit subseqsChanged(podVector);
    }

    return pod;
}

/**
  * @param i [int]
  * @param subseq [Subseq *]
  * @returns bool
  */
bool ObservableMsa::insert(int i, Subseq *subseq)
{
    ASSERT_X(i >= 1 && i <= subseqCount() + 1, "index out of range");
    if (!isCompatibleSubseq(subseq))
        return false;

    ClosedIntRange insertRange(i, i);
    emit rowsAboutToBeInserted(insertRange);
    subseqs_.insert(i-1, subseq);
    emit rowsInserted(insertRange);

    return true;
}

/**
  * @param column [int]
  * @param count [int]
  * @param gapCharacter [char]
  */
void ObservableMsa::insertGapColumns(int column, int count, char gapCharacter)
{
    if (count == 0)
        return;

    ClosedIntRange insertRange(column, column + count - 1);
    emit gapColumnsAboutToBeInserted(insertRange);
    Msa::insertGapColumns(column, count, gapCharacter);
    emit gapColumnsInserted(insertRange);
}

/**
  * Repeated code as in Msa because it is not possible in the current implementation to emit the relevant signals before
  * insertion because both the compatibility check and insertion are done in the same method call.
  *
  * @param row [int]
  * @param subseqs [const QVector<Subseq *>]
  * @returns bool
  */
bool ObservableMsa::insertRows(int row, const QVector<Subseq *> subseqs)
{
    ASSERT_X(isValidRow(row) || row == subseqs_.size() + 1, "row out of range");
    if (!isCompatibleSubseqVector(subseqs))
        return false;

    ClosedIntRange insertRange(row, row + subseqs.size() - 1);
    emit rowsAboutToBeInserted(insertRange);

    // Expand the vector with null pointers
    subseqs_.insert(row - 1, subseqs.size(), nullptr);

    // Copy subseqs into vector
    memcpy(subseqs_.data() + row - 1, subseqs.constData(), subseqs.size() * sizeof(Subseq *));

    emit rowsInserted(insertRange);

    return true;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::levelLeft(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::levelLeft(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::levelRight(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::levelRight(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param from [int]
  * @param to [int]
  */
void ObservableMsa::moveRow(int from, int to)
{
    ASSERT_X(from >= 1 && from <= subseqCount(), "from out of range");
    ASSERT_X(to >= 1 && to <= subseqCount(), "to out of range");

    if (from == to)
        return;

    emit rowsAboutToBeMoved(ClosedIntRange(from, from), to);
    Msa::moveRow(from, to);
    emit rowsMoved(ClosedIntRange(from, from), to);
}

/**
  * @param rows [const ClosedIntRange &]
  * @param to [int]
  */
void ObservableMsa::moveRowRange(const ClosedIntRange &rows, int to)
{
    ASSERT_X(to >= 1 && to <= subseqCount(), "to out of range");
    if (to == rows.begin_)
        return;

    emit rowsAboutToBeMoved(rows, to);
    Msa::moveRowRange(rows, to);
    emit rowsMoved(rows, to);
}

/**
  * @param rows [const ClosedIntRange &]
  * @param delta [int]
  */
void ObservableMsa::moveRowRangeRelative(const ClosedIntRange &rows, int delta)
{
    moveRowRange(rows, rows.begin_ + delta);
}

/**
  * @param from [int]
  * @param delta [int]
  */
void ObservableMsa::moveRowRelative(int from, int delta)
{
    moveRow(from, from + delta);
}

/**
  * @param subseq [Subseq *]
  * @returns bool
  */
bool ObservableMsa::prepend(Subseq *subseq)
{
    if (isCompatibleSubseq(subseq))
    {
        ClosedIntRange insertRange(1, 1);

        emit rowsAboutToBeInserted(insertRange);
        subseqs_.prepend(subseq);
        emit rowsInserted(insertRange);

        return true;
    }

    return false;
}

/**
  * @param i [int]
  */
void ObservableMsa::removeAt(int i)
{
    ClosedIntRange removeRange(i, i);
    emit rowsAboutToBeRemoved(removeRange);
    Msa::removeAt(i);
    emit rowsRemoved(removeRange);
}

/**
  */
void ObservableMsa::removeFirst()
{
    removeAt(1);
}

/**
  * @param rows [const ClosedIntRange &]
  */
void ObservableMsa::removeRows(const ClosedIntRange &rows)
{
    emit rowsAboutToBeRemoved(rows);
    Msa::removeRows(rows);
    emit rowsRemoved(rows);
}

/**
  * @returns QVector<ClosedIntRange>
  */
QVector<ClosedIntRange> ObservableMsa::removeGapColumns()
{
    QVector<ClosedIntRange> removedGapColumns = Msa::removeGapColumns();
    if (removedGapColumns.size() > 0)
        emit gapColumnsRemoved(removedGapColumns);

    return removedGapColumns;
}

/**
  * @param columnRange [const ClosedIntRange &]
  * @returns QVector<ClosedIntRange>
  */
QVector<ClosedIntRange> ObservableMsa::removeGapColumns(const ClosedIntRange &columnRange)
{
    QVector<ClosedIntRange> removedGapColumns = Msa::removeGapColumns(columnRange);
    if (removedGapColumns.size() > 0)
        emit gapColumnsRemoved(removedGapColumns);

    return removedGapColumns;
}

/**
  */
void ObservableMsa::removeLast()
{
    removeAt(rowCount());
}

/**
  * @param modified [bool]
  */
void ObservableMsa::setModified(bool modified)
{
    if (modified_ == modified)
        return;

    modified_ = modified;
    emit modifiedChanged(modified_);
}

/**
  * @param row [int]
  * @param newStart [int]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::setSubseqStart(int row, int newStart)
{
    SubseqChangePod pod = Msa::setSubseqStart(row, newStart);
    if (!pod.isNull())
        emit subseqsChanged(SubseqChangePodVector() << pod);

    return pod;
}

/**
  * @param row [int]
  * @param newStop [int]
  * @returns SubseqChangePod
  */
SubseqChangePod ObservableMsa::setSubseqStop(int row, int newStop)
{
    SubseqChangePod pod = Msa::setSubseqStop(row, newStop);
    if (!pod.isNull())
        emit subseqsChanged(SubseqChangePodVector() << pod);

    return pod;
}

/**
  * @param msaRect [const PosiRect &]
  * @param delta [int]
  * @returns int
  */
int ObservableMsa::slideRect(const PosiRect &msaRect, int delta)
{
    int actualDelta = Msa::slideRect(msaRect, delta);
    if (actualDelta != 0)
    {
        ClosedIntRange finalRange(msaRect.left() + actualDelta, msaRect.right() + actualDelta);
        if (finalRange.isEmpty())
            finalRange.invert();
        emit rectangleSlid(msaRect, actualDelta, finalRange);
    }

    return actualDelta;
}

/**
  * @param i [int]
  * @param j [int]
  */
void ObservableMsa::swap(int i, int j)
{
    if (i == j)
        return;

    emit rowsAboutToBeSwapped(i, j);
    Msa::swap(i, j);
    emit rowsSwapped(i, j);
}

/**
  * @param rows [const ClosedIntRange &]
  */
QVector<Subseq *> ObservableMsa::takeRows(const ClosedIntRange &rows)
{
    ASSERT_X(isValidRowRange(rows), "rows out of range");

    emit rowsAboutToBeRemoved(rows);
    QVector<Subseq *> extraction = Msa::takeRows(rows);
    emit rowsRemoved(rows);

    return extraction;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::trimLeft(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::trimLeft(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param msaColumn [int]
  * @param rows [const ClosedIntRange &]
  * @returns QVector<SubseqChangePod>
  */
QVector<SubseqChangePod> ObservableMsa::trimRight(int msaColumn, const ClosedIntRange &rows)
{
    QVector<SubseqChangePod> subseqChangePods = Msa::trimRight(msaColumn, rows);
    if (subseqChangePods.size() > 0)
        emit subseqsChanged(subseqChangePods);

    return subseqChangePods;
}

/**
  * @param subseqChangePodVector [const SubseqChangePodVector &]
  * @returns SubseqChangePodVector
  */
SubseqChangePodVector ObservableMsa::undo(const SubseqChangePodVector &subseqChangePodVector)
{
    SubseqChangePodVector undoneChangePods = Msa::undo(subseqChangePodVector);
    if (undoneChangePods.size() > 0)
        emit subseqsChanged(undoneChangePods);

    return undoneChangePods;
}
