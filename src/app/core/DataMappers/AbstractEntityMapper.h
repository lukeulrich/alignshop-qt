/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTENTITYMAPPER_H
#define ABSTRACTENTITYMAPPER_H

#include "IEntityMapper.h"
#include "../global.h"
#include "../macros.h"

template<typename T, typename PodT>
class AbstractEntityMapper : public virtual IEntityMapper<T>
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    IAdocSource *adocSource() const;

    // Each of the below methods wraps calls to their corresponding virtual vector versions. E.g. erase(int) ->
    // erase(QVector<int>), save(T *) -> save(QVector<T *>).
    using IEntityMapper<T>::erase;
    bool erase(const int id) const;
    bool erase(T *entity) const;
    bool erase(const QVector<T *> &entities) const;

    using IEntityMapper<T>::find;
    T *find(const int id) const;

    using IEntityMapper<T>::save;
    bool save(T *entity) const;

    using IEntityMapper<T>::teardown;
    void teardown(T *entity) const;
    void teardown(const QVector<T *> &entities) const;


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    AbstractEntityMapper(IAdocSource *adocSource);

    // Default method for converting a pod data structure to its object equivalent
    // The pods argument is not const because in some mappers (e.g. AminoSeqMapper), it is necessary to set some of
    // the data members
    virtual QVector<T *> convertPodsToEntities(QVector<PodT> &pods) const;


    IAdocSource *adocSource_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  */
template<typename T, typename PodT>
inline
IAdocSource *AbstractEntityMapper<T, PodT>::adocSource() const
{
    return adocSource_;
}

/**
  * @param id [const int]
  * @returns bool
  */
template<typename T, typename PodT>
inline
bool AbstractEntityMapper<T, PodT>::erase(const int id) const
{
    static QVector<int> ids(1);
    ids[0] = id;
    // Vital to use the 'this' pointer here: see log, 3 May 2011
    return this->erase(ids);
}

/**
  * @param entity [T *]
  * @returns bool
  */
template<typename T, typename PodT>
inline
bool AbstractEntityMapper<T, PodT>::erase(T *entity) const
{
    if (entity == nullptr)
        return true;

    return erase(entity->id());
}

/**
  * @param entities [const QVector<T *> &]
  * @returns bool
  */
template<typename T, typename PodT>
inline
bool AbstractEntityMapper<T, PodT>::erase(const QVector<T *> &entities) const
{
    QVector<int> ids;
    foreach (T *entity, entities)
    {
        if (entity == nullptr || entity->isNew())
            continue;

        ids << entity->id();
    }

    if (!this->erase(ids))
        return false;

    // Update the ids of all new entities to zero
    foreach (T *entity, entities)
        if (entity != nullptr && entity->isNew())
            entity->setId(0);

    return true;
}

/**
  * @param id [const int]
  * @returns T *
  */
template<typename T, typename PodT>
inline
T *AbstractEntityMapper<T, PodT>::find(const int id) const
{
    static QVector<int> ids(1);
    ids[0] = id;
    // Vital to use the 'this' pointer here: see log, 3 May 2011
    return this->find(ids).first();
}

/**
  * @param entity [T *]
  * @returns bool
  */
template<typename T, typename PodT>
inline
bool AbstractEntityMapper<T, PodT>::save(T *entity) const
{
    static QVector<T *> entities(1);
    entities[0] = entity;
    // Vital to use the 'this' pointer here: see log, 3 May 2011
    return this->save(entities);
}

/**
  * @param entity [T *]
  * @returns bool
  */
template<typename T, typename PodT>
inline
void AbstractEntityMapper<T, PodT>::teardown(T *entity) const
{
    static QVector<T *> entities(1);
    entities[0] = entity;
    // Vital to use the 'this' pointer here: see log, 3 May 2011
    teardown(entities);
}

/**
  * @param entities [const QVector<T *> &]
  */
template<typename T, typename PodT>
inline
void AbstractEntityMapper<T, PodT>::teardown(const QVector<T *> & /* entities */) const
{
    // Empty stub - base class method does nothing
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected constructor
/**
  * @param adocSource [IAdocSource *]
  */
template<typename T, typename PodT>
inline
AbstractEntityMapper<T, PodT>::AbstractEntityMapper(IAdocSource *adocSource)
    : adocSource_(adocSource)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param pods [QVector<PodT> &]
  * @returns QVector<T *>
  */
template<typename T, typename PodT>
inline
QVector<T *> AbstractEntityMapper<T, PodT>::convertPodsToEntities(QVector<PodT> &pods) const
{
    QVector<T *> entities(pods.size(), nullptr);
    for (int i=0, z=pods.size(); i<z; ++i)
        if (pods.at(i).isNull() == false)
            entities[i] = new T(pods.at(i));

    return entities;
}

#endif // ABSTRACTENTITYMAPPER_H
