/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQMAPPER_H
#define AMINOSEQMAPPER_H

#include "GenericEntityMapper.h"
#include "../Repositories/AnonSeqRepository.h"

class AminoSeq;
struct AminoSeqPod;
class Astring;

class AminoSeqMapper : public GenericEntityMapper<AminoSeq, AminoSeqPod>
{
public:
    AminoSeqMapper(IAdocSource *adocSource, AnonSeqRepository<Astring> *astringRepository);

    bool save(const QVector<AminoSeq *> &aminoSeqs) const;


protected:
    virtual QVector<AminoSeq *> convertPodsToEntities(QVector<AminoSeqPod> &pods) const;


private:
    AnonSeqRepository<Astring> *astringRepository_;
};

#endif // AMINOSEQMAPPER_H
