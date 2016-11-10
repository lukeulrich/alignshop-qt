/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSACRUD_H
#define IMSACRUD_H

#include <QtCore/QVector>

#include "IEntityCrud.h"

struct MsaMembersPod;

template<typename T, typename PodT>
class IMsaCrud : public virtual IEntityCrud<T, PodT>
{
public:
    virtual int countMembers(int msaId) = 0;
    virtual MsaMembersPod readMsaMembers(int msaId, int offset, int limit) = 0;
};

#endif // IMSACRUD_H
