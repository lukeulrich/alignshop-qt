/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTDBENTITYCRUD_H
#define ABSTRACTDBENTITYCRUD_H

#include <QtCore/QHash>

#include "IDbEntityCrud.h"

class IDbSource;

template<typename T, typename PodT>
class AbstractDbEntityCrud : public IDbEntityCrud<T, PodT>
{
public:
    virtual IDbSource *dbSource() const
    {
        return dbSource_;
    }

protected:
    AbstractDbEntityCrud(IDbSource *dbSource) : dbSource_(dbSource)
    {
    }

    IDbSource *dbSource_;
};

#endif // ABSTRACTDBENTITYCRUD_H
