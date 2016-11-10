/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOMSAMAPPER_H
#define AMINOMSAMAPPER_H

#include "GenericEntityMapper.h"
#include "IMsaMapper.h"
#include "../Repositories/GenericRepository.h"

class AminoMsa;
struct AminoMsaPod;
class AminoSeq;
class Subseq;

class AminoMsaMapper : public GenericEntityMapper<AminoMsa, AminoMsaPod>,
                       public IMsaMapper<AminoMsa>
{
public:
    AminoMsaMapper(IAdocSource *adocSource, GenericRepository<AminoSeq> *aminoSeqRepository);

    virtual int beginLoadAlignment(AminoMsa *aminoMsa);
    virtual void cancelLoadAlignment();
    virtual void endLoadAlignment();
    virtual int loadAlignmentStep(int stepsToTake);

private:
    struct MemberLoadRequest {
        QVector<Subseq *> subseqsLoading_;
        int totalMembers_;

        MemberLoadRequest(int totalMembers) : totalMembers_(totalMembers)
        {
        }

        void reset()
        {

        }
    };

    MemberLoadRequest memberLoadRequest_;
};

#endif // AMINOMSAMAPPER_H
