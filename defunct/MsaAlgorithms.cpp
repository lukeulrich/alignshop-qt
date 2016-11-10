/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MsaAlgorithms.h"

#include "Msa.h"

#include "global.h"

/**
  * If region is null, then the distribution is computed for the entire msa. If region is non null, then it must be
  * valid, non-empty, and its values must fall within the boundaries of msa. Moreover, because Msa is a 1-based entity,
  * the values of region must also be one based.
  *
  * All gap characters (as defined by constants::kGapCharacters) are ignored.
  *
  * @param msa [const Msa &]
  * @param region [const QRect &]
  * @returns QList<QHash<char, int> > or ListHashCharInt
  */
ListHashCharInt calculateMsaCharCountDistribution(const Msa &msa, const QRect &region)
{
    QRect targetRegion = region;
    if (!targetRegion.isNull())
    {
        ASSERT(targetRegion.isValid());
        ASSERT(!targetRegion.isEmpty());
        ASSERT(targetRegion.left() > 0);
        ASSERT(targetRegion.top() > 0);
        ASSERT(targetRegion.right() <= msa.length());
        ASSERT(targetRegion.bottom() <= msa.subseqCount());
    }
    else
    {
        targetRegion = QRect(1, 1, msa.length(), msa.subseqCount());
    }

    ListHashCharInt charCounts;
    if (msa.subseqCount() == 0)
        return charCounts;

    // Initialize the character count hash for each column
    for (int i=0; i< targetRegion.width(); ++i)
        charCounts << QHash<char, int>();

    // Walk through each sequence and count the characters in each position
    for (int i=targetRegion.top(), z= targetRegion.bottom(); i<=z; ++i)
    {
        QByteArray characters = msa.at(i)->bioString().sequence().toAscii();
        for (int j=targetRegion.left(), y= targetRegion.right(); j<=y; ++j)
        {
            // Must subtract one from j because that is 1-based, but index into distribution is 0-based
            char ch = characters.at(j-1);
            if (!isGapCharacter(ch))
                ++charCounts[j - targetRegion.left()][ch];
        }
    }

    return charCounts;
}
