/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQ_H
#define DNASEQ_H

#include <QtCore/QVector>

#include "AbstractSeq.h"
#include "Dstring.h"
#include "../UngappedSubseq.h"
#include "../global.h"

#include "../../primer/Primer.h"

struct DnaSeqPod;

/**
  * The private member dstringId_ is not used except during reconstitution
  */
class DnaSeq : public AbstractSeq
{
public:
    static const int kType;

    DnaSeq(int id,
           int start,
           int stop,
           const QString &name,
           const QString &source,
           const QString &description,
           const QString &notes,
           const DstringSPtr &dstring);

    DnaSeq(const DnaSeqPod &dnaSeqPod);
    DnaSeq *clone() const;

    DstringSPtr abstractAnonSeq() const;
    int type() const;

    PrimerVector primers_;

    static DnaSeq *createEntity(const QString &name, const DstringSPtr &dstring);
    static DnaSeq *createEntity(int start,
                                int stop,
                                const QString &name,
                                const QString &source,
                                const QString &description,
                                const QString &notes,
                                const DstringSPtr &dstring);


private:
    static QVector<int> dstringIdVector(const QVector<DnaSeq *> &dnaSeqs);

    friend class DnaSeqMapper;
};

Q_DECLARE_TYPEINFO(DnaSeq, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<DnaSeq> DnaSeqSPtr;

struct DnaSeqPod : public AbstractSeqPod
{
    int dstringId_;
    DstringSPtr dstring_;
    PrimerVector primers_;

    DnaSeqPod(int id = 0) : AbstractSeqPod(id), dstringId_(0)
    {
    }
};

Q_DECLARE_TYPEINFO(DnaSeqPod, Q_MOVABLE_TYPE);

#endif // DNASEQ_H
