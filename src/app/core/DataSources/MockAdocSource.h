/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKADOCSOURCE_H
#define MOCKADOCSOURCE_H

#include <QtCore/QHash>
#include <QtCore/QCryptographicHash>

#include "AbstractAdocSource.h"
#include "../Seq.h"

// All new records created have id >= 100,000
class MockAdocSource : public AbstractAdocSource
{
public:
    MockAdocSource() : AbstractAdocSource()
    {
        initializeMockObjects();
    }

    QVector<AnonSeqPod> readAminoAnonSeqs(const QVector<int> &ids)
    {
        QVector<AnonSeqPod> pods;
        foreach (int id, ids)
            pods << aminoAnonSeqs_.value(id);

        return pods;
    }

    QVector<AnonSeqPod> readAminoAnonSeqsFromDigests(const QVector<QByteArray> &digests)
    {
        QVector<AnonSeqPod> pods;
        foreach (const QByteArray &digest, digests)
        {
            if (!aminoAnonSeqsDigests_.contains(digest))
            {
                pods << AnonSeqPod();
                continue;
            }

            int id = aminoAnonSeqsDigests_.value(digest);
            ASSERT(aminoAnonSeqs_.contains(id));
            pods << aminoAnonSeqs_.value(id);
        }

        return pods;
    }

    void insertAminoAnonSeq(AnonSeqPod &anonSeqPod)
    {
        ASSERT(anonSeqPod.id_ < 0);
        ASSERT(anonSeqPod.digest_.isEmpty() == false);
        static int nextId = 100000;

        anonSeqPod.id_ = nextId;
        ++nextId;

        if (aminoAnonSeqs_.contains(anonSeqPod.id_))
            throw 0;
        if (aminoAnonSeqsDigests_.contains(anonSeqPod.digest_))
            throw 0;

        aminoAnonSeqs_.insert(anonSeqPod.id_, anonSeqPod);
        aminoAnonSeqsDigests_.insert(anonSeqPod.digest_, anonSeqPod.id_);
    }

    void eraseAminoAnonSeqs(const QVector<int> &ids)
    {
        foreach (int id, ids)
        {
            if (!aminoAnonSeqs_.contains(id))
                continue;

            AnonSeqPod pod = aminoAnonSeqs_.take(id);
            aminoAnonSeqsDigests_.remove(pod.digest_);

            coils_.remove(pod.id_);
            segs_.remove(pod.id_);
        }
    }

    QVector<CoilPod> readCoils(int aminoAnonSeqId, int /* maxStop */)
    {
        if (coils_.contains(aminoAnonSeqId) == false)
            return QVector<CoilPod>();

        return coils_.value(aminoAnonSeqId);
    }

    void saveCoils(int aminoAnonSeqId, QVector<CoilPod> &coilPods)
    {
        coils_[aminoAnonSeqId] = coilPods;
    }

    QVector<SegPod> readSegs(int aminoAnonSeqId, int /* maxStop */)
    {
        if (segs_.contains(aminoAnonSeqId) == false)
            return QVector<SegPod>();

        return segs_.value(aminoAnonSeqId);
    }

    void saveSegs(int aminoAnonSeqId, QVector<SegPod> &segPods)
    {
        segs_[aminoAnonSeqId] = segPods;
    }

    QVector<AminoSeqRecordPod> readAminoSeqRecords(const QVector<int> &ids)
    {
        QVector<AminoSeqRecordPod> pods;
        foreach (int id, ids)
        {
            if (aminoSeqs_.contains(id) == false)
                pods << AminoSeqRecordPod();

            pods << aminoSeqs_.value(id);
        }

        return pods;
    }

    void eraseAminoSeqRecords(const QVector<int> &ids)
    {
        // TODO!!
        qWarning("BLARGH");
    }

    void insertAminoSeqRecord(AminoSeqRecordPod &aminoSeqRecordPod)
    {
        qWarning("BLARGH");
    }

    void saveAminoSeqRecords(QVector<AminoSeqRecordPod> &aminoSeqRecordPods)
    {
        static int nextId = 100000;
        for (QVector<AminoSeqRecordPod>::Iterator it = aminoSeqRecordPods.begin(); it != aminoSeqRecordPods.end(); ++it)
        {
            AminoSeqRecordPod &pod = *it;

            if (pod.id_ < 0)
            {
                // Insert
                ASSERT(aminoSeqs_.contains(nextId) == false);
                pod.id_ = nextId;
                ++nextId;
            }

            aminoSeqs_[pod.id_] = pod;
        }
    }

private:
    void initializeMockObjects()
    {
        Seq seq5("ABCDEFGHIJKLMNOPQRSTUVWXYZ", eAminoGrammar);
        aminoAnonSeqs_.insert(5, AnonSeqPod(5, seq5.asByteArray(), seq5.digest()));
        aminoAnonSeqsDigests_.insert(seq5.digest(), 5);
        Seq seq6("ABCDEF", eAminoGrammar);
        aminoAnonSeqs_.insert(6, AnonSeqPod(6, seq6.asByteArray(), seq6.digest()));
        aminoAnonSeqsDigests_.insert(seq6.digest(), 6);
        Seq seq7("GHIJKLMNOP", eAminoGrammar);
        aminoAnonSeqs_.insert(7, AnonSeqPod(7, seq7.asByteArray(), seq7.digest()));
        aminoAnonSeqsDigests_.insert(seq7.digest(), 7);

        coils_.insert(5, QVector<CoilPod>() << CoilPod(50, 1, 10) << CoilPod(51, 21, 24));
        coils_.insert(7, QVector<CoilPod>() << CoilPod(70, 3, 8));

        segs_.insert(6, QVector<SegPod>() << SegPod(60, 1, 3) << SegPod(61, 4, 6));
        segs_.insert(7, QVector<SegPod>() << SegPod(70, 8, 10));

        aminoSeqs_.insert(15, AminoSeqRecordPod(15, 5, 1, 10, "Domain1", "Non-descript"));
        aminoSeqs_.insert(16, AminoSeqRecordPod(16, 6, 1, 6, "Domain2", "Entire sequence"));
    }

    // Mock data store
    QHash<int, AnonSeqPod> aminoAnonSeqs_;  // amino anon seq id -> AnonSeqPod
    QHash<QByteArray, int> aminoAnonSeqsDigests_;   // Digest -> amino anon seq id
    QHash<int, QVector<CoilPod> > coils_;   // amino anon seq id -> coils
    QHash<int, QVector<SegPod> > segs_;     // amino anon seq id -> segs
    QHash<int, AminoSeqRecordPod> aminoSeqs_;   // amino seq id -> amino seq record pods
};

#endif // MOCKADOCSOURCE_H
