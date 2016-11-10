/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Primer.h"
#include "PrimerFactory.h"
#include "PrimerSearchParameters.h"
#include "ThermodynamicCalculator.h"
#include "DimerScoreCalculator.h"
#include "../core/BioString.h"
#include "../core/macros.h"

const double PrimerFactory::kDefaultSodiumMolarity_ = .2;            // Moles; also found in PrimerSearchParameters
const double PrimerFactory::kDefaultPrimerDnaMolarity_ = .000001;    // Moles; also found in PrimerSearchParameters

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
PrimerFactory::PrimerFactory()
    : sodiumConcentration_(kDefaultSodiumMolarity_),
      primerDnaConcentration_(kDefaultPrimerDnaMolarity_)
{
    ASSERT(sodiumConcentration_ > 0.);
    ASSERT(primerDnaConcentration_ > 0.);
}

/**
  * @param sodiumConcentration [const double]
  * @param primerDnaConcentration [const double]
  */
PrimerFactory::PrimerFactory(const double sodiumConcentration, const double primerDnaConcentration)
    : sodiumConcentration_(sodiumConcentration),
      primerDnaConcentration_(primerDnaConcentration)
{
    ASSERT(sodiumConcentration_ > 0.);
    ASSERT(primerDnaConcentration_ > 0.);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param dnaString [const BioString &]
  * @param restrictionEnzyme [const RestrictionEnzyme &]
  * @returns Primer
  */
Primer PrimerFactory::makePrimer(const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme)
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());
    ASSERT(dnaString.onlyContainsACGT());

    double tm = ThermodynamicCalculator().meltingTemperature(restrictionEnzyme.recognitionSite() + dnaString, sodiumConcentration_, primerDnaConcentration_);
    return makePrimer(dnaString, restrictionEnzyme, tm);
}

Primer PrimerFactory::makePrimer(const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme, const double tm)
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());
    ASSERT(dnaString.onlyContainsACGT());

    return makePrimer(QString(), dnaString, restrictionEnzyme, tm);
}

/**
  * @param bioString [const BioString &]
  * @param restrictionEnzyme [const RestrictionEnzyme &]
  * @param tm [const double]
  * @returns Primer
  */
Primer PrimerFactory::makePrimer(const QString &name, const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme, const double tm)
{
    ASSERT(dnaString.grammar() == eDnaGrammar);
    ASSERT(!dnaString.hasGaps());

    DimerScoreCalculator dimerScoreCalculator;
    double homoDimerScore = dimerScoreCalculator.homoDimerScore(Primer::sequence(dnaString, restrictionEnzyme));
    Primer newPrimer = Primer(::newEntityId<Primer>(), dnaString, restrictionEnzyme, tm, homoDimerScore, primerSearchParameters_);
    newPrimer.setName(name);
    return newPrimer;
}

/**
  * @returns double
  */
double PrimerFactory::primerDnaConcentration() const
{
    return primerDnaConcentration_;
}

void PrimerFactory::reset()
{
    sodiumConcentration_ = kDefaultSodiumMolarity_;
    primerDnaConcentration_ = kDefaultPrimerDnaMolarity_;
    primerSearchParameters_.clear();
}

/**
  * @param newPrimerDnaConcentration [const double]
  */
void PrimerFactory::setPrimerDnaConcentration(const double newPrimerDnaConcentration)
{
    ASSERT(newPrimerDnaConcentration > 0.);
    primerDnaConcentration_ = newPrimerDnaConcentration;
}

void PrimerFactory::setPrimerSearchParameters(const QSharedPointer<PrimerSearchParameters> &primerSearchParameters)
{
    primerSearchParameters_ = primerSearchParameters;
}

/**
  * @param newSodiumDnaConcentration [const double]
  */
void PrimerFactory::setSodiumConcentration(const double newSodiumDnaConcentration)
{
    ASSERT(newSodiumDnaConcentration > 0.);
    sodiumConcentration_ = newSodiumDnaConcentration;
}

/**
  * @returns double
  */
double PrimerFactory::sodiumConcentration() const
{
    return sodiumConcentration_;
}
