/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "LiveMsaCharCountDistribution.h"

#include "ObservableMsa.h"
#include "util/ClosedIntRange.h"
#include "util/MsaAlgorithms.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param msa [const Msa *]
  * @param parent [QObject *]
  */
LiveMsaCharCountDistribution::LiveMsaCharCountDistribution(const ObservableMsa *msa, QObject *parent)
    : AbstractLiveCharCountDistribution(parent), msa_(msa)
{
    if (msa_)
    {
        charCountDistribution_ = calculateMsaCharCountDistribution(*msa_);
//        charCountDistribution_.setDivisor(msa->rowCount());

        connect(msa_, SIGNAL(gapColumnsInserted(ClosedIntRange)), SLOT(onMsaGapColumnsInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)), SLOT(onMsaGapColumnsRemoved(QVector<ClosedIntRange>)));
        connect(msa_, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)), SLOT(onMsaRectangleSlid(PosiRect,int,ClosedIntRange)));
        connect(msa_, SIGNAL(msaReset()), SLOT(onMsaReset()));
        connect(msa_, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)), SLOT(onMsaRowsAboutToBeRemoved(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsRemoved(ClosedIntRange)), SLOT(onMsaRowsRemoved(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsInserted(ClosedIntRange)), SLOT(onMsaRowsInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)), SLOT(onMsaSubseqsChanged(SubseqChangePodVector)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns const ObservableMsa *
  */
const ObservableMsa *LiveMsaCharCountDistribution::msa() const
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
  * @param gapColumns [const ClosedIntRange &]
  */
void LiveMsaCharCountDistribution::onMsaGapColumnsInserted(const ClosedIntRange &gapColumns)
{
    ASSERT(msa_);
    ASSERT(gapColumns.begin_ >= 1 && gapColumns.begin_ <= msa_->length());
    ASSERT(gapColumns.end_ >= gapColumns.begin_ && gapColumns.end_ <= msa_->length());

    charCountDistribution_.insertBlanks(gapColumns.begin_, gapColumns.length());
    emit columnsInserted(gapColumns);
}

/**
  * This slot is called whenever the MSA has had some of its completely gap columns removed.
  *
  * If there is at least one empty column to remove, emit the appropriate columnsRemoved signal.
  */
void LiveMsaCharCountDistribution::onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &gapColumnRanges)
{
    ASSERT(msa_);
    for (int i=gapColumnRanges.count()-1; i >= 0; --i)
    {
        charCountDistribution_.remove(gapColumnRanges.at(i).begin_, gapColumnRanges.at(i).length());
        emit columnsRemoved(gapColumnRanges.at(i));
    }
}

/**
  * @param msaRect [const PosiRect &]
  * @param delta [int]
  * @param finalRange [const ClosedIntRange &]
  */
void LiveMsaCharCountDistribution::onMsaRectangleSlid(const PosiRect &msaRect,
                                                      int delta,
                                                      const ClosedIntRange &finalRange)
{
    ASSERT(msa_);
    ASSERT(delta != 0);

    Q_UNUSED(delta);

    PosiRect normalizedMsaRect = msaRect.normalized();
    ClosedIntRange horizontalRange = normalizedMsaRect.horizontalRange();
    if (horizontalRange.isEmpty())
        horizontalRange.invert();


    // Two stages:
    // 1) Update the counts by the displacing msa rect
    {
        // Update the character count distribution specifically for the region that changed
        PosiRect finalMsaRect(QPoint(finalRange.begin_, normalizedMsaRect.top()),
                              QPoint(finalRange.end_, normalizedMsaRect.bottom()));
        CharCountDistribution block = ::calculateMsaCharCountDistribution(*msa_, finalMsaRect);
        charCountDistribution_.subtract(block, horizontalRange.begin_);
        charCountDistribution_.add(block, finalRange.begin_);
    }

    // 2) Update the displaced counts
    {
        PosiRect displacedRect = normalizedMsaRect;
        int subtractOffset = 0;
        int addOffset = 0;
        if (delta > 0)
        {
            displacedRect.setRight(finalRange.begin_ - 1);
            subtractOffset = normalizedMsaRect.right() + 1;
            addOffset = normalizedMsaRect.left();
        }
        else // Negative delta
        {
            displacedRect.setLeft(finalRange.end_ + 1);
            subtractOffset = finalRange.begin_;
            addOffset = finalRange.end_ + 1;
        }
        CharCountDistribution displacedBlock = ::calculateMsaCharCountDistribution(*msa_, displacedRect);
        charCountDistribution_.subtract(displacedBlock, subtractOffset);
        charCountDistribution_.add(displacedBlock, addOffset);
    }

    // Determine the full range affected by this slide operation
    int minLeft = qMin(horizontalRange.begin_, finalRange.begin_);
    int maxRight = qMax(horizontalRange.end_, finalRange.end_);

    charCountDistribution_.removeZeroValueKeys(minLeft, maxRight);

    emit dataChanged(ClosedIntRange(minLeft, maxRight));
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
    emit columnsRemoved(ClosedIntRange(1, oldWidth));
}


/**
  * Two possibilities:
  * o Msa will no longer have any sequences and the resulting distribution should be empty. Simply assign an empty value
  *   to the internal charCountDistribution_. Emit columnsRemoved signal for the relevant columns
  * o Msa will still have sequences left - calculate the distribution for those sequences being removed and subtract
  *   this value from the internal distribution. Emit dataChanged signals.
  *
  * TODO:
  * OPTIMIZATION: If more than half are being removed, it is more efficient to recompute the distribution for the
  *               remaining members rather than calculate the distribution for those being removed and subtract this
  *               value from the internal distribution.
  *
  * @param const ClosedIntRange & [rows]
  */
void LiveMsaCharCountDistribution::onMsaRowsAboutToBeRemoved(const ClosedIntRange &rows)
{
    ASSERT(msa_);

    bool rowsSpansAllSequences = rows.begin_ == 1 && rows.end_ == msa_->rowCount();
    if (!rowsSpansAllSequences)
    {
        CharCountDistribution difference = ::calculateMsaCharCountDistribution(*msa_,
                                                                               PosiRect(QPoint(1,
                                                                                               rows.begin_),
                                                                                        QPoint(msa_->length(),
                                                                                               rows.end_)));
        charCountDistribution_.subtract(difference);
        charCountDistribution_.removeZeroValueKeys();
    }
    else
    {
        charCountDistribution_ = CharCountDistribution();

        // We emit this signal here while we can still access the length of the msa
        emit columnsRemoved(ClosedIntRange(1, msa_->length()));
    }
}

/**
  * Two possibilities:
  * o Msa did not have any sequences beforehand in which case the distribution would have been empty. In this case,
  *   simply calculate the distribution for the entire alignment and assign this to the internal charCountDistribution_.
  *   Emit columnsInserted signal
  * o Msa already had some sequences, calculate the distribution of the new sequences and add this to the existing
  *   values. Emits dataChanged signal for across the entire alignment
  *
  * @param rows [const ClosedIntRange &]
  */
void LiveMsaCharCountDistribution::onMsaRowsInserted(const ClosedIntRange &rows)
{
    ASSERT(msa_);
    // It is vital to update the divisor **before** emitting the data changed signal because downstream components (e.g.
    // information content will utilize the divisor in response to this signal.
    charCountDistribution_.setDivisor(msa_->rowCount());

    if (charCountDistribution_.length())
    {
        CharCountDistribution difference =
                ::calculateMsaCharCountDistribution(*msa_,
                                                    PosiRect(QPoint(1, rows.begin_),
                                                             QPoint(msa_->length(), rows.end_)));
        charCountDistribution_.add(difference);
        charCountDistribution_.removeZeroValueKeys();
        emit dataChanged(ClosedIntRange(1, msa()->length()));
    }
    else
    {
        ASSERT_X(rows.begin_ == 1 && rows.end_ == msa_->rowCount(),
                 "if distribution is empty, rows must cover all sequences in msa");

        charCountDistribution_ = calculateMsaCharCountDistribution(*msa_);
        emit columnsInserted(ClosedIntRange(1, msa_->length()));
    }
}

/**
  * @param rows [const ClosedIntRange &]
  */
void LiveMsaCharCountDistribution::onMsaRowsRemoved(const ClosedIntRange & /* rows */)
{
    // It is vital to update the divisor **before** emitting the data changed signal because downstream components (e.g.
    // information content will utilize the divisor in response to this signal.
    charCountDistribution_.setDivisor(msa_->rowCount());

    if (msa_->rowCount() > 0)
        // The data changed signal is only emitted if at least one sequence remains in the msa
        emit dataChanged(ClosedIntRange(1, msa_->length()));
}

/**
  * @param subseqChangePods [const SubseqChangePodVector &]
  */
void LiveMsaCharCountDistribution::onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods)
{
    ASSERT(msa_);
    if (subseqChangePods.isEmpty())
        return;

    ClosedIntRange affectedColumns(msa_->length() + 1, 0);
    foreach (const SubseqChangePod &pod, subseqChangePods)
    {
        BioString difference = pod.difference_;
        QByteArray byteArray = difference.translateGaps(' ').asByteArray();
        switch (pod.operation_)
        {
        case SubseqChangePod::eExtendLeft:
        case SubseqChangePod::eExtendRight:
            charCountDistribution_.add(byteArray, ' ', pod.columns_.begin_);
            break;
        case SubseqChangePod::eTrimLeft:
        case SubseqChangePod::eTrimRight:
            charCountDistribution_.subtract(byteArray, ' ', pod.columns_.begin_);
            break;
        case SubseqChangePod::eInternal:
            charCountDistribution_.subtract(byteArray, ' ', pod.columns_.begin_);
            charCountDistribution_.add(msa_->at(pod.row_)->mid(pod.columns_).translateGaps(' ').asByteArray(),
                                       ' ',
                                       pod.columns_.begin_);
            break;

        default:
            return;
        }

        if (pod.columns_.begin_ < affectedColumns.begin_)
            affectedColumns.begin_ = pod.columns_.begin_;
        if (pod.columns_.end_ > affectedColumns.end_)
            affectedColumns.end_ = pod.columns_.end_;
    }

    charCountDistribution_.removeZeroValueKeys(affectedColumns.begin_, affectedColumns.end_);

    emit dataChanged(affectedColumns);
}
