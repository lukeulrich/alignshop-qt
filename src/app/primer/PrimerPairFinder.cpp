/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "PrimerPairFinder.h"
#include "PrimerFactory.h"
#include "PrimerPairFactory.h"
#include "PrimerSearchParameters.h"
#include "ThermodynamicCalculator.h"
#include "../core/DnaPattern.h"
#include "../core/macros.h"

#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  */
PrimerPairFinder::PrimerPairFinder()
    : canceled_(false)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void PrimerPairFinder::cancel()
{
    canceled_ = true;
}

/**
  * @param dnaString [const BioString &]
  * @param range [const ClosedIntRange &]
  * @param primerSearchParameters [const PrimerSearchParameters &]
  * @returns QVector<PrimerPair>
  */
QVector<PrimerPair> PrimerPairFinder::findPrimerPairs(const BioString &dnaString, const ClosedIntRange &range, const PrimerSearchParameters &primerSearchParameters)
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(dnaString.isValidRange(range));
    ASSERT(!dnaString.hasGaps());
    ASSERT(primerSearchParameters.isValid());
    // The entire amplicon range must fit within the requested range
    ASSERT(primerSearchParameters.ampliconLengthRange_.end_ <= range.length());

    canceled_ = false;
    primerSearchParameters_ = primerSearchParameters;

    QVector<ClosedIntRange> acgtRanges = findACGTRangesWithin(dnaString, range);
    if (acgtRanges.isEmpty())
        return QVector<PrimerPair>();

    int absMaxPrimerStart = absoluteMaxPrimerStart(range);
    QVector<LitePrimer> forwardLitePrimers;
    QVector<LitePrimer> reverseLitePrimers;

    foreach (const ClosedIntRange &acgtRange, acgtRanges)
    {
        // Skip all ranges that are less than the minimum primer length
        if (rangeIsLessThanMinimumPrimerLength(acgtRange))
            continue;

        forwardLitePrimers << findCompatibleLitePrimers(dnaString,
                                                        acgtRange,
                                                        absMaxPrimerStart,
                                                        primerSearchParameters.forwardRestrictionEnzyme_,
                                                        primerSearchParameters.forwardTerminalPattern_);

        // Invert the range for the reverse direction
        ClosedIntRange reverseRange(dnaString.length() - acgtRange.end_ + 1,
                                    dnaString.length() - acgtRange.begin_ + 1);
        reverseLitePrimers << findCompatibleLitePrimers(dnaString.reverseComplement(),
                                                        reverseRange,
                                                        absMaxPrimerStart,
                                                        primerSearchParameters.reverseRestrictionEnzyme_,
                                                        primerSearchParameters.reverseTerminalPattern_);
        if (canceled_)
            return QVector<PrimerPair>();
    }

    if (forwardLitePrimers.isEmpty() || reverseLitePrimers.isEmpty())
        return QVector<PrimerPair>();

    return findCompatiblePrimerPairs(forwardLitePrimers,
                                     reverseLitePrimers,
                                     dnaString);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param range [const ClosedIntRange &]
  * @param minAmpliconLength [const int]
  * @returns int
  */
int PrimerPairFinder::absoluteMaxPrimerStart(const ClosedIntRange &range) const
{
    int minAmpliconLength = primerSearchParameters_.ampliconLengthRange_.begin_;
    return range.length() - minAmpliconLength + 1;
}

bool PrimerPairFinder::rangeIsLessThanMinimumPrimerLength(const ClosedIntRange &range) const
{
    return range.length() < primerSearchParameters_.primerLengthRange_.begin_;
}

/**
  * @param dnaString [const BioString &]
  * @param range [const ClosedIntRange &]
  * @returns QVector<ClosedIntRange>
  */
