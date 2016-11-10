/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef DIMERSCORECALCULATOR_H
#define DIMERSCORECALCULATOR_H

class BioString;
#include "../core/util/ClosedIntRange.h"

struct PairwiseHydrogenBondCount
{
    int hydrogenBonds_;
    ClosedIntRange queryRange_;
    ClosedIntRange subjectRange_;
};

/**
  * DimerScoreCalculator computes a dimerization score for homodimers and heterodimers.
  *
  * Currently restricted to the strict DNA character set, ACGT.
  */
class DimerScoreCalculator
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns the dimer score between dnaString1 and dnaString2 which should be in the 5' -> 3' direction
    double dimerScore(const BioString &dnaStringA, const BioString &dnaStringB) const;
    //! Returns the dimer score given hydrogenBonds and shorterPrimerLength
    double dimerScore(const int hydrogenBonds, const int shorterPrimerLength) const;
    double homoDimerScore(const BioString &dnaString) const;   //!< Computes the score of dnaString dimerizing to itself
    //! Returns the maximum number of hydrogen bonds that may form between dnaStringA and dnaStringB; both sequences should be oriented in the 5' -> 3' direction
    int maximumHydrogenBonds(const BioString &dnaStringA, const BioString &dnaStringB) const;


    // Untested!
    PairwiseHydrogenBondCount locateMaximumHydrogenBonds(const BioString &dnaStringA, const BioString &dnaStringB) const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    char complement(const char nucleotide) const;           //!< Returns the complementary base for nucleotide
    //! Returns the number of hydrogen bonds between nucleotide1 and nucleotide2
    int hydrogenBondsBetween(const char nucleotide1, const char nucleotide2) const;
    //! Returns the number of potential hydrogen bonds that nucleotide may form (when paired with its cognate nucleotide)
    int potentialHydrogenBonds(const char nucleotide) const;
};

#endif // DIMERSCORECALCULATOR_H
