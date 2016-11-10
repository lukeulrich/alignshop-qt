/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IDBENTITYCRUD_H
#define IDBENTITYCRUD_H

#include "IEntityCrud.h"

class IDbSource;

template<typename T, typename PodT>
class IDbEntityCrud : public virtual IEntityCrud<T, PodT>
{
protected:
    virtual IDbSource *dbSource() const = 0;
};

#endif // IDBENTITYCRUD_H
