/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef LIVEMSACHARCOUNTDISTRIBUTION_H
#define LIVEMSACHARCOUNTDISTRIBUTION_H

#include "LiveCharCountDistribution.h"

#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;

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
class LiveMsaCharCountDistribution : public LiveCharCountDistribution
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    LiveMsaCharCountDistribution(const Msa *msa, QObject *parent = 0);  //!< Calculate the character count distribution for msa and associate with parent. Also hooks up signals to react to MSA mutations

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int divisor() const;                                                //!< Returns the divisor for this character count distribution, which corresponds to the number of sequences in this msa
    const Msa *msa() const;                                             //!< Returns the msa actively being watched
    CharCountDistribution charCountDistribution() const;                //!< Returns the current character count distribution

private slots:
    void onMsaGapColumnsInserted(int column, int count);
    void onMsaGapColumnsRemoved();
    void onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight);
    void onMsaReset();
    void onMsaSubseqExtended(int subseqIndex, int column, const QString &extension);
    void onMsaSubseqInternallyChanged(int subseqIndex, int column, const QString &newSubSequence, const QString &oldSubSequence);
    void onMsaSubseqTrimmed(int subseqIndex, int column, const QString &trimmings);
    void onMsaSubseqsInserted(int start, int end);
    void onMsaSubseqsAboutToBeRemoved(int start, int end);

private:
    const Msa *msa_;
    CharCountDistribution charCountDistribution_;
};

#endif // LIVEMSACHARCOUNTDISTRIBUTION_H
