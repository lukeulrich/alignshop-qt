/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "PrimerPair.h"
#include "../core/enums.h"

static int qRegisterTypes()
{
    qRegisterMetaType<PrimerPair>("PrimerPair");
    qRegisterMetaType<PrimerPairVector>("PrimerPairVector");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterTypes)


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
PrimerPair::PrimerPair()
    : score_(0.)
{
}

/**
  * @param name [const QString &]
  * @param forwardPrimer [const Primer &]
  * @param reversePrimer [const Primer &]
  * @param score [const double]
  */
PrimerPair::PrimerPair(const Primer &forwardPrimer, const Primer &reversePrimer, const double score)
    : forwardPrimer_(forwardPrimer),
      reversePrimer_(reversePrimer),
      score_(score)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param other [const PrimerPair &]
  * @returns bool
  */
bool PrimerPair::operator==(const PrimerPair &other) const
{
    return name_ == other.name_ &&
           forwardPrimer_ == other.forwardPrimer_ &&
           reversePrimer_ == other.reversePrimer_ &&
           qFuzzyCompare(score_, other.score_);
}

/**
  * @param other [const PrimerPair &]
  * @returns bool
  */
bool PrimerPair::operator!=(const PrimerPair &other) const
{
     return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns double
  */
double PrimerPair::deltaTm() const
{
    return deltaTm(forwardPrimer_, reversePrimer_);
}

/**
  * @returns Primer
  */
Primer PrimerPair::forwardPrimer() const
{
    return forwardPrimer_;
}

/**
  * @param dnaString [const BioString &]
  * @returns ClosedIntRange
  */
ClosedIntRange PrimerPair::locateForwardPrimerCoreSequenceIn(const BioString &dnaString) const
{
    return forwardPrimer_.locateCoreSequenceIn(dnaString);
}

/**
  * @param dnaString [const BioString &]
  * @returns ClosedIntRange
  */
ClosedIntRange PrimerPair::locateReversePrimerCoreSequenceIn(const BioString &dnaString) const
{
    return reversePrimer_.locateCoreSequenceInCognateStrand(dnaString);
}

/**
  * @param dnaString (const BioString &)
  * @returns int
  */
int PrimerPair::longestAmpliconLength(const BioString &dnaString) const
{
    ClosedIntRange ampliconLocation;
    ampliconLocation.begin_ = forwardPrimer_.locateCoreSequenceStartIn(dnaString);
    if (ampliconLocation.begin_ == -1)
        return 0;

    ampliconLocation.end_ = reversePrimer_.locateCoreSequenceStopInCognateStrand(dnaString);
    if (ampliconLocation.end_ == -1)
        return 0;

    return ampliconLocation.length();
}

/**
  * @returns QString
  */
QString PrimerPair::name() const
{
    return name_;
}

/**
  * @returns Primer
  */
Primer PrimerPair::reversePrimer() const
{
    return reversePrimer_;
}

/**
  * @returns double
  */
double PrimerPair::score() const
{
    return score_;
}

void PrimerPair::setForwardPrimerName(const QString &forwardPrimerName)
{
    forwardPrimer_.setName(forwardPrimerName);
}

/**
  * @param newName [const QString &]
  */
void PrimerPair::setName(const QString &newName)
{
    name_ = newName;
}

void PrimerPair::setReversePrimerName(const QString &reversePrimerName)
{
    reversePrimer_.setName(reversePrimerName);
}


// ------------------------------------------------------------------------------------------------
// Static public methods
/**
  * @param primer1 [const Primer &]
  * @param primer2 [const Primer &]
  * @returns double
  */
double PrimerPair::deltaTm(const Primer &primer1, const Primer &primer2)
{
    return qAbs(primer1.tm() - primer2.tm());
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Debug methods
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const PrimerPair &primerPair)
{
    const RestrictionEnzyme &forwardRe = primerPair.forwardPrimer().restrictionEnzyme();
    const RestrictionEnzyme &reverseRe = primerPair.reversePrimer().restrictionEnzyme();

    QString message = "PrimerPair(";
    if (!primerPair.name().isEmpty())
        message += primerPair.name() + ": ";
    message += "F - {";
    if (!forwardRe.isEmpty())
    {
        message += "[";
        if (!forwardRe.name().isEmpty())
            message += forwardRe.name() + " - ";
        message += forwardRe.recognitionSite().asByteArray() + "]";
    }
    message += primerPair.forwardPrimer().coreSequence().asByteArray() + "}; R - {";
    if (!reverseRe.isEmpty())
    {
        message += "[";
        if (!reverseRe.name().isEmpty())
            message += reverseRe.name() + " - ";
        message += reverseRe.recognitionSite().asByteArray() + "]";
    }
    message += primerPair.reversePrimer().coreSequence().asByteArray() + "}";
    message += ", " + QString::number(primerPair.score()) + ")";

    debug << message;

    return debug.maybeSpace();
}
#endif
