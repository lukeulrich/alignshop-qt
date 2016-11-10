/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOANONSEQREPOSITORY_H
#define AMINOANONSEQREPOSITORY_H

#include <QtCore/QHash>

#include "GenericRepository.h"
#include "../Entities/AminoAnonSeq.h"

class AminoAnonSeqMapper;
class Seq;

class AminoAnonSeqRepository : public GenericRepository<AminoAnonSeq>
{
public:
    AminoAnonSeqRepository(AminoAnonSeqMapper *aminoAnonSeqMapper);

    virtual bool add(const QVector<AminoAnonSeq *> &aminoAnonSeqs, bool ignoreNullPointers);
    virtual AminoAnonSeq *findBySeq(const Seq &seq);
    virtual AminoAnonSeq *findBySeqOrCreate(const Seq &seq);
    virtual bool erase(const QVector<AminoAnonSeq *> &aminoAnonSeqs);

    //    virtual void remove(const QVector<AminoAnonSeq *> &aminoAnonSeqs);
private:
    QHash<QByteArray, AminoAnonSeq *> seqIdentityMap_;
};

#endif // AMINOANONSEQREPOSITORY_H
