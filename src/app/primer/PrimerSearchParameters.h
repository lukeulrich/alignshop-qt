/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERSEARCHPARAMETERS_H
#define PRIMERSEARCHPARAMETERS_H

#include <QtCore/QString>

#include "RestrictionEnzyme.h"

#include "../core/ValueObjects/ValueObject.h"

#include "../core/BioString.h"
#include "../core/DnaPattern.h"
#include "../core/util/ClosedIntRange.h"
#include "../core/util/Range.h"

/**
  * PrimerSearchParameters aggregates the various parameters involved in searching for primers.
  *
  * All parameters are exposed as public properties. Use the isValid() method to check if the set as a whole contains
  * a valid configuration. If it returns false, call errorMessage() to determine the error.
  */
class PrimerSearchParameters : public ValueObject
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerSearchParameters();
    explicit PrimerSearchParameters(const int id);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString errorMessage() const;                   //!< Returns any error message
    double microMolarDnaConcentration() const;
    int milliMolarSodiumConcentration() const;
    bool isValid() const;                           //!< If the parameter set is valid, then clears the internal error message and returns true; otherwise the internal error message indicating the problem is set and false is returned
    void setSodiumConcentrationFromMilliMoles(const double milliMolarSodiumConcentration);
    void setPrimerDnaConcentrationFromMicroMoles(const double microMolarDnaConcentration);


    // Public properties
    ClosedIntRange ampliconLengthRange_;            //!< Range of acceptable amplicon sizes
    ClosedIntRange primerLengthRange_;              //!< Range of allowed primer lengths
    RestrictionEnzyme forwardRestrictionEnzyme_;    //!< The forward prefix or restriction enzyme site; 5' addition
    RestrictionEnzyme reverseRestrictionEnzyme_;    //!< The reverse prefix or restriction enzyme site; 5' addition
    DnaPattern forwardTerminalPattern_;             //!< The forward suffix that must be present; 3' specification
    DnaPattern reverseTerminalPattern_;             //!< The reverse suffix that must be present; 3' specification
    RangeF individualPrimerTmRange_;                //!< Acceptable melting temperature range for individual primers
    double sodiumConcentration_;                    //!< Molar sodium concentration
    double primerDnaConcentration_;                 //!< Concentration of primer dna (molar)
    double maximumPrimerPairDeltaTm_;               //!< Maximum difference in melting temperatures for any given pair of primers


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    mutable QString errorMessage_;      //!< Error message that is set when the isValid method is called
};

Q_DECLARE_TYPEINFO(PrimerSearchParameters, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(PrimerSearchParameters)

#endif // PRIMERSEARCHPARAMETERS_H
