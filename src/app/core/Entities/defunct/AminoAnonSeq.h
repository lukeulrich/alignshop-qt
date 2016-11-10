/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOANONSEQ_H
#define AMINOANONSEQ_H

#include <QtCore/QVector>

#include "AbstractAnonSeq.h"
#include "../ValueObjects/Coil.h"
#include "../ValueObjects/Seg.h"
#include "../ValueObjects/Pfam.h"
#include "../util/DecrementNumberGenerator.h"

class AminoAnonSeq : public AbstractAnonSeq
{
public:
    AminoAnonSeq(int id, const Seq &seq) : AbstractAnonSeq(id, seq)
    {
        ASSERT(seq.grammar() == eAminoGrammar);
    }

    QVector<Coil> coils() const
    {
        return coils_;
    }
    bool addCoil(Coil coil)
    {
        ASSERT(coil.end() <= seq_.length());

        // Only add if unique
        if (coils_.indexOf(coil) != -1)
            return false;

        coil.clearId();
        coils_ << coil;

        setDirty(true);

        return true;
    }
    void removeCoil(int i)
    {
        coils_.remove(i);

        setDirty(true);
    }
    void setCoils(const QVector<Coil> &coils)
    {
        coils_.clear();
        foreach (const Coil &coil, coils)
            addCoil(coil);

        setDirty(true);
    }

    QVector<Seg> segs() const
    {
        return segs_;
    }
    bool addSeg(Seg seg)
    {
        ASSERT(seg.end() <= seq_.length());

        // Only add if unique
        if (segs_.indexOf(seg) != -1)
            return false;

        seg.clearId();
        segs_ << seg;

        setDirty(true);

        return true;
    }
    void removeSeg(int i)
    {
        segs_.remove(i);

        setDirty(true);
    }
    void setSegs(const QVector<Seg> &segs)
    {
        segs_.clear();

        foreach (const Seg &seg, segs)
            addSeg(seg);

        setDirty(true);
    }

    static AminoAnonSeq *create(const Seq &seq)
    {
        ASSERT(seq.grammar() == eAminoGrammar);

        return new AminoAnonSeq(decrementor_.nextValue(), seq);
    }

private:
    static DecrementNumberGenerator decrementor_;

    QVector<Coil> coils_;
    QVector<Seg> segs_;
//    QVector<Pfam> pfams_;

    friend class AminoAnonSeqMapper;
};

#endif // AMINOANONSEQ_H
