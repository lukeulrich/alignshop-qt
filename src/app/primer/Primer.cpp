/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "Primer.h"
#include "PrimerSearchParameters.h"
#include "../core/macros.h"

const int Primer::kType = ePrimerEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
Primer::Primer()
    : AbstractEntity(0),
      coreSequence_(eDnaGrammar),
      tm_(0.),
      homoDimerScore_(0.)
{
}

/**
  * @param coreSequence [const BioString &]
  * @param restrictionEnzyme [const RestrictionEnzyme &]
  * @param tm [const double]
  * @param homoDimerScore [const double]
  */
Primer::Primer(const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore)
    : AbstractEntity(0),
      coreSequence_(coreSequence),
      restrictionEnzyme_(restrictionEnzyme),
      tm_(tm),
      homoDimerScore_(homoDimerScore)
{
    ASSERT(coreSequence.grammar() == eDnaGrammar);
    ASSERT(!coreSequence.hasGaps());
}

Primer::Primer(const int id, const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore)
    : AbstractEntity(id),
      coreSequence_(coreSequence),
      restrictionEnzyme_(restrictionEnzyme),
      tm_(tm),
      homoDimerScore_(homoDimerScore)
{
    ASSERT(coreSequence.grammar() == eDnaGrammar);
    ASSERT(!coreSequence.hasGaps());
}

Primer::Primer(const int id, const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore, const QSharedPointer<PrimerSearchParameters> &primerSearchParameters)
    : AbstractEntity(id),
      coreSequence_(coreSequence),
      restrictionEnzyme_(restrictionEnzyme),
      tm_(tm),
      homoDimerScore_(homoDimerScore),
      primerSearchParameters_(primerSearchParameters)
{
    ASSERT(coreSequence.grammar() == eDnaGrammar);
    ASSERT(!coreSequence.hasGaps());
}


Primer::Primer(const Primer &other)
    : AbstractEntity(other),
      name_(other.name_),
      coreSequence_(other.coreSequence_),
      restrictionEnzyme_(other.restrictionEnzyme_),
      tm_(other.tm_),
      homoDimerScore_(other.homoDimerScore_),
      primerSearchParameters_(other.primerSearchParameters_)
{
}

Primer::~Primer()
{
}

Primer &Primer::operator=(const Primer &other)
{
    if (this == &other)
        return *this;

    AbstractEntity::operator=(other);

    name_ = other.name_;
    coreSequence_ = other.coreSequence_;
    restrictionEnzyme_ = other.restrictionEnzyme_;
    tm_ = other.tm_;
    homoDimerScore_ = other.homoDimerScore_;
    primerSearchParameters_ = other.primerSearchParameters_;

    return *this;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const Primer &]
  * @returns bool
  */
bool Primer::operator==(const Primer &other) const
{
    return coreSequence_ == other.coreSequence_ &&
           restrictionEnzyme_ == other.restrictionEnzyme_ &&
           qFuzzyCompare(tm_, other.tm_) &&
           qFuzzyCompare(homoDimerScore_, other.homoDimerScore_);
}

/**
  * @param other [const Primer &]
  * @returns bool
  */
bool Primer::operator!=(const Primer &other) const
{
    return !operator==(other);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioString
  */
BioString Primer::coreSequence() const
{
    return coreSequence_;
}

int Primer::countCoreSequenceForwardMatchesIn(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);

    return dnaString.count(coreSequence_);
}

int Primer::countCoreSequenceMatchesIn(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);

    return countCoreSequenceForwardMatchesIn(dnaString) +
           countCoreSequenceReverseMatchesIn(dnaString);
}

int Primer::countCoreSequenceReverseMatchesIn(const BioString &dnaString) const
{
    ASSERT(dnaString.grammar() == eDnaGrammar);

    return dnaString.reverseComplement().count(coreSequence_);
}

QVector<ClosedIntRange> Primer::coreSequenceForwardLocationsIn(const BioString &dnaString) const
{
    if (coreSequence_.isEmpty())
        return QVector<ClosedIntRange>();

    QVector<ClosedIntRange> matches;
    int from = 0;
    forever
    {
        from = dnaString.indexOf(coreSequence_, from + 1);
        if (from != -1)
        {
            matches << ClosedIntRange(from, from + coreSequence_.length() - 1);
            continue;
        }

        break;
    }
    return matches;
}

