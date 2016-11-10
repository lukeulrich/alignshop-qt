/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MsaAlgorithms.h"

#include "../Msa.h"
#include "../macros.h"
#include "../misc.h"

/**
  * If msaRect is null, then the distribution is computed for the entire msa. If region is non null, then it must be
  * valid, non-empty, and its values must fall within the boundaries of msa. Moreover, because Msa is a 1-based entity,
  * the values of region must also be one based.
  *
  * All gap characters (as defined by constants::kGapCharacters) are ignored.
  *
  * @param msa [const Msa &]
  * @param msaRect [const PosiRect &]
  * @returns CharCountDistribution
  */
CharCountDistribution calculateMsaCharCountDistribution(const Msa &msa, const PosiRect &msaRect)
{
    if (msa.isEmpty())
        return CharCountDistribution();

    PosiRect targetRect = msaRect.normalized();
    if (targetRect.isNull())
        targetRect = PosiRect(1, 1, msa.length(), msa.rowCount());

    ASSERT(targetRect.isValid());
    ASSERT(targetRect.left() > 0);
    ASSERT(targetRect.top() > 0);
    ASSERT(targetRect.right() <= msa.length());
    ASSERT(targetRect.bottom() <= msa.subseqCount());

    VectorHashCharInt charCounts;
    if (msa.subseqCount() == 0)
        return charCounts;

    // Initialize the character count hash for each column
    int w = targetRect.width();
    charCounts.resize(w);

    // Walk through each sequence and count the characters in each position
    for (int i=targetRect.top(), z= targetRect.bottom(); i<=z; ++i)
    {
        const char *x = msa[i]->constData() + targetRect.left() - 1;
        for (int k=0; k< w; ++k, ++x)
            if (!::isGapCharacter(*x))
                ++charCounts[k][*x];
    }

    return CharCountDistribution(charCounts, targetRect.height());
}
