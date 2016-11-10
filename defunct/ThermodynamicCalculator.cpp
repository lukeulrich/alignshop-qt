/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QDebug>
#include <math.h>
#include "ThermodynamicCalculator.h"
#include "DnaString.h"
#include "BioString.h"


ThermodynamicCalculator::ThermodynamicCalculator()
{
    // values are derived from Santa Lucia (1998) and are identical to those presented by Allawi and Santa Lucia (1997)
    nnEnthalpy_["AA"] = -7.9;
    nnEnthalpy_["TT"] = -7.9;
    // AT/TA
    nnEnthalpy_["AT"] = -7.2;
    // TA/AT
    nnEnthalpy_["TA"] = -7.2;
    // CA/GT
    nnEnthalpy_["CA"] = -8.5;
    nnEnthalpy_["TG"] = -8.5;
    // GT/CA
    nnEnthalpy_["GT"] = -8.4;
    nnEnthalpy_["AC"] = -8.4;
    // CT/GA
    nnEnthalpy_["CT"] = -7.8;
    nnEnthalpy_["AG"] = -7.8;
    // GA/CT
    nnEnthalpy_["GA"] = -8.2;
    nnEnthalpy_["TC"] = -8.2;
    // CG/GC
    nnEnthalpy_["CG"] = -10.6;
    // GC/CG
    nnEnthalpy_["GC"] = -9.8;
    // GG/CC
    nnEnthalpy_["GG"] = -8.0;
    nnEnthalpy_["CC"] = -8.0;
    // Initiation and Symmetry Correction
    nnEnthalpy_["InitWithTerminalGC"] = 0.1;
    nnEnthalpy_["InitWithTerminalAT"] = 2.3;
    nnEnthalpy_["SymmetryCorrection"] = 0;

    // Nearest Neighbor Entropy Values
    // AA/TT
    nnEntropy_["AA"] = -22.2;
    nnEntropy_["TT"] = -22.2;
    // AT/TA
    nnEntropy_["AT"] = -20.4;
    // TA/AT
    nnEntropy_["TA"] = -21.3;
    // CA/GT
    nnEntropy_["CA"] = -22.7;
    nnEntropy_["TG"] = -22.7;
    // GT/CA
    nnEntropy_["GT"] = -22.4;
    nnEntropy_["AC"] = -22.4;
    // CT/GA
    nnEntropy_["CT"] = -21.0;
    nnEntropy_["AG"] = -21.0;
    // GA/CT
    nnEntropy_["GA"] = -22.2;
    nnEntropy_["TC"] = -22.2;
    // CG/GC
    nnEntropy_["CG"] = -27.2;
    // GC/CG
    nnEntropy_["GC"] = -24.4;
    // GG/CC
    nnEntropy_["GG"] = -19.9;
    nnEntropy_["CC"] = -19.9;
    // Initiation and Symmetry Correction
    nnEntropy_["InitWithTerminalGC"] = -2.8;
    nnEntropy_["InitWithTerminalAT"] = 4.1;
    nnEntropy_["SymmetryCorrection"] = -1.4;
}


QString ThermodynamicCalculator::sequence()
{
    return sequence_;
}

void ThermodynamicCalculator::setSequence(QString currentSequence)
{
    sequence_ = currentSequence;
}


/* calculateEntropy() scans through the entire sequence for all nearest neighbors; these values are
   assigned to hash with keys corresponding to base position in the sequence; returns the total Entropy sum
  */
