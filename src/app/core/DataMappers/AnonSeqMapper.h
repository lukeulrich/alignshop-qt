/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ANONSEQMAPPER_H
#define ANONSEQMAPPER_H

#include "GenericEntityMapper.h"
#include "AbstractAnonSeqMapper.h"
#include "../global.h"

// Disable the virtual inheritance warning about dominant functions
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

template<typename T, typename PodT>
class AnonSeqMapper : public GenericEntityMapper<T, PodT>,
                      public AbstractAnonSeqMapper<T>
{
public:
    AnonSeqMapper(IAdocSource *adocSource)
        : GenericEntityMapper<T, PodT>(adocSource), AbstractAnonSeqMapper<T>()
    {
    }

    virtual QVector<T *> findByDigests(const QVector<QByteArray> &digests) const
    {
        GenericEntityMapper<T, PodT>::adocSource_->begin();
        QVector<PodT> pods = GenericEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->readByDigests(digests);
        QVector<T *> entities = GenericEntityMapper<T, PodT>::convertPodsToEntities(pods);
        GenericEntityMapper<T, PodT>::adocSource_->end();

        return entities;
//        return GenericEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->readByDigests(digests);
    }
};

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#endif // ABSTRACTANONSEQMAPPER_H
