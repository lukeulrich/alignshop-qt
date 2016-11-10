/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAMSAMAPPER_H
#define DNAMSAMAPPER_H

#include "MsaMapper.h"
#include "../Entities/DnaMsa.h"
#include "../Entities/DnaSeq.h"

// Disable the virtual inheritance warning about dominant functions
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class DnaMsaMapper : public MsaMapper<DnaMsa, DnaMsaPod, DnaSeq>
{
public:
    DnaMsaMapper(IAdocSource *adocSource, GenericRepository<DnaSeq> *dnaSeqRepository)
        : MsaMapper<DnaMsa, DnaMsaPod, DnaSeq>(adocSource, dnaSeqRepository)
    {
    }
};

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#endif // DNAMSAMAPPER_H
