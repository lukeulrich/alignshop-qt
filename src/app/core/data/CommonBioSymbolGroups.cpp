/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "CommonBioSymbolGroups.h"
#include "../BioSymbolGroup.h"
#include "../BioSymbol.h"
#include "../macros.h"

namespace constants
{
    namespace CommonBioSymbolGroups
    {
        const char kDefaultConsensusSymbol = ' ';
        BioSymbolGroup initializeClustalAminoGroup()
        {
            BioSymbolGroup clustalSymbols;
            clustalSymbols << BioSymbol('%', "WLVIMAFCYHP", .6)
                           << BioSymbol('#', "WLVIMAFCYHP", .8)
                           << BioSymbol('-', "DE", .5)
                           << BioSymbol('+', "KR", .6)
                           << BioSymbol('g', "G", .5)
                           << BioSymbol('n', "N", .5)
                           << BioSymbol('q', "QE", .5)
                           << BioSymbol('p', "P", .5)
                           << BioSymbol('t', "ST", .5)
                           << BioSymbol('A', "A", .85)
                           << BioSymbol('C', "C", .85)
                           << BioSymbol('D', "D", .85)
                           << BioSymbol('E', "E", .85)
                           << BioSymbol('F', "F", .85)
                           << BioSymbol('G', "G", .85)
                           << BioSymbol('H', "H", .85)
                           << BioSymbol('I', "I", .85)
                           << BioSymbol('K', "K", .85)
                           << BioSymbol('L', "L", .85)
                           << BioSymbol('M', "M", .85)
                           << BioSymbol('N', "N", .85)
                           << BioSymbol('P', "P", .85)
                           << BioSymbol('Q', "Q", .85)
                           << BioSymbol('R', "R", .85)
                           << BioSymbol('S', "S", .85)
                           << BioSymbol('T', "T", .85)
                           << BioSymbol('V', "V", .85)
                           << BioSymbol('W', "W", .85)
                           << BioSymbol('Y', "Y", .85);
            return clustalSymbols;
        }
        const BioSymbolGroup kClustalAminoSymbolGroup(initializeClustalAminoGroup());

        BioSymbolGroup defaultConsensusSymbolGroup()
        {
            QByteArray aromatic("FYWH");
            QByteArray aliphatic("IVL");
            QByteArray hydrophobic = aromatic + aliphatic + "AGMCKRT";
            QByteArray positive("HKR");
            QByteArray negative("DE");
            QByteArray charged = positive + negative;
            QByteArray polar = charged + "QNSTC";
            QByteArray alcohol("ST");
            QByteArray tiny("GAS");
            QByteArray small = tiny + "VTDNPC";
            QByteArray turnlike = tiny + polar;

            BioSymbolGroup consensusSymbols;
            consensusSymbols << BioSymbol('G', "G")
                             << BioSymbol('A', "A")
                             << BioSymbol('I', "I")
                             << BioSymbol('V', "V")
                             << BioSymbol('L', "L")
                             << BioSymbol('M', "M")
                             << BioSymbol('F', "F")
                             << BioSymbol('Y', "Y")
                             << BioSymbol('W', "W")
                             << BioSymbol('H', "H")
                             << BioSymbol('C', "C")
                             << BioSymbol('P', "P")
                             << BioSymbol('K', "K")
                             << BioSymbol('R', "R")
                             << BioSymbol('D', "D")
                             << BioSymbol('E', "E")
                             << BioSymbol('Q', "Q")
                             << BioSymbol('N', "N")
                             << BioSymbol('S', "S")
                             << BioSymbol('T', "T")
                             << BioSymbol("aromatic", 'a', aromatic)
                             << BioSymbol("aliphatic", 'l', aliphatic)
                             << BioSymbol("hydrophobic", 'h', hydrophobic)
                             << BioSymbol("positive", '+', positive)
                             << BioSymbol("negative", '-', negative)
                             << BioSymbol("charged", 'c', charged)
                             << BioSymbol("polar", 'p', polar)
                             << BioSymbol("alcohol", 'o', alcohol)
                             << BioSymbol("tiny", 'u', tiny)
                             << BioSymbol("small", 's', small)
                             << BioSymbol("turnlike", 't', turnlike);
            return consensusSymbols;
        }
    }
}

