/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQMAPPER_H
#define DNASEQMAPPER_H

#include "GenericEntityMapper.h"
#include "../Repositories/AnonSeqRepository.h"

class DnaSeq;
struct DnaSeqPod;
class Dstring;

class DnaSeqMapper : public GenericEntityMapper<DnaSeq, DnaSeqPod>
{
public:
    DnaSeqMapper(IAdocSource *adocSource, AnonSeqRepository<Dstring> *dstringRepository);

    bool save(const QVector<DnaSeq *> &dnaSeqs) const;

protected:
    virtual QVector<DnaSeq *> convertPodsToEntities(QVector<DnaSeqPod> &pods) const;

private:
    AnonSeqRepository<Dstring> *dstringRepository_;
};

#endif // DNASEQMAPPER_H
