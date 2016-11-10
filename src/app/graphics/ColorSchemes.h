/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COLORSCHEMES_H
#define COLORSCHEMES_H

class BioSymbolGroup;
class CharColorScheme;
class LinearColorScheme;
class SymbolColorScheme;

namespace ColorSchemes
{
    extern const SymbolColorScheme kClustalAminoScheme;
    extern const CharColorScheme kZappoAminoScheme;
    extern const CharColorScheme kTaylorAminoScheme;
    extern const CharColorScheme kHydrophobocityScheme;
    extern const CharColorScheme kHelixPropensityScheme;
    extern const CharColorScheme kStrandPropensityScheme;
    extern const CharColorScheme kTurnPropensityScheme;
    extern const CharColorScheme kBuriedIndexScheme;
    extern const LinearColorScheme kSecondaryStructureScheme;

    extern const CharColorScheme kClustalDnaScheme;

    extern const CharColorScheme kLogoAminoScheme;
}

#endif // COLORSCHEMES_H
