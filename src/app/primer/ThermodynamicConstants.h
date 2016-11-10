/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef THERMODYNAMICCONSTANTS_H
#define THERMODYNAMICCONSTANTS_H

namespace constants
{
    // The following thermodynamic characteristics are taken from Allawi and Santa Lucia, 1997:
    //
    // Allawi, H.T. and J. SantaLucia. 1997. Thermodynamics and NMR of internal GT mismatches in DNA. Biochemistry
    // 36:10581-10594.
    //
    // Santa Lucia, J. 1998. A unified view of polymer, dumbbell, and oligonucleotide DNA nearest - neighbor
    // thermodynamics. Proc Natl Acad Sci USA 95: 1460-1465.
    extern const double kEnthalpyDimerKcalPerMole[4][4];
    extern const double kEnthalpyMonomerKcalPerMole[4];     // Terminal enthalpy
    extern const double kEnthalpySymmetryCorrection;

    extern const double kEntropyDimerCalPerKPerMole[4][4];
    extern const double kEntropyMonomerCalPerKPerMole[4];   // Terminal entropy
    extern const double kEntropySymmetryCorrection;

    // Universal gas constant
    extern const double kR;     // cal per K per mole
}

#endif // THERMODYNAMICCONSTANTS_H
