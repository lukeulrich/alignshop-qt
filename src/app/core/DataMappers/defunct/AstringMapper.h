/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ASTRINGMAPPER_H
#define ASTRINGMAPPER_H

#include "GenericEntityMapper.h"
#include "AbstractAnonSeqMapper.h"
#include "../DataSources/IAdocSource.h"
#include "../global.h"

class Astring;

class AstringMapper : public GenericEntityMapper<Astring>,
                      public AbstractAnonSeqMapper<Astring>
{
public:
    AstringMapper(IAdocSource *adocSource)
        : GenericEntityMapper<Astring>(adocSource), AbstractAnonSeqMapper<Astring>()
    {
    }

    virtual QVector<Astring *> findByDigests(const QVector<QByteArray> &digests) const
    {
        return adocSource_->crud(static_cast<Astring *>(nullptr))->readByDigests(digests);
    }
};

#endif // ASTRINGMAPPER_H