qreal ThermodynamicCalculator::calculateEntropy()
{
    sequenceEntropy_.clear();           //remove all items from the hash; start fresh!
    if(sequence_ == ""){return 0;}      //sequence is empty; return 0 value

    QChar base1 = sequence_.at(0);      //first pair will start with the first base in the sequence
    QChar base2;
    qreal totalEntropy = 0;

    // check sequence for symmetry and modify totalEntropy accordingly
    if(testSymmetry()){totalEntropy += nnEntropy_["SymmetryCorrection"];}
    //qDebug() << testSymmetry();
    //qDebug() << totalEntropy;

    // Add 5' terminus-specific entropy values
    if(sequence_.at(0) == 'A' || sequence_.at(0) == 'T'){totalEntropy += nnEntropy_["InitWithTerminalAT"];}
    if(sequence_.at(0) == 'G' || sequence_.at(0) == 'C'){totalEntropy += nnEntropy_["InitWithTerminalGC"];}

    // Add 3' terminus-specific entropy values
    if(sequence_.at(sequence_.length()-1) == 'A' || sequence_.at(sequence_.length()-1) == 'T'){totalEntropy += nnEntropy_["InitWithTerminalAT"];}
    if(sequence_.at(sequence_.length()-1) == 'G' || sequence_.at(sequence_.length()-1) == 'C'){totalEntropy += nnEntropy_["InitWithTerminalGC"];}

    for (int i = 1, z = sequence_.length(); i<z; ++i) // step through the sequence to calculate all nearest neighbors; start at second base in sequence; end at next to last (last base is not paired)
    {
        base2 = sequence_[i];
        QString nn;
        nn.append(base1);
        nn.append(base2);

        if(nnEntropy_.contains(nn))         // assign corresponding entropy value from hash
        {
            sequenceEntropy_.append(nnEntropy_[nn]);
            //sequenceEntropy_[i] = nnEntropy_[nn];
            //qDebug() << nn << sequenceEntropy_;
            totalEntropy = totalEntropy + nnEntropy_[nn];
        }
        //else qDebug()<<"ERROR:" <<nn << "is not present in the nnEntropy hash!";               //an error catcher just to make sure that all pairs are present in the nn hash
        base1 = base2;                              //base2 is now base1 and the loop moves forward
        //qDebug() << "Total Entropy elements: " << currentSequenceEntropy_.count() << nn;
    }

    //qDebug() << totalEntropy;
    return totalEntropy;
}


/* calculateEnthalpy() scans through the entire sequence for all nearest neighbors; these values are
   assigned to hash with keys corresponding to base position in the sequence; returns the total Enthalpy sum
  */
qreal ThermodynamicCalculator::calculateEnthalpy()
{
    sequenceEnthalpy_.clear();          //remove all items from the hash; start fresh!
    if(sequence_ == ""){return 0;}      //sequence is empty; return 0 value

    QChar base1 = sequence_.at(0);      //first pair will start with the first base in the sequence
    QChar base2;
    qreal totalEnthalpy = 0;

    // no need to test for symmetry here as symmetry does not impact enthalpy
    // Add 5' terminus-specific enthalpy values
    if(sequence_.at(0) == 'A' || sequence_.at(0) == 'T'){totalEnthalpy += nnEnthalpy_["InitWithTerminalAT"];}
    if(sequence_.at(0) == 'G' || sequence_.at(0) == 'C'){totalEnthalpy += nnEnthalpy_["InitWithTerminalGC"];}

    // Add 3' terminus-specific enthalpy values
    if(sequence_.at(sequence_.length()-1) == 'A' || sequence_.at(sequence_.length()-1) == 'T'){totalEnthalpy += nnEnthalpy_["InitWithTerminalAT"];}
    if(sequence_.at(sequence_.length()-1) == 'G' || sequence_.at(sequence_.length()-1) == 'C'){totalEnthalpy += nnEnthalpy_["InitWithTerminalGC"];}

    //qDebug() << totalEnthalpy;
    for (int i = 1, z = sequence_.length(); i<z; ++i) //step through the sequence to calculate all nearest neighbors; start at second base in sequence; end at next to last (last base is not paired)
    {
        base2 = sequence_[i];
        QString nn;
        nn.append(base1);
        nn.append(base2);

        if(nnEnthalpy_.contains(nn)) //assign corresponding entropy value from hash
        {
            sequenceEnthalpy_.append(nnEnthalpy_[nn]);
            //sequenceEnthalpy_[i] = nnEnthalpy_[nn];
            //qDebug() << nn << sequenceEnthalpy_;
            totalEnthalpy = totalEnthalpy + nnEnthalpy_[nn];
            //qDebug() << totalEnthalpy;
        }
        //else qDebug()<<"ERROR:" <<nn << "is not present in the nnEntropy hash!";               //an error catcher just to make sure that all pairs are present in the nn hash
        base1 = base2;                              //base2 is now base1 and the loop moves forward
        //qDebug() << "Total Entropy elements: " << currentSequenceEntropy_.count() << nn;
    }
    //qDebug() << totalEnthalpy;
    return totalEnthalpy;
}


