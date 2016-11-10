/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQ_H
#define AMINOSEQ_H

#include "AbstractSeq.h"
// It is vital to include Astring.h in the header file because otherwise, the compiler will not know that Astring is
// a derivative of AbstractAnonSeq. Consequently, the compiler will generate an error that Astring is an invalid
// covariant return type (for the virtual anonSeq method)!
#include "Astring.h"
#include "../UngappedSubseq.h"
#include "../global.h"

struct AminoSeqPod;

/**
  * The private member astringId_ is not used except during reconstitution
  */
class AminoSeq : public AbstractSeq
{
public:
    static const int kType;

    typedef boost::shared_ptr<AminoSeq> SPtr;

    AminoSeq(int id,
             int start,
             int stop,
             const QString &name,
             const QString &source,
             const QString &description,
             const QString &notes,
             const AstringSPtr &astring);
    AminoSeq(const AminoSeqPod &aminoSeqPod);
    AminoSeq *clone() const;

    AstringSPtr abstractAnonSeq() const;
    int type() const;

    static AminoSeq *createEntity(const QString &name, const AstringSPtr &astring);
    static AminoSeq *createEntity(int start,
                                  int stop,
                                  const QString &name,
                                  const QString &source,
                                  const QString &description,
                                  const QString &notes,
                                  const AstringSPtr &astring);

private:
    static QVector<int> astringIdVector(const QVector<AminoSeq *> &aminoSeqs);

    friend class AminoSeqMapper;
};

Q_DECLARE_TYPEINFO(AminoSeq, Q_MOVABLE_TYPE);


struct AminoSeqPod : public AbstractSeqPod
{
    int astringId_;
    AstringSPtr astring_;

    AminoSeqPod(int id = 0) : AbstractSeqPod(id), astringId_(0)
    {
    }
};

Q_DECLARE_TYPEINFO(AminoSeqPod, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<AminoSeq> AminoSeqSPtr;

#endif // AMINOSEQ_H
