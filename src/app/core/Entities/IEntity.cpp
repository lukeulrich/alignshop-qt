/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QHash>

#include "IEntity.h"

uint qHash(const IEntitySPtr &entity)
{
    return ::qHash(entity.get());
}
