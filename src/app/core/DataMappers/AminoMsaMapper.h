/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOMSAMAPPER_H
#define AMINOMSAMAPPER_H

#include "MsaMapper.h"
#include "../Entities/AminoMsa.h"
#include "../Entities/AminoSeq.h"

// Disable the virtual inheritance warning about dominant functions
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class AminoMsaMapper : public MsaMapper<AminoMsa, AminoMsaPod, AminoSeq>
{
public:
    AminoMsaMapper(IAdocSource *adocSource, GenericRepository<AminoSeq> *aminoSeqRepository)
        : MsaMapper<AminoMsa, AminoMsaPod, AminoSeq>(adocSource, aminoSeqRepository)
    {
    }
};

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#endif // AMINOMSAMAPPER_H
