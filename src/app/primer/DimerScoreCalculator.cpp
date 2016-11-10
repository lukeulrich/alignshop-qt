/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "DimerScoreCalculator.h"
#include "../core/BioString.h"
#include "../core/macros.h"

static const double kStandardPrimerLength = 10.;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaString1 [const BioString &]
  * @param dnaString2 [const BioString &]
  * @returns double
  */
double DimerScoreCalculator::dimerScore(const BioString &dnaStringA, const BioString &dnaStringB) const
{
    ASSERT(dnaStringA.grammar() == eDnaGrammar);
    ASSERT(dnaStringB.grammar() == eDnaGrammar);

    // If one or the other input sequences is empty, it is not possible to dimerize
    if (dnaStringA.isEmpty() || dnaStringB.isEmpty())
        return 0.;

    int hydrogenBonds = maximumHydrogenBonds(dnaStringA, dnaStringB);
    int shorterPrimerLength = qMin(dnaStringA.length(), dnaStringB.length());

    return dimerScore(hydrogenBonds, shorterPrimerLength);
}

/**
  * @param hydrogenBonds [const int]
  * @param shorterPrimerLength [const int]
  * @returns double
  */
double DimerScoreCalculator::dimerScore(const int hydrogenBonds, const int shorterPrimerLength) const
{
    ASSERT(hydrogenBonds >= 0);
    ASSERT(shorterPrimerLength > 0);

    return hydrogenBonds * kStandardPrimerLength / shorterPrimerLength;
}

/**
  * @param dnaString [const BioString &]
  * @returns double
  */
double DimerScoreCalculator::homoDimerScore(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);

    return dimerScore(dnaString, dnaString);
}

PairwiseHydrogenBondCount DimerScoreCalculator::locateMaximumHydrogenBonds(const BioString &dnaStringA, const BioString &dnaStringB) const
{
    ASSERT(dnaStringA.grammar() == eDnaGrammar);
    ASSERT(dnaStringB.grammar() == eDnaGrammar);

    BioString threeToFiveString = dnaStringB;
    threeToFiveString.reverse();

    int aOffset = threeToFiveString.length();
    int finalSlidePosition = dnaStringA.length() + threeToFiveString.length() - 1;

    int maxHydrogenBonds = 0;
    PairwiseHydrogenBondCount result;
    for (int i=1; i< finalSlidePosition; ++i)
    {
        int hydrogenBonds = 0;
        for (int j=i; j< i + threeToFiveString.length(); ++j)
        {
            int aIndex = j - aOffset + 1;
            int bIndex = j - i + 1;     // Add one to account for 1-based values

            if (aIndex < 1 || aIndex > dnaStringA.length())
                continue;

            hydrogenBonds += hydrogenBondsBetween(dnaStringA.at(aIndex), threeToFiveString.at(bIndex));
        }

        if (hydrogenBonds > maxHydrogenBonds)
        {
            result.queryRange_.begin_ = qMax(1, i - aOffset + 1);
            result.queryRange_.end_ = qMin(dnaStringA.length(), i + dnaStringB.length() - aOffset);

            result.subjectRange_.begin_ = qMax(1, aOffset - i + 1);
            result.subjectRange_.end_ = qMin(dnaStringB.length(), aOffset + dnaStringA.length() - 1 - i + 1);

            maxHydrogenBonds = hydrogenBonds;
        }
    }

    result.hydrogenBonds_ = maxHydrogenBonds;

    return result;
}

/**
  * The sliding window algorithm works as follows:
  *
  * Given dnaStringA: 5' ATATG 3'
  *       dnaStringB: 5' ATATG 3' (homodimer test, but doesn't matter for our purposes)
  *
  * threeToFiveString = 3' GTATA 5'
  *
  * Position dnaStringA such that it only overlaps with one character in threeToFiveString:
  *
  * 1234567890123
  *     ATATG         [i = 1]
  * GTATA
  *
  *     ATATG         [i = 2]
  *  GTATA
  *
  *     ATATG         [i = 3]
  *   GTATA
  *
  * ...
  *
  *     ATATG         [i = 9] (finalSlidePosition)
  *         GTATA
  *
  * The threeToFiveString will progressively slide along the "fixed" dnaStringA, which is virtually positioned at the
  * fifth column. A for loop then iterates over all characters in threeToFiveString and if there is a valid character in
  * both sequences, sums any potential hydrogen bonds it may form to the existing score.
  *
  * @param dnaString1 [const BioString &]
  * @param dnaString2 [const BioString &]
  * @returns int
  */
int DimerScoreCalculator::maximumHydrogenBonds(const BioString &dnaStringA, const BioString &dnaStringB) const
{
    ASSERT(dnaStringA.grammar() == eDnaGrammar);
    ASSERT(dnaStringB.grammar() == eDnaGrammar);

    BioString threeToFiveString = dnaStringB;
    threeToFiveString.reverse();

    int aOffset = threeToFiveString.length();
    int finalSlidePosition = dnaStringA.length() + threeToFiveString.length() - 1;

    int maxHydrogenBonds = 0;

    for (int i=1; i< finalSlidePosition; ++i)
    {
        int hydrogenBonds = 0;
        for (int j=i; j< i + threeToFiveString.length(); ++j)
        {
            int aIndex = j - aOffset + 1;
            int bIndex = j - i + 1;     // Add one to account for 1-based values

            if (aIndex < 1 || aIndex > dnaStringA.length())
                continue;

            hydrogenBonds += hydrogenBondsBetween(dnaStringA.at(aIndex), threeToFiveString.at(bIndex));
        }

        if (hydrogenBonds > maxHydrogenBonds)
            maxHydrogenBonds = hydrogenBonds;
    }

    return maxHydrogenBonds;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param nucleotide [const char]
  * @returns char
  */
char DimerScoreCalculator::complement(const char nucleotide) const
{
    switch (nucleotide)
    {
    case 'A':
    case 'a':
        return 'T';
    case 'C':
    case 'c':
        return 'G';
    case 'G':
    case 'g':
        return 'C';
    case 'T':
    case 't':
        return 'A';

    default:
        return '?';
    }
}

/**
  * @param nucleotide1 [const char]
  * @param nucleotide2 [const char]
  * @returns int
  */
int DimerScoreCalculator::hydrogenBondsBetween(const char nucleotide1, const char nucleotide2) const
{
    if (nucleotide1 == complement(nucleotide2))
        return potentialHydrogenBonds(nucleotide1);

    return 0;
}

/**
  * @param nucleotide [const char]
  * @returns int
  */
int DimerScoreCalculator::potentialHydrogenBonds(const char nucleotide) const
{
    switch (nucleotide)
    {
    case 'A':
    case 'a':
    case 'T':
    case 't':
        return 2;
    case 'C':
    case 'c':
    case 'G':
    case 'g':
        return 3;

    default:
        return 0;
    }
}
