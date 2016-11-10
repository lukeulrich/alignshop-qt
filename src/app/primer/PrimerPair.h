/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef PRIMERPAIR_H
#define PRIMERPAIR_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "Primer.h"

class PrimerPair;
typedef QVector<PrimerPair> PrimerPairVector;
Q_DECLARE_METATYPE(PrimerPairVector)

/**
  * PrimerPair represents a pair of primers.
  *
  * Similar to Primer instances, objects of this class are value objects and properties such as the score must be
  * supplied during construction. Use an appropriate factory that properly creates PrimerPairs with this data.
  */
class PrimerPair
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    PrimerPair();                                       //!< Constructs an empty primer pair
    //! Constructs a primer pair from name, forwardPrimer, reversePrimer, and score
    PrimerPair(const Primer &forwardPrimer, const Primer &reversePrimer, const double score);


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const PrimerPair &other) const;     //!< Returns true if other is equivalent to this instance; false otherwise
    bool operator!=(const PrimerPair &other) const;     //!< Returns true if other is not equivalent to this instance; false otherwise


    // ------------------------------------------------------------------------------------------------
    // Public methods
    double deltaTm() const;                             //!< Returns the absolute difference in melting temperatures between the forward and reverse primers
    Primer forwardPrimer() const;                       //!< Returns the forward primer
    //! Returns the location (5' -> 3' coordinates) of the first occurrence of the forward primer's core sequence in bioString
    ClosedIntRange locateForwardPrimerCoreSequenceIn(const BioString &dnaString) const;
    //! Returns the location (5' -> 3' coordinates) of the first occurrence (relative to the 5' position of the reverse complement) of the reverse primer's core sequence in bioString
    ClosedIntRange locateReversePrimerCoreSequenceIn(const BioString &dnaString) const;
    //! Returns the longest amplicon found in dnaString or 0 if there are no matches to the primer sequences (or one of the primer sequences is empty)
    int longestAmpliconLength(const BioString &dnaString) const;
    QString name() const;                               //!< Returns the primer pair name
    Primer reversePrimer() const;                       //!< Returns the reverse primer
    double score() const;                               //!< Returns the score
    void setForwardPrimerName(const QString &forwardPrimerName);
    void setName(const QString &newName);               //!< Sets the name to newName
    void setReversePrimerName(const QString &reversePrimerName);


    // ------------------------------------------------------------------------------------------------
    // Static public methods
    //! Returns the absolute difference in melting temperatures between the forward and reverse primers
    static double deltaTm(const Primer &primer1, const Primer &primer2);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QString name_;              //!< Arbitrary user name for this primer pair
    Primer forwardPrimer_;      //!< Forward primer
    Primer reversePrimer_;      //!< Reverse primer
    double score_;              //!< Score of forward primer associated with the reverse primer
};

Q_DECLARE_TYPEINFO(PrimerPair, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const PrimerPair &range);
#endif

#endif // PRIMERPAIR_H
