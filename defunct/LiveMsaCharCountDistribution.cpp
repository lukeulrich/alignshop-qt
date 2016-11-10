/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "LiveMsaCharCountDistribution.h"

#include "Msa.h"
#include "MsaAlgorithms.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param msa [const Msa *]
  * @param parent [QObject *]
  */
LiveMsaCharCountDistribution::LiveMsaCharCountDistribution(const Msa *msa, QObject *parent)
    : LiveCharCountDistribution(parent), msa_(msa)
{
    if (msa_)
    {
        charCountDistribution_ = calculateMsaCharCountDistribution(*msa_);

        connect(msa_, SIGNAL(gapColumnsInserted(int,int)), SLOT(onMsaGapColumnsInserted(int,int)));
        connect(msa_, SIGNAL(gapColumnsRemoved(int)), SLOT(onMsaGapColumnsRemoved()));
        connect(msa_, SIGNAL(msaReset()), SLOT(onMsaReset()));
        connect(msa_, SIGNAL(subseqsInserted(int,int)), SLOT(onMsaSubseqsInserted(int,int)));
        connect(msa_, SIGNAL(subseqsAboutToBeRemoved(int,int)), SLOT(onMsaSubseqsAboutToBeRemoved(int,int)));
        connect(msa_, SIGNAL(regionSlid(int,int,int,int,int,int,int)), SLOT(onMsaRegionSlid(int,int,int,int,int,int,int)));
        connect(msa_, SIGNAL(subseqExtended(int,int,QString)), SLOT(onMsaSubseqExtended(int,int,QString)));
        connect(msa_, SIGNAL(subseqInternallyChanged(int,int,QString,QString)), SLOT(onMsaSubseqInternallyChanged(int,int,QString,QString)));
        connect(msa_, SIGNAL(subseqTrimmed(int,int,QString)), SLOT(onMsaSubseqTrimmed(int,int,QString)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Returns zero if msa is not defined. It is the caller's responsibility to ensure that no attempt is made to divide
  * by zero.
  *
  * @returns int
  */
int LiveMsaCharCountDistribution::divisor() const
{
    if (!msa_)
        return 0;

    return msa_->subseqCount();
}

/**
  * @returns const Msa *
  */
const Msa *LiveMsaCharCountDistribution::msa() const
{
    return msa_;
}

/**
  *
  *
  * @returns CharCountDistribution
  */
CharCountDistribution LiveMsaCharCountDistribution::charCountDistribution() const
{
    return charCountDistribution_;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Insert the corresponding blank columns in this distribution and emit the columnsInserted signal.
  *
  * @param column [int]
  * @param count [int]
  */
void LiveMsaCharCountDistribution::onMsaGapColumnsInserted(int column, int count)
{
    ASSERT(msa_);
    ASSERT(column >= 1 && column <= msa_->length());
    ASSERT(count >= 0);

    charCountDistribution_.insertBlanks(column, count);
    emit columnsInserted(column, column + count - 1);
}

/**
  * This slot is called whenever the MSA has had all its completely gap columns removed. Because we determine the
  * character count distribution via the calculateMsaCharCountDistribution method and it ignores all gap characters,
  * it is simply necessary to iterate over all columns and remove those that have an empty hash.
  *
  * If there are multiple non-contiguous stretches of gap columns, remove in reverse order.
  *
  * If there is at least one empty column to remove, emit the appropriate columnsRemoved signal.
  */
void LiveMsaCharCountDistribution::onMsaGapColumnsRemoved()
{
    ASSERT(msa_);

    ListHashCharInt charCounts = charCountDistribution_.charCounts();
    QList<QPair<int, int> > contiguousEmptyColumns;
    for (int i=0, z=charCounts.length(); i < z; ++i)
    {
        if (!charCounts.at(i).isEmpty())
            continue;

        if (contiguousEmptyColumns.isEmpty() || contiguousEmptyColumns.last().second != i-1)
            contiguousEmptyColumns.append(qMakePair(i, i));
        else // This gap column is contiguous with the previous all gap column. Increment the last all gap range column.
            ++contiguousEmptyColumns.last().second;
    }

    for (int i=contiguousEmptyColumns.count()-1; i >= 0; --i)
    {
        int first = contiguousEmptyColumns.at(i).first;
        charCountDistribution_.remove(first+1, contiguousEmptyColumns.at(i).second - first + 1);
        emit columnsRemoved(first+1, contiguousEmptyColumns.at(i).second+1);
    }
}

/**
  * @param left [int]
  * @param top [int]
  * @param right [int]
  * @param bottom [int]
  * @param delta [int]
  * @param finalLeft [int]
  * @param finalRight [int]
  */
void LiveMsaCharCountDistribution::onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight)
{
    ASSERT(msa_);
    ASSERT(left > 0 && left <= right);
    ASSERT(right <= msa_->length());
    ASSERT(top > 0 && top <= bottom);
    ASSERT(bottom <= msa_->subseqCount());
    ASSERT(delta != 0);

    Q_UNUSED(delta);

    // Update the character count distribution specifically for the region that changed
    CharCountDistribution block(calculateMsaCharCountDistribution(*msa_, QRect(QPoint(finalLeft, top), QPoint(finalRight, bottom))));
    charCountDistribution_.subtract(block, left);
    charCountDistribution_.add(block, finalLeft);

    // Determine the full range affected by this slide operation
    int minLeft = qMin(left, finalLeft);
    int maxRight = qMax(right, finalRight);

    charCountDistribution_.removeZeroValueKeys(minLeft, maxRight);

    emit dataChanged(minLeft, maxRight);
}

/**
  * At present, the only means for the msa to be reset is via the clear method, which removes all the member subseqs.
  * Consequently, this distribution should be emptied. Emits the columnsRemoved signal if there is at least one column
  * to remove.
  */
void LiveMsaCharCountDistribution::onMsaReset()
{
    ASSERT(msa_);

    int oldWidth = charCountDistribution_.length();
    if (!oldWidth)
        return;

    charCountDistribution_ = CharCountDistribution();
    emit columnsRemoved(1, oldWidth);
}

/**
  * @param subseqIndex [int]
  * @param column [int]
  * @param extension [const QString &]
  */
void LiveMsaCharCountDistribution::onMsaSubseqExtended(int /* subseqIndex */, int column, const QString &extension)
{
    ASSERT(msa_);
    ASSERT_X(column > 0 && column <= msa_->length(), "column out of range");
    ASSERT_X(column + extension.length() - 1 <= msa_->length(), "extension out of range");

    charCountDistribution_.add(BioString(extension).substituteGapsWith(' '), ' ', column);

    int to = column + extension.length() - 1;
    charCountDistribution_.removeZeroValueKeys(column, to);

    emit dataChanged(column, to);
}

/**
  * @param subseqIndex [int]
  * @param column [int]
  * @param newSubSequence [const QString &]
  * @param oldSubSequence [const QString &]
  */
void LiveMsaCharCountDistribution::onMsaSubseqInternallyChanged(int /* subseqIndex */, int column, const QString &newSubSequence, const QString &oldSubSequence)
{
    ASSERT(msa_);
    ASSERT_X(column > 0 && column <= msa_->length(), "column out of range");
    ASSERT_X(newSubSequence.length() == oldSubSequence.length(), "newSubSequence.length() does not equal oldSubSequence.length()");
    ASSERT_X(column + newSubSequence.length() - 1 <= msa_->length(), "sequence data out of range");

    charCountDistribution_.subtract(BioString(oldSubSequence).substituteGapsWith(' '), ' ', column);
    charCountDistribution_.add(BioString(newSubSequence).substituteGapsWith(' '), ' ', column);

    int to = column + newSubSequence.length() - 1;
    charCountDistribution_.removeZeroValueKeys(column, to);

    emit dataChanged(column, to);
}


/**
  * @param subseqIndex [int]
  * @param column [int]
  * @param trimmings [const QString &]
  */
void LiveMsaCharCountDistribution::onMsaSubseqTrimmed(int /* subseqIndex */, int column, const QString &trimmings)
{
    ASSERT(msa_);
    ASSERT_X(column > 0 && column <= msa_->length(), "column out of range");
    ASSERT_X(column + trimmings.length() - 1 <= msa_->length(), "extension out of range");

    charCountDistribution_.subtract(BioString(trimmings).substituteGapsWith(' '), ' ', column);

    int to = column + trimmings.length() - 1;
    charCountDistribution_.removeZeroValueKeys(column, column + trimmings.length() - 1);

    emit dataChanged(column, to);
}

/**
  * Two possibilities:
  * o Msa did not have any sequences beforehand in which case the distribution would have been empty. In this case,
  *   simply calculate the distribution for the entire alignment and assign this to the internal charCountDistribution_.
  *   Emit columnsInserted signal
  * o Msa already had some sequences, calculate the distribution of the new sequences and add this to the existing
  *   values. Emits dataChanged signal for across the entire alignment
  *
  * @param start [int]
  * @param end [int]
  */
void LiveMsaCharCountDistribution::onMsaSubseqsInserted(int start, int end)
{
    ASSERT(msa_);
    ASSERT(start >= 1);
    ASSERT(start <= end);
    ASSERT(end <= msa_->subseqCount());

    if (charCountDistribution_.length())
    {
        CharCountDistribution difference = calculateMsaCharCountDistribution(*msa_, QRect(1, start, msa_->length(), end - start + 1));
        charCountDistribution_.add(difference);
        charCountDistribution_.removeZeroValueKeys(start, end);
        emit dataChanged(1, msa()->length());
    }
    else
    {
        ASSERT_X(start == 1 && end == msa_->subseqCount(), "if distribution is empty, start and end must cover all sequences in msa");

        charCountDistribution_ = calculateMsaCharCountDistribution(*msa_);
        emit columnsInserted(1, msa_->length());
    }
}

/**
  * Two possibilities:
  * o Msa will no longer have any sequences and the resulting distribution should be empty. Simply assign an empty value
  *   to the internal charCountDistribution_. Emit columnsRemoved signal for the relevant columns
  * o Msa will still have sequences left - calculate the distribution for those sequences being removed and subtract
  *   this value from the internal distribution. Emit dataChanged signals.
  *
  * OPTIMIZATION: If more than half are being removed, it is more efficient to recompute the distribution for the
  *               remaining members rather than calculate the distribution for those being removed and subtract this
  *               value from the internal distribution.
  *
  * @param start [int]
  * @param end [int]
  */
void LiveMsaCharCountDistribution::onMsaSubseqsAboutToBeRemoved(int start, int end)
{
    ASSERT(msa_);
    ASSERT(start >= 1);
    ASSERT(start <= end);
    ASSERT(end <= msa_->subseqCount());

    if (start == 1 && end == msa_->subseqCount())
    {
        charCountDistribution_ = CharCountDistribution();
        emit columnsRemoved(1, msa_->length());
    }
    else
    {
        CharCountDistribution difference = calculateMsaCharCountDistribution(*msa_, QRect(1, start, msa_->length(), end - start + 1));
        charCountDistribution_.subtract(difference);
        charCountDistribution_.removeZeroValueKeys(start, end);
        emit dataChanged(1, msa_->length());
    }
}