QVector<ClosedIntRange> PrimerPairFinder::findACGTRangesWithin(const BioString &dnaString, const ClosedIntRange &range) const
{
    QVector<ClosedIntRange> acgtRanges;

    const char *x = dnaString.constData() + range.begin_ - 1;
    for (int i=range.begin_; i<= range.end_; ++i, ++x)
    {
        if (!isACGT(*x))
            continue;

        if (acgtRanges.isEmpty() || acgtRanges.last().end_ + 1 != i)
            acgtRanges << ClosedIntRange(i, i);
        else
            ++acgtRanges.last().end_;
    }

    return acgtRanges;
}

/**
  * Note, only dnaString.mid(range) is searched for compatible primers; however, the entire dnaString is searched for
  * uniqueness constraints.
  *
  * dnaString must be in the 5' -> 3' orientation! Similarly, range must also be relevant to the 5' -> 3' direction.
  *
  * @param dnaString [const BioString &]
  * @param range [const ClosedIntRange &]
  * @param absoluteMaxPrimerStart [const int]
  * @param primerLengthRange [const ClosedIntRange &]
  * @param restrictionEnzyme [const RestrictionEnzyme &]
  * @param tmRange [const RangeF &]
  * @param terminalPattern [const DnaPattern &]
  * @param sodiumConcentration [const double]
  * @param primerDnaConcentration [const double]
  * @returns QVector<LitePrimer>
  */
QVector<PrimerPairFinder::LitePrimer>
PrimerPairFinder::findCompatibleLitePrimers(const BioString &dnaString,
                                            const ClosedIntRange &range,
                                            const int absoluteMaxPrimerStart,
                                            const RestrictionEnzyme &restrictionEnzyme,
                                            const DnaPattern &terminalPattern)
{
    const ClosedIntRange &primerLengthRange = primerSearchParameters_.primerLengthRange_;
    const RangeF &tmRange = primerSearchParameters_.individualPrimerTmRange_;
    const double sodiumConcentration = primerSearchParameters_.sodiumConcentration_;
    const double primerDnaConcentration = primerSearchParameters_.primerDnaConcentration_;

    BioString fullRCString = dnaString.reverseComplement(); // Used for searching for unique primers
    BioString searchString = dnaString.mid(range);

    // Translation is the amount to add to both positions of a compatible primer to map its coordinates back to
    // the original sequence (assuming 5' -> 3').
    int translation = range.begin_ - 1;

    ThermodynamicCalculator thermodynamicCalculator;
    int reSiteLength = restrictionEnzyme.recognitionSite().length();
    bool hasEndPattern = !terminalPattern.isEmpty();

    QVector<LitePrimer> compatiblePrimers;
    for (int primerLength = primerLengthRange.begin_; !canceled_ && primerLength <= primerLengthRange.end_; ++ primerLength)
    {
        QByteArray primerSeq;
        primerSeq.resize(reSiteLength + primerLength);
        char *x = primerSeq.data();
        if (reSiteLength > 0)
        {
            qstrcpy(x, restrictionEnzyme.recognitionSite().constData());
            x += reSiteLength;
        }

        int localMaxPrimerStart = qMin(absoluteMaxPrimerStart, range.length() - primerLength);
        for (int j=1; !canceled_ && j<= localMaxPrimerStart; ++j)
        {
            // Copy the relevant sequence characters from the search string to the working primer string
            qMemCopy(x, searchString.constData() + j - 1, primerLength);

            BioString primerString(primerSeq, eDnaGrammar);

            if (hasEndPattern && !terminalPattern.matchesAtEnd(primerString))
                continue;

            double tm = thermodynamicCalculator.meltingTemperature(primerString, sodiumConcentration, primerDnaConcentration);
            if (!tmRange.contains(tm))
                continue;

            // Make sure the core primer sequence only occurs once in both strands
            if (dnaString.count(x) + fullRCString.count(x) != 1)
                continue;

            compatiblePrimers << LitePrimer(tm, ClosedIntRange(j + translation, (j + primerLength - 1) + translation));
        }
    }

    // Each of the locations in LitePrimer is with respect to searchString - not the original dnaString (unless the
    // range spanned the entire dnaString).
    return compatiblePrimers;
}

