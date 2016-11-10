/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IENTITYMAPPER_H
#define IENTITYMAPPER_H

#include <QtCore/QVector>

class IAdocSource;

template<typename T>
class IEntityMapper
{
public:
    virtual ~IEntityMapper()  {}

    virtual IAdocSource *adocSource() const = 0;

    virtual bool erase(const int id) const = 0;
    virtual bool erase(const QVector<int> &ids) const = 0;
    // Erase does not actually free the memory associated in each of the entities; rather it merely
    // removes it permanently from the data store. On success, the id is set to zero to indicate that this entity is no
    // longer valid. If the entity is new, its id is not changed.
    virtual bool erase(T *entity) const = 0;
    virtual bool erase(const QVector<T *> &entities) const = 0;

    virtual T *find(const int id) const = 0;
    virtual QVector<T *> find(const QVector<int> &ids) const = 0;

    virtual bool save(T *entity) const = 0;
    virtual bool save(const QVector<T *> &entities) const = 0;
    // Trigger method for properly uninitializing entity. This may entail such things as removing from other
    // repositories (if entity contains hasMany pointers). It does not free the memory associated with entity. That is
    // the responsibility of the owning repository.
    virtual void teardown(T *entity) const = 0;
    virtual void teardown(const QVector<T *> &entities) const = 0;
};

#endif // IENTITYMAPPER_H
