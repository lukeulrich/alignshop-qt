/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#ifndef THERMODYNAMICCALCULATOR_H
#define THERMODYNAMICCALCULATOR_H

/** Estimates thermodynamic parameters (enthalpy, entropy, melting temperature/Tm) of DNA sequences.
  *
  * Uses nearest-neighbor algorithm to calculate enthalpy and entropy characteristics of a given sequence.
  * Nearest-neighbor thermodynamic values were taken from Santa Lucia(1998) and are identical to those presented
  * by Allawi and Santa Lucia (1997)
  *
  * Two utility functions are provided that will likely be superceded by DnaString/BioString. These two functions
  * set the sequence and return the sequence.
  *
  * Melting temperature (Tm) calculation is key goal of this class. It is returned as a qreal, rounded to the first decimal point.
  * Tm calculation relies upon calculation of enthalpy and entropy. Several functions are dedicated to calculation of enthalpy and entropy. testSymmetry() is provided in order to
  * inform these calculations as defined in the literature. Entropy and enthalpy calculations return respective values.
  */

class ThermodynamicCalculator
{
public:
    //Constructors
    ThermodynamicCalculator();
    //Destructor
    ~ThermodynamicCalculator();

    QString sequence();             // returns sequence
    void setSequence(QString);      // assigns passed QString to sequence_
    qreal calculateEntropy();       // calculates total entropy based on sequence
    qreal calculateEnthalpy();      // calculates total entropy based on sequence
    void sumEntropyAndEnthalpy(qreal &totalEntropy, qreal &totalEnthalpy);    // sum up entropy values across a sequence entropy/enthalpy list
    qreal meltingTemperature(float sodiumConcentration = 1.0);     // calculates Tm
    bool testSymmetry();            // checks sequence for symmetry



private:
    QHash<QString, qreal> nnEnthalpy_;  // nearest neighbor enthalpy values
    QHash<QString, qreal> nnEntropy_;   // nearest neighbor entropy values
    QString sequence_;                  // working sequence
    QList<qreal> sequenceEnthalpy_;     // working sequence enthalpy
    QList<qreal> sequenceEntropy_;      // working sequence entropy
};

inline
ThermodynamicCalculator::~ThermodynamicCalculator()
{

}


#endif // THERMODYNAMICCALCULATOR_H
