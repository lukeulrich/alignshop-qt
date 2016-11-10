/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "PrimerPair.h"

#include <PrimerDesign/DimerCalculator.h>

using namespace PrimerDesign;

const int PRIMERPAIR_UNDEFINED_VALUE = -2;

PrimerPair::PrimerPair(const PrimerDesignInputRef &params)
{
    params_ = params;
    score_ = PRIMERPAIR_UNDEFINED_VALUE;
}

PrimerPair::~PrimerPair()
{
}

PrimerDesignInput * PrimerPair::params() const
{
    return params_.data();
}

/**
  * @return Primer
  */
Primer &PrimerPair::forwardPrimer()
{
    return forward_;
}

/**
  * @param forward [Primer]
  * @return bool
  */
bool PrimerPair::setForwardPrimer(const Primer &forward)
{
    forward_ = forward;
    return true;
}

QString PrimerPair::name() const
{
    return name_;
}

bool PrimerPair::setName(const QString &name)
{
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
    {
        return false;
    }

    name_ = trimmed;
    return true;
}

/**
  * @return Primer
  */
Primer &PrimerPair::reversePrimer()
{
    return reverse_;
}

/**
  * @param opposite [Primer]
  * @return bool
  */
bool PrimerPair::setReversePrimer(const Primer &reverse)
{
    reverse_ = reverse;
    return true;
}

/**
  * @return double
  */
double PrimerPair::score() const
{
    return score_;
}

void PrimerPair::recomputePrimerPairScore()
{
    double heteroDimerScore = DimerCalculator::score(forward_.sequence(), reverse_.sequence());
    double deltaTm = abs(forward_.tm() - reverse_.tm());
    double forwardHomoDiv = (double)forward_.homoDimerScore() / forward_.sequence().length();
    double reverseHomoDiv = (double)reverse_.homoDimerScore() / reverse_.sequence().length();
    double heteroDiv = (double)heteroDimerScore / (forward_.sequence().length() + reverse_.sequence().length());

    score_ = deltaTm + forwardHomoDiv + reverseHomoDiv + (heteroDiv / 2);
}

/**
  * @param heteroScore [double]
  * @return bool
  */
bool PrimerPair::setScore(double value)
{
    if (value >= 0)
    {
        score_ = value;
        return true;
    }

    return false;
}

int PrimerPair::ampliconLength() const
{
    return params_->forwardPrefix.length() + params_->reversePrefix.length() +
           reverse_.sequencePosition() - forward_.sequencePosition();
}
