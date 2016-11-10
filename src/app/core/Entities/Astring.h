/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ASTRING_H
#define ASTRING_H

#include <QtCore/QVector>

#include "AbstractAnonSeq.h"
#include "../ValueObjects/Coil.h"
#include "../ValueObjects/Seg.h"
#include "../PODs/Q3Prediction.h"

struct AstringPod;

class Astring : public AbstractAnonSeq
{
public:
    typedef boost::shared_ptr<Astring> SPtr;
    static const int kType;

    Astring(int id, const Seq &seq);
    explicit Astring(const AstringPod &astringPod);

    bool operator==(const Astring &other) const
    {
        return id() == other.id() &&
                seq_ == other.seq_ &&
                coils_ == other.coils_ &&
                segs_ == other.segs_ &&
                q3_ == other.q3_;
    }
    bool operator!=(const Astring &other) const
    {
        return !operator==(other);
    }

    QVector<Coil> coils() const;
    bool addCoil(Coil coil);
    void removeCoil(int i);
    void setCoils(const QVector<Coil> &coils);

    QVector<Seg> segs() const;
    bool addSeg(Seg seg);
    void removeSeg(int i);
    void setSegs(const QVector<Seg> &segs);

    Q3Prediction q3() const;
    void setQ3(const Q3Prediction &q3);
    int type() const;

    static Astring *createEntity(const Seq &seq);

private:
    QVector<Coil> coils_;
    QVector<Seg> segs_;
    Q3Prediction q3_;
};

Q_DECLARE_TYPEINFO(Astring, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<Astring> AstringSPtr;


struct AstringPod : public AbstractAnonSeqPod
{
    QVector<Coil> coils_;
    QVector<Seg> segs_;
    Q3Prediction q3_;

    AstringPod(int id = 0) : AbstractAnonSeqPod(id)
    {
    }
};

Q_DECLARE_TYPEINFO(AstringPod, Q_MOVABLE_TYPE);

#endif // ASTRING_H
