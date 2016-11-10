/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <QtCore/QVector>

#include "../Entities/IEntity.h"

// Possibly should add paramter to find method for excluding records that are not found
class IRepository
{
public:
    // ------------------------------------------------------------------------------------------------
    // Destructor
    virtual ~IRepository() {}

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual bool addGeneric(const IEntitySPtr &entity, bool ignoreNull) = 0;
    virtual bool addGeneric(const QVector<IEntitySPtr> &entities, bool ignoreNull) = 0;
    virtual void erase(const int id) = 0;
    virtual void erase(const QVector<int> &ids) = 0;
    virtual bool eraseGeneric(const IEntitySPtr &entity) = 0;
    virtual bool eraseGeneric(const QVector<IEntitySPtr> &entities) = 0;
    IEntitySPtr find(const int id)
    {
        return vFind(id);
    }
    QVector<IEntitySPtr> find(const QVector<int> &ids)
    {
        return vFind(ids);
    }
    virtual bool saveAll() = 0;
    virtual bool save(const int id) = 0;
    virtual bool save(const QVector<int> &ids) = 0;
    virtual bool saveGeneric(const IEntitySPtr &entity) = 0;
    virtual bool saveGeneric(const QVector<IEntitySPtr> &entities) = 0;
    virtual bool unerase(const int id) = 0;
    virtual bool unerase(const QVector<int> &ids) = 0;
    virtual bool uneraseGeneric(const IEntitySPtr &entity) = 0;
    virtual bool uneraseGeneric(const QVector<IEntitySPtr> &entities) = 0;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    virtual IEntitySPtr vFind(const int id) = 0;
    virtual QVector<IEntitySPtr> vFind(const QVector<int> &ids) = 0;
};

#endif // IREPOSITORY_H
