/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERFACTORY_H
#define PRIMERFACTORY_H

#include <QtCore/QSharedPointer>

class BioString;
class Primer;
class PrimerSearchParameters;
class RestrictionEnzyme;

/**
  * PrimerFactory produces Primer objects and in particular encapsulates the derivation of their melting point
  * temperatures and homodimer scores.
  *
  * By separating the creation of Primer objects to this class, Primer objects focus on "carrying data" rather than
  * being responsible for the various calculations needed to derive its relevant properties.
  *
  * The sodium concentration and primer dna concentrations are needed to calculate the melting temperatures. These can
  * supplied during construction. Default values are used if the default constructor is invoked. All concentrations are
  * in moles.
  */
class PrimerFactory
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerFactory();                            //!< Construct an instance with default values
    //! Construct a primer factory with sodiumConcentration and primerDnaConcentration
    PrimerFactory(const double sodiumConcentration, const double primerDnaConcentration);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Create a primer from dnaString and restrictionEnzyme; also calculates the tm and homoDimerScore
    Primer makePrimer(const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme);
    //! Create a primer from dnaString and restrictionEnzyme and tm; also calculates homoDimerScore; does not utilize the sodium or primer dna concentration values
    Primer makePrimer(const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme, const double tm);
    //! Create a primer from dnaString and restrictionEnzyme and tm; also calculates homoDimerScore; does not utilize the sodium or primer dna concentration values
    Primer makePrimer(const QString &name, const BioString &dnaString, const RestrictionEnzyme &restrictionEnzyme, const double tm);
    double primerDnaConcentration() const;      //!< Returns the primer dna concentration (moles)
    void reset();
    //! Sets the primer dna concentration to newPrimerDnaConcentration (moles)
    void setPrimerDnaConcentration(const double newPrimerDnaConcentration);
    void setPrimerSearchParameters(const QSharedPointer<PrimerSearchParameters> &primerSearchParameters);
    //! Sets the sodium concentration to newSodiumDnaConcentration (moles)
    void setSodiumConcentration(const double newSodiumDnaConcentration);
    double sodiumConcentration() const;         //!< Returns the sodium concentration


    // ------------------------------------------------------------------------------------------------
    // Public static const members
    static const double kDefaultSodiumMolarity_;       // Moles
    static const double kDefaultPrimerDnaMolarity_;    // Moles


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    double sodiumConcentration_;        //!< [Na+] concentration (moles)
    double primerDnaConcentration_;     //!< Primer DNA concentration (moles)
    QSharedPointer<PrimerSearchParameters> primerSearchParameters_;
};

#endif // PRIMERFACTORY_H
