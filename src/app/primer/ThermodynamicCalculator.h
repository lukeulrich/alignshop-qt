/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef THERMODYNAMICCALCULATOR_H
#define THERMODYNAMICCALCULATOR_H

class BioString;

/**
  * ThermodynamicCalculator estimates thermodynamic parameters (enthalpy, entropy, melting temperature/Tm) of DNA
  * sequences.
  *
  * Uses the nearest-neighbor algorithm to calculate enthalpy and entropy characteristics of a given sequence.
  * Nearest-neighbor thermodynamic values were taken from Santa Lucia(1998) and are identical to those presented
  * by Allawi and Santa Lucia (1997).
  *
  * Constraints:
  * All input sequences must only contain ATCG. Degenerate sequences in any form are not supported.
  *
  * Definitions:
  * o Palindrome - ungapped dna sequence which has the same sequence as its reverse complement
  * o Inverted repeat - a palindrome that is split in the middle by one ore more unpaired bases
  * o Complementary - the complement of a dna sequence
  * o Self-complementary - synonym for palindrome
  * o Symmetrical - synonym for palindrome
  *
  * In the context of primer design, palindromes do not make good primers because they would stick to itself rather than
  * the target amplicon.
  *
  * Two cases:
  * 1) Input sequence is not a palindrome (most frequent case). The melting temperature is calculated with the
  *    following:
  *
  *    Tm = 1000 cal kcal-1 * H°[1 M Na+] / (S°[x M Na+] + R ln (C/2)) - 273.15
  *
  *    Where:
  *    1) H°[1 M Na+]: sum of enthalpy values for all nearest neighbor pairs (dimers) and terminal monomers at a sodium
  *       concentration of 1 molar
  *    2) S°[x M Na+]: salt corrected sum of entropy values for all nearest neighbor pairs (dimers) and terminal
  *       monomers at a sodium concentration of x molar.
  *
  *       This value may be derived from the entropy values at 1 molar as follows:
  *
  *       S°[x M Na+] = S°[1 M Na+] + 0.368 * (N-1) * ln [Na+]            (Santa Lucia, 1998)
  *
  *       where N = length of DNA sequence and [Na+] is the molar concentration of sodium.
  *
  *    3) R: universal gas constant, which equals 1.987 cal per Kelvin per mole
  *    4) C: molar concentration of primer DNA (typically 1 micromolar)
  *
  * 2) Input sequence is a palindrome (or self-complementary). In this case, the melting temperature is calculated with
  *    the following:
  *
  *    Tm = 1000 cal kcal-1 * H°[1 M Na+] / (S°[x M Na+] + R ln (C)) - 273.15
  */
class ThermodynamicCalculator
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    double enthalpy(const BioString &dnaString) const;          //!< Returns the total enthalpy (kcal per mole) of all dimers (also accounts for terminal monomers) within dnaString at a sodium concentration of 1 molar
    double enthalpy(const char nucleotide) const;               //!< Returns the enthalpy (kcal per mole) of an isolated nucleotide at a sodium concentration of 1 molar
    //! Returns the enthalpy (kcal per mole) of a dimer pair represented by upstreamNucleotide and downstreamNucleotide at a sodium concentration of 1 molar
    double enthalpy(const char upstreamNucleotide, const char downstreamNucleotide) const;

    double entropy(const BioString &dnaString) const;           //!< Returns the total entropy (calories per Kelvin per mole) of all dimers (also accounts for terminal monomers) within dnaString at a sodium concentration of 1 molar
    double entropy(const char nucleotide) const;                //!< Returns the entropy (calories per Kelvin per mole) of an isolated nucleotide at a sodium concentration of 1 molar
    //! Returns the entropy (calories per Kelvin per mole) of a dimer pair represented by upstreamNucleotide and downstreamNucleotide at a sodium concentration of 1 molar
    double entropy(const char upstreamNucleotide, const char downstreamNucleotide) const;

    //! Returns the melting temperature (degrees Celsius) of dnaString given sodiumConcentration (molar units) and primerDnaConcentration (molar units); returns 0 if dnaString is empty
    double meltingTemperature(const BioString &dnaString, const double sodiumConcentration, const double primerDnaConcentration) const;
    //! Returns the melting temperature (degrees Celsius) given sodiumCorrectedEntropy (calories per Kelvin per mole), primerDnaConcentration (molar units), and isPalindrome
    double meltingTemperature(const double enthalpy, const double sodiumCorrectedEntropy, const double primerDnaConcentration, bool isPalindrome) const;
    //! Returns the normalized value of entropy (sodium concentration of 1 molar) for nDimers and target sodiumConcentration
    double sodiumCorrectedEntropy(const double entropy, const int sequenceLength, const double sodiumConcentration) const;
};

#endif // THERMODYNAMICCALCULATOR_H
