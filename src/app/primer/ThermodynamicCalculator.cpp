/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include "ThermodynamicCalculator.h"

#include "../core/BioString.h"
#include "../core/macros.h"
#include "ThermodynamicConstants.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaString [const BioString &]
  * @returns double
  */
double ThermodynamicCalculator::enthalpy(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());

    if (dnaString.isEmpty())
        return 0.;

    if (dnaString.length() == 1)
        return enthalpy(dnaString.at(1));

    const char *x = dnaString.constData();
    double sum = enthalpy(*x);
    for (int i=0, z=dnaString.length()-1; i<z; ++i, ++x)
        sum += enthalpy(*x, *(x+1));
    sum += enthalpy(*x);

    if (dnaString.isPalindrome())
        sum += constants::kEnthalpySymmetryCorrection;

    return sum;
}

/**
  * @param nucleotide [const char]
  * @returns double
  */
double ThermodynamicCalculator::enthalpy(const char nucleotide) const
{
    switch(nucleotide)
    {
    case 'A':   return constants::kEnthalpyMonomerKcalPerMole[0];
    case 'C':   return constants::kEnthalpyMonomerKcalPerMole[1];
    case 'G':   return constants::kEnthalpyMonomerKcalPerMole[2];
    case 'T':   return constants::kEnthalpyMonomerKcalPerMole[3];

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(nucleotide).toAscii());
        return 0.;
    }
}

/**
  * @param upstreamNucleotide [const char]
  * @param downstreamNucleotide [const char]
  * @returns double
  */
double ThermodynamicCalculator::enthalpy(const char upstreamNucleotide, const char downstreamNucleotide) const
{
    int row = -1;
    switch (upstreamNucleotide)
    {
    case 'A':   row = 0; break;
    case 'C':   row = 1; break;
    case 'G':   row = 2; break;
    case 'T':   row = 3; break;

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(upstreamNucleotide).toAscii());
        return 0.;
    }

    int column = -1;
    switch (downstreamNucleotide)
    {
    case 'A':   column = 0; break;
    case 'C':   column = 1; break;
    case 'G':   column = 2; break;
    case 'T':   column = 3; break;

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(downstreamNucleotide).toAscii());
        return 0.;
    }

    return constants::kEnthalpyDimerKcalPerMole[row][column];
}

/**
  * @param dnaString [const BioString &]
  * @returns double
  */
double ThermodynamicCalculator::entropy(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());

    if (dnaString.isEmpty())
        return 0.;

    if (dnaString.length() == 1)
        return entropy(dnaString.at(1));

    const char *x = dnaString.constData();
    double sum = entropy(*x);

    for (int i=0, z=dnaString.length()-1; i<z; ++i, ++x)
        sum += entropy(*x, *(x+1));

    sum += entropy(*x);

    if (dnaString.isPalindrome())
        sum += constants::kEntropySymmetryCorrection;

    return sum;
}

/**
  * @param nucleotide [const char]
  * @returns double
  */
double ThermodynamicCalculator::entropy(const char nucleotide) const
{
    switch(nucleotide)
    {
    case 'A':        return constants::kEntropyMonomerCalPerKPerMole[0];
    case 'C':        return constants::kEntropyMonomerCalPerKPerMole[1];
    case 'G':        return constants::kEntropyMonomerCalPerKPerMole[2];
    case 'T':        return constants::kEntropyMonomerCalPerKPerMole[3];

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(nucleotide).toAscii());
        return 0;
    }
}

/**
  * @param upstreamNucleotide [const char]
  * @param downstreamNucleotide [const char]
  * @returns double
  */
double ThermodynamicCalculator::entropy(const char upstreamNucleotide, const char downstreamNucleotide) const
{
    int row = -1;
    switch (upstreamNucleotide)
    {
    case 'A':   row = 0; break;
    case 'C':   row = 1; break;
    case 'G':   row = 2; break;
    case 'T':   row = 3; break;

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(upstreamNucleotide).toAscii());
        return 0.;
    }

    int column = -1;
    switch (downstreamNucleotide)
    {
    case 'A':   column = 0; break;
    case 'C':   column = 1; break;
    case 'G':   column = 2; break;
    case 'T':   column = 3; break;

    default:
        ASSERT_X(false, QString("Invalid nucleotide: %1").arg(downstreamNucleotide).toAscii());
        return 0.;
    }

    return constants::kEntropyDimerCalPerKPerMole[row][column];
}

/**
  * @param dnaString [const BioString &]
  * @param sodiumConcentration [const double]
  * @param primerDnaConcentration [const double]
  * @returns double
  */
double ThermodynamicCalculator::meltingTemperature(const BioString &dnaString, const double sodiumConcentration, const double primerDnaConcentration) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());
    ASSERT(sodiumConcentration > 0.);
    ASSERT(primerDnaConcentration > 0.);

    if (dnaString.length() == 0)
        return 0.;

    return meltingTemperature(enthalpy(dnaString),
                              sodiumCorrectedEntropy(entropy(dnaString), dnaString.length(), sodiumConcentration),
                              primerDnaConcentration,
                              dnaString.isPalindrome());
}

/**
  * @param enthalpy [const double]
  * @param sodiumCorrectedEntropy [const double]
  * @param primerDnaConcentration [const double]
  * @param isPalindrome [bool]
  * @returns double
  */
double ThermodynamicCalculator::meltingTemperature(const double enthalpy, const double sodiumCorrectedEntropy, const double primerDnaConcentration, bool isPalindrome) const
{
    ASSERT(primerDnaConcentration > 0.);

    double adjustedPrimerConcentration = (!isPalindrome) ? primerDnaConcentration / 2.
                                                         : primerDnaConcentration;

    return 1000. * enthalpy / (sodiumCorrectedEntropy + constants::kR * log(adjustedPrimerConcentration)) - 273.15;
}

/**
  * @param entropy [const double]
  * @param sequenceLength [const int]
  * @param sodiumConcentration [const double]
  * @returns double
  */
double ThermodynamicCalculator::sodiumCorrectedEntropy(const double entropy, const int sequenceLength, const double sodiumConcentration) const
{
    ASSERT(sequenceLength > 0);
    ASSERT(sodiumConcentration > 0.);

#ifdef QT_DEBUG
    if (sodiumConcentration <= .1)
        qWarning("Entropy correction for sodium only accurate when concentration is above .1 M.");
#endif

    return entropy + 0.368 * (sequenceLength - 1) * log(sodiumConcentration);
}
