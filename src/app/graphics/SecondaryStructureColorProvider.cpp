/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SecondaryStructureColorProvider.h"
#include "../core/Subseq.h"
#include "../core/Entities/AminoSeq.h"
#include "../core/Entities/Astring.h"
#include "../core/global.h"
#include "../core/misc.h"
#include "../core/Msa.h"
#include "../core/PODs/Q3Prediction.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param linearColorScheme [const LinearColorScheme &]
  */
SecondaryStructureColorProvider::SecondaryStructureColorProvider(const LinearColorScheme &linearColorScheme)
    : PositionalMsaColorProvider(),
      linearColorScheme_(linearColorScheme)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param msa [const Msa &]
  * @param row [int]
  * @param column [int]
  * @returns TextColorStyle
  */
TextColorStyle SecondaryStructureColorProvider::color(const Msa &msa, int row, int column) const
{
    const Subseq *subseq = msa.at(row);
    char character = subseq->at(column);

    // Extract the secondary structure prediction
    ASSERT(boost::shared_dynamic_cast<AminoSeq>(subseq->seqEntity_));
    const AminoSeqSPtr &aminoSeq = boost::shared_static_cast<AminoSeq>(subseq->seqEntity_);
    ASSERT(aminoSeq);
    const AstringSPtr astring = aminoSeq->abstractAnonSeq();
    ASSERT(astring);

    const Q3Prediction q3 = astring->q3();
    if (q3.isEmpty() == false)
    {
        // Determine the character index within the astring
        int seqPos = subseq->mapToSeq(column);
        if (seqPos != -1)
            return linearColorScheme_.textColorStyle(q3.q3_.at(seqPos-1), q3.confidence_.at(seqPos-1));
    }

    return linearColorScheme_.textColorStyle(character);
}

/**
  * @param msa [const Msa &]
  * @param row [int]
  * @param columns [const ClosedIntRange &]
  * @returns QVector<TextColorStyle>
  */
QVector<TextColorStyle> SecondaryStructureColorProvider::colors(const Msa &msa, int row, const ClosedIntRange &columns) const
{
    QVector<TextColorStyle> styles;
    styles.reserve(columns.length());

    // Extract the secondary structure prediction
    const Subseq *subseq = msa.at(row);
    ASSERT(boost::shared_dynamic_cast<AminoSeq>(subseq->seqEntity_));
    const AminoSeqSPtr &aminoSeq = boost::shared_static_cast<AminoSeq>(subseq->seqEntity_);
    ASSERT(aminoSeq);
    const AstringSPtr &astring = aminoSeq->abstractAnonSeq();
    ASSERT(astring);

    const char *ch = subseq->constData() + columns.begin_ - 1;
    const Q3Prediction q3 = astring->q3();
    if (q3.isEmpty() == false)
    {
        const char *q3_ptr = nullptr;
        const double *conf_ptr = nullptr;

        for (int i=columns.begin_; i<=columns.end_; ++i, ++ch)
        {
            if (::isGapCharacter(*ch))
            {
                styles << linearColorScheme_.textColorStyle(*ch);
                continue;
            }

            // Now we know this is not a gap character
            if (q3_ptr != nullptr)
            {
                ++q3_ptr;
                ++conf_ptr;
            }
            else
            {
                int seqPos = subseq->mapToSeq(i);
                ASSERT(seqPos != -1);
                q3_ptr = q3.q3_.constData() + seqPos - 1;
                conf_ptr = q3.confidence_.constData() + seqPos - 1;
            }

            styles << linearColorScheme_.textColorStyle(*q3_ptr, *conf_ptr);
        }
    }
    else
    {
        for (int i=0, z=columns.length(); i<z; ++i, ++ch)
            styles << linearColorScheme_.textColorStyle(*ch);
    }

    return styles;
}
