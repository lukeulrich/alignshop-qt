/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef PRIMER_H
#define PRIMER_H

#include <QtCore/QSharedPointer>
#include <QtCore/QVector>
#include "../core/Entities/AbstractEntity.h"
#include "../core/BioString.h"
#include "RestrictionEnzyme.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class PrimerSearchParameters;

class Primer;
typedef QVector<Primer> PrimerVector;
Q_DECLARE_METATYPE(PrimerVector)

/**
  * Primer is a value object representing an individual primer with an optional restriction enzyme.
  *
  * Because this is a value object, any melting point and score data must be supplied during construction. To derive
  * these values use an appropriate factory that properly creates Primers with this data.
  *
  * All primer sequence data is stored in the 5' -> 3' orientation with the 5' position having a lower value than 3'.
  * Any return values are relative to this orientation.
  */
class Primer : public AbstractEntity
{
public:
    static const int kType;

    // ------------------------------------------------------------------------------------------------
    // Constructor
    Primer();                                       //!< Constructs a null primer
    //! Constructs a primer with coreSequence, restrictionEnyzme, tm, and homoDimerScore
    Primer(const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore);
    Primer(const int id, const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore);
    Primer(const int id, const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme, const double tm, const double homoDimerScore, const QSharedPointer<PrimerSearchParameters> &primerSearchParameters);

    // The following methods are needed to prevent QSharedPointer from issuing warnings about deleting incomplete types
    // Reference: https://bugreports.qt.nokia.com/browse/QTBUG-7302
    Primer(const Primer &other);
    ~Primer();
    Primer &operator=(const Primer &other);


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const Primer &other) const;     //!< Returns true if other is equivalent to this object; false otherwise
    bool operator!=(const Primer &other) const;     //!< Returns true if other is not equivalent to this object; false otherwise


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString coreSequence() const;                 //!< Returns the primer sequence excluding any restriction enzyme recognition site
    //!< Returns the number of matches to the primer core sequence that occur in dnaString
    int countCoreSequenceForwardMatchesIn(const BioString &dnaString) const;
    //!< Returns the sum of forward and reverse matches to the primer core sequence that occur in dnaString
    int countCoreSequenceMatchesIn(const BioString &dnaString) const;
    //!< Returns the number of matches to the primer core sequence that occur in the reverse complement of dnaString
    int countCoreSequenceReverseMatchesIn(const BioString &dnaString) const;
    QVector<ClosedIntRange> coreSequenceForwardLocationsIn(const BioString &dnaString) const;
    QVector<ClosedIntRange> coreSequenceReverseLocationsIn(const BioString &dnaString) const;
    double homoDimerScore() const;                  //!< Returns the score of this primer pairing with itself
    bool isNull() const;                            //!< Returns true if a core sequence has not been defined; false otherwise
    //! Returns the range where the core sequence first occurs in dnaString
    ClosedIntRange locateCoreSequenceIn(const BioString &dnaString) const;
    //! Returns the range where the cognate, or reverse complement, of the core sequence first occurs in dnaString
    ClosedIntRange locateCoreSequenceInCognateStrand(const BioString &dnaString) const;
    //! Returns the 1-based start position where the core sequence first occurs in dnaString or -1 if core sequence is empty or not found
    int locateCoreSequenceStartIn(const BioString &dnaString) const;
    //! Returns the 1-based start position where the cognate, or reverse complement, of the core sequence first occurs in dnaString or -1 if core sequence is empty or not found
    int locateCoreSequenceStartInCognateStrand(const BioString &dnaString) const;
    //! Returns the 1-based stop position where the core sequence first occurs in dnaString or -1 if core sequence is empty or not found
    int locateCoreSequenceStopIn(const BioString &dnaString) const;
    //! Returns the 1-based stop position where the cognate, or reverse complement, of the core sequence first occurs in dnaString or -1 if core sequence is empty or not found
    int locateCoreSequenceStopInCognateStrand(const BioString &dnaString) const;
    QString name() const;
    //! Returns a const pointer to any associated PrimerSearchParameters
    const PrimerSearchParameters *primerSearchParameters() const;
    RestrictionEnzyme restrictionEnzyme() const;    //!< Returns the restriction enzyme
    void setName(const QString &name);
    BioString sequence() const;                     //!< Convenience method that calls its static equivalent; returns the full primer sequence including any restriction enzyme recognition site
    double tm() const;                              //!< Returns the melting temperature of this primer
    int type() const;                               //!< Returns ePrimerEntity


    void setPrimerSearchParametersId(const int id);     //!< Only to be called by CRUD


    // ------------------------------------------------------------------------------------------------
    // Static methods
    //! Returns the full primer sequence including any restriction enzyme recognition site; useful from factories and avoids duplicating code
    static BioString sequence(const BioString &coreSequence, const RestrictionEnzyme &restrictionEnzyme);
    //! Returns the first row of the PrimerPair within primerPairVector that has an id of primerPairId or -1 if none is found
    static int primerRowById(const PrimerVector &primerVector, const int primerId);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QString name_;
    BioString coreSequence_;                //!< The primer sequence excluding any restriction enzyme
    RestrictionEnzyme restrictionEnzyme_;   //!< Optional restriction enzyme; 5' addition
    double tm_;                             //!< Melting point of the entire primer sequence (includes the restriction enzyme recognition site)
    double homoDimerScore_;                 //!< Score of this primer as paired with itself
    QSharedPointer<PrimerSearchParameters> primerSearchParameters_;
};

Q_DECLARE_TYPEINFO(Primer, Q_MOVABLE_TYPE);

#endif // PRIMER_H
