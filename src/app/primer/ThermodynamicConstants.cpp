/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ThermodynamicConstants.h"

namespace constants
{
    // The following thermodynamic characteristics are taken from Allawi and Santa Lucia, 1997

    // --------------
    // Enthalpy table
    const double kEnthalpyDimerKcalPerMole[4][4] = {
        // AA       AC       AG       AT
        {  -7.9,    -8.4,    -7.8,    -7.2 },

        // CA       CC       CG       CT
        {  -8.5,    -8.0,    -10.6,   -7.8 },

        // GA       GC       GG       GT
        {  -8.2,    -9.8,    -8.0,    -8.4 },

        // TA       TC       TG       TT
        {  -7.2,    -8.2,    -8.5,    -7.9 }
    };

    //                                              A    C    G    T
    const double kEnthalpyMonomerKcalPerMole[4] = { 2.3, 0.1, 0.1, 2.3 };
    const double kEnthalpySymmetryCorrection = 0.;

    // --------------
    // Entropy table
    const double kEntropyDimerCalPerKPerMole[4][4] = {
        // AA      AC       AG       AT
        {  -22.2,  -22.4,   -21.0,   -20.4 },

        // CA      CC       CG       CT
        {  -22.7,  -19.9,   -27.2,   -21.0 },

        // GA      GC       GG       GT
        {  -22.2,  -24.4,   -19.9,   -22.4 },

        // TA      TC       TG       TT
        {  -21.3,  -22.2,   -22.7,   -22.2 }
    };

    //                                                A    C     G     T
    const double kEntropyMonomerCalPerKPerMole[4] = { 4.1, -2.8, -2.8, 4.1 };
    const double kEntropySymmetryCorrection = -1.4;

    const double kR = 1.987;
}
