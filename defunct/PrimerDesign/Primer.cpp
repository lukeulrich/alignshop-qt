/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "Primer.h"
#include "DnaSequence.h"
#include "global.h"
#include "ThermodynamicCalculator.h"
#include <PrimerDesign/DimerCalculator.h>

using namespace PrimerDesign;

const double PRIMER_VALUE_UNINITIALIZED = -2;
const double Primer::DEFAULT_ENTROPY = .5;
const double Primer::DEFAULT_ENTHALPY = .5;
const double Primer::DEFAULT_HOMO_SCORE = 0;

Primer::Primer()
    : sequence_(StrictDnaString::isNotEmpty)
{
    tm_ = PRIMER_VALUE_UNINITIALIZED;
    entropy_ = PRIMER_VALUE_UNINITIALIZED;
    enthalpy_ = PRIMER_VALUE_UNINITIALIZED;
    homoDimerScore_ = PRIMER_VALUE_UNINITIALIZED;
    sequencePosition_ = -1;
}

Primer::Primer(const QString &sequence, const PrimerDesignInput &params)
{
    ThermodynamicCalculator b;
    b.setSequence(sequence);

    setSequence(sequence);
    setEnthalpy(b.calculateEnthalpy());
    setEntropy(b.calculateEntropy());
    setTm(b.meltingTemperature(params.sodiumConcentration));

    homoDimerScore_ = PRIMER_VALUE_UNINITIALIZED;
    sequencePosition_ = -1;
}

/**
  * @return QString
  */
QString Primer::sequence() const
{
    return sequence_.value();
}

/**
  * @param sequence [QString]
  * @return bool
  */
bool Primer::setSequence(const QString &sequence)
{
    return sequence_.setValue(sequence);
}

/**
  * @return double
  */
double Primer::tm() const
{
    return tm_;
}

/**
  * @param tm [double]
  * @return bool
  */
bool Primer::setTm(double tm)
{
    tm_ = tm;
    return true;
}

/**
  * @return double
  */
double Primer::entropy() const
{
    return entropy_;
}

/**
  * @param entropy [double]
  * @return bool
  */
bool Primer::setEntropy(double entropy)
{
    entropy_ = entropy;
    return true;
}

/**
  * @return double
  */
double Primer::enthalpy() const
{
    return enthalpy_;
}

/**
  * @param enthalpy [double]
  * @return bool
  */
bool Primer::setEnthalpy(double enthalpy)
{
    enthalpy_ = enthalpy;
    return true;
}

/**
  * @return int
  */
int Primer::homoDimerScore()
{
    if (homoDimerScore_ == PRIMER_VALUE_UNINITIALIZED)
    {
        homoDimerScore_ = DimerCalculator::score(sequence_, sequence_);
    }

    return homoDimerScore_;
}

/**
  * @param homoDimerScore [int]
  * @return bool
  */
bool Primer::setHomoDimerScore(int homoDimerScore)
{
    homoDimerScore_ = homoDimerScore;
    return true;
}

/**
  * @return int
  */
int Primer::sequencePosition() const
{
    return sequencePosition_;
}

/**
  * @param homoDimerScore [int]
  * @return bool
  */
bool Primer::setSequencePosition(int position)
{
    sequencePosition_ = position;
    return true;
}
