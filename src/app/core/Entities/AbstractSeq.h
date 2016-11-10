/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTSEQ_H
#define ABSTRACTSEQ_H

#include <QtCore/QString>

#include "AbstractBasicEntity.h"
#include "AbstractAnonSeq.h"

class Seq;

struct AbstractSeqPod;

class AbstractSeq : public AbstractBasicEntity
{
public:
    typedef boost::shared_ptr<AbstractSeq> SPtr;

    AbstractAnonSeqSPtr abstractAnonSeq() const;
    BioString parentBioString() const;
    BioString bioString() const;

    int length() const;
    ClosedIntRange range() const;
    int seqLength() const;                          //!< Returns the total length of the parent seq object
    int start() const;
    int stop() const;
    QString source() const;

    bool setStart(int start);
    bool setStop(int stop);

    void setSource(const QString &source);

protected:
    AbstractSeq(int id,
                int start,
                int stop,
                const QString &name,
                const QString &source,
                const QString &description,
                const QString &notes,
                const AbstractAnonSeqSPtr &abstractAnonSeq);
    // These are protected fields rather than public so that when they are changed, we can toggle the dirty status
    // of this object
    int start_;
    int stop_;
    QString source_;
    AbstractAnonSeqSPtr abstractAnonSeq_;

    // TODO: Implement relative start
//    int relativeStart_;
};
Q_DECLARE_TYPEINFO(AbstractSeq, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<AbstractSeq> AbstractSeqSPtr;


struct AbstractSeqPod : public AbstractBasicEntityPod
{
    int start_;
    int stop_;
    QString source_;

    AbstractSeqPod(int id = 0)
        : AbstractBasicEntityPod(id), start_(0), stop_(-1)
    {
    }
};

Q_DECLARE_TYPEINFO(AbstractSeqPod, Q_MOVABLE_TYPE);

#endif // ABSTRACTSEQ_H
