/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ENTITYFLAGS_H
#define ENTITYFLAGS_H

namespace Ag
{
    enum EntityFlags {
        eCoreDataFlag = 0x1
    };

    enum AstringFlags {
        eCoilsFlag = 0x2,   // 2
        eSegsFlag =  0x4,   // 4
        eQ3Flag =    0x8    // 8
    };

    enum DnaSeqFlags {
        ePrimersFlag = 0x2  // 2
    };
}

#endif // ENTITYFLAGS_H
