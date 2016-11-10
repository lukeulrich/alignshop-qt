/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ANONSEQREPOSITORY_H
#define ANONSEQREPOSITORY_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>

#include <boost/make_shared.hpp>

#include "GenericRepository.h"
#include "IAnonSeqRepository.h"
#include "../DataMappers/IAnonSeqMapper.h"
#include "../Entities/AbstractEntity.h"         // For nextEntityId()
#include "../Seq.h"

// Disable the virtual inheritance warning about dominant functions
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

template<typename T>
class AnonSeqRepository : public virtual GenericRepository<T>,
                          public virtual IAnonSeqRepository<T>
{
public:
    AnonSeqRepository(IAnonSeqMapper<T> *anonSeqMapper)
        : GenericRepository<T>(anonSeqMapper), IAnonSeqRepository<T>(), anonSeqMapper_(anonSeqMapper)
    {
    }

    using GenericRepository<T>::add;
    virtual bool add(const QVector<typename T::SPtr> &anonSeqs, bool ignoreNullPointers)
    {
        if (!GenericRepository<T>::add(anonSeqs, ignoreNullPointers))
            return false;

        foreach (const typename T::SPtr &anonSeq, anonSeqs)
        {
            if (!anonSeq)
            {
                if (ignoreNullPointers)
                    continue;

                return false;
            }

            ASSERT(seqIdentityMap_.contains(anonSeq->seq_.digest()) == false);
            seqIdentityMap_.insert(anonSeq->seq_.digest(), anonSeq);
        }

        return true;
    }

    virtual typename T::SPtr findBySeq(const Seq &seq)
    {
        const QByteArray &digest = seq.digest();
        if (seqIdentityMap_.contains(digest))
        {
            // Since we are finding by an alternative to the id, it is necessary to increase the reference count, which
            // would not otherwise be done.
            return seqIdentityMap_.value(digest);
        }

        // Normally, we would use the protected entityMapper_ instance; however, because of virtual inheritance, this
        // would require a dynamic_cast:
        //
        // dynamic_cast<IAnonSeqMapper *>(this->entityMapper_)
        //
        // Generally speaking, dynamic_casts are bad because they require RTTI among other things. To sidestep this
        // inconvenience, we utilize a local pointer of the proper type obtained upon construction.
        typename T::SPtr anonSeq(anonSeqMapper_->findOneByDigest(digest));
        this->add(anonSeq, true);  // Ignore null pointers
        return anonSeq;
    }

    typename T::SPtr findBySeqOrCreate(const Seq &seq)
    {
        typename T::SPtr anonSeq = findBySeq(seq);
        if (anonSeq)
            return anonSeq;

        // No object found for this sequence, create a new record
        anonSeq.reset(T::createEntity(seq));
        this->add(anonSeq, false);  // Do not ignore null pointers
        return anonSeq;
    }

    virtual bool erase(const QVector<typename T::SPtr> &anonSeqs)
    {
        if (!GenericRepository<T>::erase(anonSeqs))
            return false;

        // Remove digests from seqIdentity Hash
        foreach (const typename T::SPtr &anonSeq, anonSeqs)
        {
            ASSERT(anonSeq);
            ASSERT(seqIdentityMap_.contains(anonSeq->seq_.digest()));
            seqIdentityMap_.remove(anonSeq->seq_.digest());
        }

        return true;
    }

private:
    IAnonSeqMapper<T> *anonSeqMapper_;          // Should be equivalent to the protected entityMapper_ instance,
                                                // but keeping a copy for ourselves avoids the need to make a dynamic
                                                // cast in the findBySeq* methods.
    QHash<QByteArray, typename T::SPtr> seqIdentityMap_;     // Digest -> id
};

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#endif // ANONSEQREPOSITORY_H
