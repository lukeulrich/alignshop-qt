/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerSearchParameters.h"
#include "../core/macros.h"

static const int kDefaultMinPrimerLength = 20;
static const int kDefaultMaxPrimerLength = 25;
static const double kDefaultMinTmRange = 55.0;
static const double kDefaultMaxTmRange = 65.0;
static const double kDefaultSodiumMolarity = .2;            // Molar; also found in PrimerFactory
static const double kDefaultPrimerDnaMolarity = .000001;    // Molar; also found in PrimerFactory
static const double kDefaultMaximumPrimerPairDeltaTm = 5.;  // Degrees Celsius


static int qRegisterTypes()
{
    qRegisterMetaType<PrimerSearchParameters>("PrimerSearchParameters");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterTypes)



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [const int]
  */
PrimerSearchParameters::PrimerSearchParameters()
    : primerLengthRange_(kDefaultMinPrimerLength, kDefaultMaxPrimerLength),
      individualPrimerTmRange_(kDefaultMinTmRange, kDefaultMaxTmRange),
      sodiumConcentration_(kDefaultSodiumMolarity),
      primerDnaConcentration_(kDefaultPrimerDnaMolarity),
      maximumPrimerPairDeltaTm_(kDefaultMaximumPrimerPairDeltaTm)
{
}

/**
  * @param id [const int]
  */
PrimerSearchParameters::PrimerSearchParameters(const int id)
    : ValueObject(id),
      primerLengthRange_(kDefaultMinPrimerLength, kDefaultMaxPrimerLength),
      individualPrimerTmRange_(kDefaultMinTmRange, kDefaultMaxTmRange),
      sodiumConcentration_(kDefaultSodiumMolarity),
      primerDnaConcentration_(kDefaultPrimerDnaMolarity),
      maximumPrimerPairDeltaTm_(kDefaultMaximumPrimerPairDeltaTm)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString PrimerSearchParameters::errorMessage() const
{
    return errorMessage_;
}

double PrimerSearchParameters::microMolarDnaConcentration() const
{
    return primerDnaConcentration_ * 1000000.;
}

int PrimerSearchParameters::milliMolarSodiumConcentration() const
{
    return static_cast<int>(sodiumConcentration_ * 1000.);
}

/**
  * @returns bool
  */
bool PrimerSearchParameters::isValid() const
{
    errorMessage_.clear();

//    if (sourceSequence_.isEmpty())
//        errorMessage_ = "No DNA sequence has been configured.";
//    else if (searchRange_.isEmpty())
//        errorMessage_ = "Invalid search range. The start position must be less than or equal to the stop position.";
//    else if (searchRange_.begin_ < 1)
//        errorMessage_ = "The search range start must be greater than or equal to 1.";
//    else if (searchRange_.end_ > sourceSequence_.length())
//        errorMessage_ = "The search range stop must be less than or equal to the sequence length.";
    if (ampliconLengthRange_.isEmpty())
        errorMessage_ = "Invalid amplicon length range. The start value must be less than or equal to the stop value.";
    else if (ampliconLengthRange_.begin_ < 1)
        errorMessage_ = "The amplicon length minimum must be greater than or equal to 1.";
//    else if (ampliconLengthRange_.end_ > searchRange_.end_)
//        errorMessage_ = "The maximum amplicon length may not be larger than the target sequence length.";
    else if (primerLengthRange_.isEmpty())
        errorMessage_ = "Invalid primer length range. The start value must be less than or equal to the stop value.";
    else if (primerLengthRange_.begin_ < 1)
        errorMessage_ = "The minimum primer length must be greater than or equal to 1.";
    //                          Minimum primer length             Maximum amplicon length
    //                          ||||||                            ||||
    else if (primerLengthRange_.begin_ * 2 > ampliconLengthRange_.end_)
        errorMessage_ = "The amplicon size that you have selected is too small. The maximum amplicon size must be at least 2 times longer than the minimum primer length.";
    else if (!individualPrimerTmRange_.isValid())
        errorMessage_ = "Invalid melting point range. The start value must be less than or equal to the stop value.";
    else if (sodiumConcentration_ < 0.)
        errorMessage_ = "Sodium concentration must be a positive molar value.";
    else if (primerDnaConcentration_ < 0.)
        errorMessage_ = "Primer DNA concentration must be a positive molar value.";
    else if (maximumPrimerPairDeltaTm_ < 0)
        errorMessage_ = "The maximum melting temperature difference for a given primer pair must be positive.";

    return errorMessage_.isEmpty();
}

void PrimerSearchParameters::setSodiumConcentrationFromMilliMoles(const double milliMolarSodiumConcentration)
{
    sodiumConcentration_ = milliMolarSodiumConcentration / 1000.;
}

void PrimerSearchParameters::setPrimerDnaConcentrationFromMicroMoles(const double microMolarDnaConcentration)
{
    primerDnaConcentration_ = microMolarDnaConcentration / 1000000.;
}
