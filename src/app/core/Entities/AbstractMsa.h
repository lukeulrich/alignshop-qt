/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSA_H
#define ABSTRACTMSA_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "AbstractBasicEntity.h"
#include "../enums.h"

class ObservableMsa;
class Subseq;

/**
  * How to capture changes to the msa and toggle this nodes dirty status? Perhaps unnecessary because all changes to the
  * msa should be handled within the Msa window class.
  */
class AbstractMsa : public AbstractBasicEntity
{
public:
    typedef boost::shared_ptr<AbstractMsa> SPtr;

    ~AbstractMsa();

    void addDeadSubseqs(const QVector<Subseq *> subseqs);
    void clearDeadSubseqs();                                    //!< Frees the memory utilized by the deadSubseqs vector and clears it
    QVector<Subseq *> deadSubseqs() const;
    ObservableMsa *msa() const;
    void setMsa(ObservableMsa *msa);            //!< Takes ownership of msa
    virtual Grammar grammar() const = 0;

protected:
    AbstractMsa(int id,
                const QString &name,
                const QString &description,
                const QString &notes);

    ObservableMsa *msa_;
    QVector<Subseq *> deadSubseqs_;
};
Q_DECLARE_TYPEINFO(AbstractMsa, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<AbstractMsa> AbstractMsaSPtr;

struct AbstractMsaPod : public AbstractBasicEntityPod
{
    AbstractMsaPod(int id) : AbstractBasicEntityPod(id)
    {
    }
};
Q_DECLARE_TYPEINFO(AbstractMsaPod, Q_MOVABLE_TYPE);

struct MsaMembersPod
{
    QVector<int> seqIds_;
    QVector<QByteArray> gappedSequences_;
};
Q_DECLARE_TYPEINFO(MsaMembersPod, Q_MOVABLE_TYPE);

#endif // ABSTRACTMSA_H
