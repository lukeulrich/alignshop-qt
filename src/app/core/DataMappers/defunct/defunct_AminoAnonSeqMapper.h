/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOANONSEQMAPPER_H
#define AMINOANONSEQMAPPER_H

#include <QtCore/QVector>

#include "AbstractDataMapper.h"
#include "CoilMapper.h"
#include "SegMapper.h"

class AnonSeqPod;
class AminoAnonSeq;

class AminoAnonSeqMapper : public AbstractDataMapper<AminoAnonSeq>
{
public:
    AminoAnonSeqMapper(IAdocSource *adocSource);

    QVector<AminoAnonSeq *> find(const QVector<int> &ids) const;
    AminoAnonSeq *findOneByDigest(const QByteArray &digest) const;
    QVector<AminoAnonSeq *> findByDigests(const QVector<QByteArray> &digests) const;
    bool save(const QVector<AminoAnonSeq *> &aminoAnonSeqs) const;

private:
    // Helper function to avoid typing the same code
    QVector<AminoAnonSeq *> reconstituteFromPods(const QVector<AnonSeqPod> &aminoAnonSeqPods) const;
    void erase(const QVector<int> &ids) const;

    CoilMapper coilMapper_;
    SegMapper segMapper_;
};

#endif // AMINOANONSEQMAPPER_H