/**
  * @param forwardPrimers [const QVector<LitePrimer> &]
  * @param reversePrimers [const QVector<LitePrimer> &]
  * @param dnaString [const BioString &]
  * @param forwardRestrictionEnzyme [const RestrictionEnzyme &]
  * @param reverseRestrictionEnzyme [const RestrictionEnzyme &]
  * @param ampliconLengthRange [const ClosedIntRange &]
  * @param maximumDeltaTm [const double]
  * @returns QVector<PrimerPair>
  */
QVector<PrimerPair> PrimerPairFinder::findCompatiblePrimerPairs(const QVector<LitePrimer> &forwardPrimers,
                                                                const QVector<LitePrimer> &reversePrimers,
                                                                const BioString &dnaString)
{
    const RestrictionEnzyme &forwardRestrictionEnzyme = primerSearchParameters_.forwardRestrictionEnzyme_;
    const RestrictionEnzyme &reverseRestrictionEnzyme = primerSearchParameters_.reverseRestrictionEnzyme_;
    const ClosedIntRange &ampliconLengthRange = primerSearchParameters_.ampliconLengthRange_;
    const double maximumDeltaTm = primerSearchParameters_.maximumPrimerPairDeltaTm_;

    PrimerFactory primerFactory;
    QSharedPointer<PrimerSearchParameters> searchParameters(new PrimerSearchParameters(primerSearchParameters_));
    primerFactory.setPrimerSearchParameters(searchParameters);
    PrimerPairFactory primerPairFactory;
    QVector<PrimerPair> compatiblePrimerPairs;
    foreach (const LitePrimer &forwardLitePrimer, forwardPrimers)
    {
        foreach (const LitePrimer &reverseLitePrimer, reversePrimers)
        {
            if (canceled_)
                return compatiblePrimerPairs;

            // Important to note that the reverseLitePrimer's coordinates are reversed!
            // Check 1: is the difference in tm's significant?
            double deltaTm = qAbs(forwardLitePrimer.tm_ - reverseLitePrimer.tm_);
            if (deltaTm > maximumDeltaTm)
                continue;

            // Normalize the locations of the reverse primer sequence
            ClosedIntRange reverseSenseLocation(dnaString.length() - reverseLitePrimer.location_.end_ + 1,
                                                dnaString.length() - reverseLitePrimer.location_.begin_ + 1);

            // Check 2: Do the primer location's amplify a region that falls within the acceptable range
            int ampliconSize = reverseSenseLocation.end_ - forwardLitePrimer.location_.begin_ + 1;
            if (!ampliconLengthRange.contains(ampliconSize))
                continue;

            // Check 3: Do these primers overlap at all?
            if (forwardLitePrimer.location_.end_ >= reverseSenseLocation.begin_)
                continue;

            // All good to go, make the primer pair
            BioString forwardPrimerSequence = dnaString.mid(forwardLitePrimer.location_);
            BioString reversePrimerSequence = dnaString.mid(reverseSenseLocation).reverseComplement();
            Primer forwardPrimer = primerFactory.makePrimer(forwardPrimerSequence, forwardRestrictionEnzyme, forwardLitePrimer.tm_);
            Primer reversePrimer = primerFactory.makePrimer(reversePrimerSequence, reverseRestrictionEnzyme, reverseLitePrimer.tm_);
            compatiblePrimerPairs << primerPairFactory.makePrimerPair(forwardPrimer, reversePrimer);
        }
    }

    return compatiblePrimerPairs;
}

/**
  * @param nucleotide [const char]
  * @returns bool
  */
bool PrimerPairFinder::isACGT(const char nucleotide) const
{
    return nucleotide == 'A' ||
           nucleotide == 'C' ||
           nucleotide == 'G' ||
           nucleotide == 'T';
}