QVector<ClosedIntRange> Primer::coreSequenceReverseLocationsIn(const BioString &dnaString) const
{
    if (coreSequence_.isEmpty())
        return QVector<ClosedIntRange>();

    BioString reverseComplementedCoreSequence = coreSequence_.reverseComplement();
    QVector<ClosedIntRange> matches;
    int from = 0;
    forever
    {
        from = dnaString.indexOf(reverseComplementedCoreSequence, from + 1);
        if (from != -1)
        {
            matches << ClosedIntRange(from, from + reverseComplementedCoreSequence.length() - 1);
            continue;
        }

        break;
    }
    return matches;
}

/**
  * @returns double
  */
double Primer::homoDimerScore() const
{
    return homoDimerScore_;
}

/**
  * @returns bool
  */
bool Primer::isNull() const
{
    return coreSequence_.isEmpty();
}

/**
  * @param dnaString [const BioString &]
  * @returns ClosedIntRange
  */
ClosedIntRange Primer::locateCoreSequenceIn(const BioString &dnaString) const
{
    int start = locateCoreSequenceStartIn(dnaString);
    if (start == -1)
        return ClosedIntRange();

    int stop = start + coreSequence_.length() - 1;
    return ClosedIntRange(start, stop);
}

/**
  * Because this method deals with the reverse complement, the first occurrence is relative to the right most position
  * in dnaString.
  *
  * @param dnaString [const BioString &]
  * @returns ClosedIntRange
  */
ClosedIntRange Primer::locateCoreSequenceInCognateStrand(const BioString &dnaString) const
{
    int start = locateCoreSequenceStartInCognateStrand(dnaString);
    if (start == -1)
        return ClosedIntRange();

    int stop = start + coreSequence_.length() - 1;
    return ClosedIntRange(start, stop);
}

/**
  * @param dnaString [const BioString &]
  * @returns int
  */
int Primer::locateCoreSequenceStartIn(const BioString &dnaString) const
{
    if (coreSequence_.isEmpty())
        return -1;

    return dnaString.indexOf(coreSequence_);
}

/**
  * @param dnaString [const BioString &]
  * @returns int
  */
int Primer::locateCoreSequenceStartInCognateStrand(const BioString &dnaString) const
{
    if (coreSequence_.isEmpty())
        return -1;

    BioString reverseComplementedCoreSequence = coreSequence_.reverseComplement();
    return dnaString.lastIndexOf(reverseComplementedCoreSequence);
}

/**
  * @param dnaString [const BioString &]
  * @returns int
  */
int Primer::locateCoreSequenceStopIn(const BioString &dnaString) const
{
    int start = locateCoreSequenceStartIn(dnaString);
    if (start == -1)
        return -1;

    int stop = start + coreSequence_.length() - 1;
    return stop;
}

/**
  * @param dnaString [const BioString &]
  * @returns int
  */
int Primer::locateCoreSequenceStopInCognateStrand(const BioString &dnaString) const
{
    int start = locateCoreSequenceStartInCognateStrand(dnaString);
    if (start == -1)
        return -1;

    int stop = start + coreSequence_.length() - 1;
    return stop;
}

QString Primer::name() const
{
    return name_;
}

/**
  * @returns const PrimerSearchParameters *
  */
const PrimerSearchParameters *Primer::primerSearchParameters() const
{
    return primerSearchParameters_.data();
}

/**
  * @returns RestrictionEnzyme
  */
RestrictionEnzyme Primer::restrictionEnzyme() const
{
    return restrictionEnzyme_;
}

void Primer::setName(const QString &name)
{
    name_ = name;
}

/**
  * @returns BioString
  */
BioString Primer::sequence() const
{
    return sequence(coreSequence_, restrictionEnzyme_);
}

/**
  * @returns double
  */
double Primer::tm() const
{
    return tm_;
}

/**
  * @returns int
  */
int Primer::type() const
{
    return kType;
}


/**
  * @param id [const int]
  */
void Primer::setPrimerSearchParametersId(const int id)
{
    if (primerSearchParameters_.isNull())
        return;

    primerSearchParameters_->id_ = id;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static methods
/**
  * @param coreSequence [const BioString &]
  * @param RestrictionEnzyme [const RestrictionEnzyme &]
  * @returns BioString
  */
BioString Primer::sequence(const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme)
{
    ASSERT(coreSequence.grammar() == eDnaGrammar);

    return restrictionEnzyme.recognitionSite() + coreSequence;
}

/**
  * @param primerPairVector [const PrimerPairVector &]
  * @param primerPairId [const int]
  * @returns int
  */
int Primer::primerRowById(const PrimerVector &primerVector, const int primerId)
{
    for (int i=0; i< primerVector.size(); ++i)
        if (primerVector.at(i).id() == primerId)
            return i;

    return -1;
}
