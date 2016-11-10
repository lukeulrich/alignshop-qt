/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef LIVEMSACHARCOUNTDISTRIBUTION_H
#define LIVEMSACHARCOUNTDISTRIBUTION_H

#include "AbstractLiveCharCountDistribution.h"

#include "PODs/SubseqChangePod.h"
#include "util/PosiRect.h"
#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class ObservableMsa;

/**
  * LiveMsaCharCountDistribution encapsulates and maintains a current representation of the character count distribution
  * within a MSA.
  *
  * If a valid msa is provided upon construction, its character count distribution is immediately computed and stored
  * as a private member. Moreover, the distribution is updated whenever the MSA is modified in such a way as to change
  * the character count distribution.
  *
  * Automatically removes zero value keys as distribution is updated.
  */
class LiveMsaCharCountDistribution : public AbstractLiveCharCountDistribution
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Calculate the character count distribution for msa
    LiveMsaCharCountDistribution(const ObservableMsa *msa, QObject *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const ObservableMsa *msa() const;                                   //!< Returns the msa actively being observed
    CharCountDistribution charCountDistribution() const;                //!< Returns the current character count distribution

private slots:
    // No longer update via the collapse signals, but rather via the subseqsChanged signal
//    void onMsaAboutToCollapse(const PosiRect &msaRect);
//    void onMsaCollapsed(const PosiRect &msaRect);
    void onMsaGapColumnsInserted(const ClosedIntRange &gapColumns);
    void onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &gapColumnRanges);
    void onMsaRectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange);
    void onMsaReset();
    void onMsaRowsAboutToBeRemoved(const ClosedIntRange &rows);
    void onMsaRowsInserted(const ClosedIntRange &rows);
    void onMsaRowsRemoved(const ClosedIntRange &rows);
    void onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods);

private:
    const ObservableMsa *msa_;
    CharCountDistribution charCountDistribution_;
};

#endif // LIVEMSACHARCOUNTDISTRIBUTION_H