/* sumEntropyAndEnthalpy() is a leaner version of calculateEntropy() and calculateEnthalpy(). It accesses the objects thermodynamic QLIST and
   sums up the total entropy and enthalpy values for a given sequence; it is passed the references of variables declared from the calling function;
   the total thermodynamic values are then assigned to these memory addresses
*/
void ThermodynamicCalculator::sumEntropyAndEnthalpy(qreal &totalEntropy, qreal &totalEnthalpy)
{
    for(int i = 0, z = sequence_.length()-1; i<z; ++i)     //cycles through the nearest neighbor pairs in the primer set and sums
    {
        totalEntropy += sequenceEntropy_[i];
        totalEnthalpy += sequenceEnthalpy_[i];
    }

    // Add 5' terminus-specific enthalpy values
    if(sequence_.at(0) == 'A' || sequence_.at(0) == 'T'){totalEnthalpy += nnEnthalpy_["InitWithTerminalAT"];}
    if(sequence_.at(0) == 'G' || sequence_.at(0) == 'C'){totalEnthalpy += nnEnthalpy_["InitWithTerminalGC"];}

    // Add 3' terminus-specific enthalpy values
    if(sequence_.at(sequence_.length()-1) == 'A' || sequence_.at(sequence_.length()-1) == 'T'){totalEnthalpy += nnEnthalpy_["InitWithTerminalAT"];}
    if(sequence_.at(sequence_.length()-1) == 'G' || sequence_.at(sequence_.length()-1) == 'C'){totalEnthalpy += nnEnthalpy_["InitWithTerminalGC"];}

    // Add 5' terminus-specific entropy values
    if(sequence_.at(0) == 'A' || sequence_.at(0) == 'T'){totalEntropy += nnEntropy_["InitWithTerminalAT"];}
    if(sequence_.at(0) == 'G' || sequence_.at(0) == 'C'){totalEntropy += nnEntropy_["InitWithTerminalGC"];}

    // Add 3' terminus-specific entropy values
    if(sequence_.at(sequence_.length()-1) == 'A' || sequence_.at(sequence_.length()-1) == 'T'){totalEntropy += nnEntropy_["InitWithTerminalAT"];}
    if(sequence_.at(sequence_.length()-1) == 'G' || sequence_.at(sequence_.length()-1) == 'C'){totalEntropy += nnEntropy_["InitWithTerminalGC"];}
}

/* meltingTemperature returns the Tm (in degrees Celsius) of a given sequence
   returns a qreal that has been rounded to the first decimal place
*/
qreal ThermodynamicCalculator::meltingTemperature(float sodiumConcentration)
{
    qreal R = 1.987;                            // universal gas constant, 1.987 cal / (K * mol)
    qreal primerConcentration = 0.000001;       // Primer concentration in molarity; 1 uM is a pretty good default; user may need control over this sometime down the line
    int symmetryConstant = 4;                   // for nonself complementary = 4; self-complementary = 1; this may need to be = 2 for PCR calc's
    if(testSymmetry()){symmetryConstant = 1;}   // check for symmetry and adjust constant if needed

    qreal numerator = 1000*calculateEnthalpy() - 0.114 * (sequence_.length() -1) * log(static_cast<double>(sodiumConcentration)); // includes salt adjustment per Santa Lucia et al (1998)
    qreal denominator = calculateEntropy() + 0.368 * (sequence_.length()-1) * log(static_cast<double>(sodiumConcentration)) + R * log(primerConcentration/symmetryConstant); //currently the default concentration is 1 uM (0.000001)

    //2011-05-18 Paul: Other salt correction formulae
    //qreal saltCorrection = 16.6 * log10(static_cast<double>(sodiumConcentration)); //salt correction by SchiltKraut and Lilson (1996)
    //qreal saltCorrection = 12.5 * log10(static_cast<double>(sodiumConcentration)); //salt correction by Santa Lucia et al (1996)
    //qreal tm = numerator / denominator + saltCorrection - 273.15;

    qreal tm = numerator / denominator - 273.15; // simplified Tm calculation; verified against melting 4.3 server at http://www.ebi.ac.uk/compneur-srv/melting/melt.php
    //qDebug() << sequence() << "Numerator: " << numerator << "Denominator: " << denominator <<  "Tm: " << tm;



//    tm = round(tm*10)/10;                       // round the Tm to one decimal place
    // EDIT (Luke, 30 July 2010): Windows does not support the round function, thus
    //      to make cross-platform compatible, we use a hack that requires only floor
    tm = floor(tm*10. + .5)/10.;                       // round the Tm to one decimal place
    //qDebug() << "Tm " << tm;
    return tm;
}

// testSymmetry() determines if the sequence is "symmetrical" (pallindromic). If so, the symmetry constant is updated in the entropy and enthalpy calculations
bool ThermodynamicCalculator::testSymmetry()
{
    DnaString c = sequence();
    QString reverseComplement = c.reverseComplement();
    if(reverseComplement == sequence())
    {
       return 1;
    }

    else{return 0;}
}
