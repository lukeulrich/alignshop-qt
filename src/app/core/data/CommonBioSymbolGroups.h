/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COMMONBIOSYMBOLGROUPS_H
#define COMMONBIOSYMBOLGROUPS_H

class BioSymbolGroup;

namespace constants
{
    namespace CommonBioSymbolGroups
    {
        extern const char kDefaultConsensusSymbol;
        BioSymbolGroup defaultConsensusSymbolGroup();
        extern const BioSymbolGroup kClustalAminoSymbolGroup;
    }
}

#endif // COMMONBIOSYMBOLGROUPS_H
