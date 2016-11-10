/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IENTITY_H
#define IENTITY_H

#include <boost/shared_ptr.hpp>

class IEntity;
typedef boost::shared_ptr<IEntity> IEntitySPtr;

class IEntity
{
public:
    typedef boost::shared_ptr<IEntity> SPtr;

    virtual ~IEntity() {}

    virtual IEntity *clone() const = 0;

    virtual int id() const = 0;
    virtual bool isDirty() const = 0;
    virtual bool isDirty(const int dirtyFlag) const = 0;
    virtual bool isNew() const = 0;
    virtual void setClean() = 0;
    virtual void setDirty(const int dirtyFlag, const bool dirty) = 0;
    virtual int type() const = 0;
};

uint qHash(const IEntitySPtr &entity);

#endif // IENTITY_H
