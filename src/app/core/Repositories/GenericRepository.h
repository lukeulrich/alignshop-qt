/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GENERICREPOSITORY_H
#define GENERICREPOSITORY_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVector>

#include "MemoryOnlyRepository.h"
#include "../DataMappers/IEntityMapper.h"
#include "../global.h"
#include "../macros.h"

#include <QtDebug>

typedef QPair<int, int> IntPair;

/**
  */
template<typename T>
class GenericRepository : public virtual MemoryOnlyRepository<T>
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    GenericRepository(IEntityMapper<T> *entityMapper);
    ~GenericRepository();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual IEntityMapper<T> *entityMapper() const;

    // Duplicate ids are permitted; however, the reference count will be incremented accordingly (3 of the same id will
    // increase the reference count 3x).
    typename T::SPtr find(const int id);
    QVector<typename T::SPtr> find(const QVector<int> &ids);

    bool save(const QVector<int> &ids);
    virtual bool save(const QVector<typename T::SPtr> &entities);
    virtual bool saveAll();


protected:
    IEntityMapper<T> *entityMapper_;


private:
    QVector<typename T::SPtr> toSmartPointers(const QVector<T *> &rawEntities);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param entityMapper [IEntityMapper<T> *]
  */
template<typename T>
inline
GenericRepository<T>::GenericRepository(IEntityMapper<T> *entityMapper)
    : MemoryOnlyRepository<T>(),
      entityMapper_(entityMapper)
{
    ASSERT(entityMapper_ !=  nullptr);
}

/**
  */
template<typename T>
inline
GenericRepository<T>::~GenericRepository()
{
    QVector<T *> entities;
    typename QHash<int, typename T::SPtr>::ConstIterator it = this->identityHash_.constBegin();
    for (; it != this->identityHash_.constEnd(); ++it)
        entities << boost::shared_static_cast<T>(it.value()).get();

    entityMapper_->teardown(entities);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns IEntityMapper<T> *
  */
template<typename T>
inline
IEntityMapper<T> *GenericRepository<T>::entityMapper() const
{
    return entityMapper_;
}

/**
  * @param id [const int]
  * @returns T::SPtr
  */
template<typename T>
inline
typename T::SPtr GenericRepository<T>::find(const int id)
{
    static QVector<int> ids(1);
    ids[0] = id;
    return find(ids).first();
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<T *>
  */
template<typename T>
inline
QVector<typename T::SPtr> GenericRepository<T>::find(const QVector<int> &ids)
{
    // Helper vectors used to batch fetch records not in repo and preserve their order
    QSet<int> idsNotInRepo;
    QVector<int> indicesOfIdsNotInRepo;

    // Initialize an array of nullptr pointers with the same number of elements as the ids vector
    QVector<typename T::SPtr> entities(ids.size());
    for (int i = 0, z = ids.size(); i<z; ++i)
    {
        int id = ids.at(i);

        // Make sure this entry has not already been "removed' via an erase command. Because we initialized the
        // entities array with null pointers, there is not a need to add one here. In other words, continuing
        // essentially adds a null pointer for this id.
        if (this->softErasedIds_.contains(id))
            continue;

        if (this->identityHash_.contains(id))
        {
            entities[i] = this->identityHash_.value(id);
            continue;
        }

        // Object with this id not in local cache, add to list of ids to request from the data mapper
        idsNotInRepo << id;
        indicesOfIdsNotInRepo << i;
    }

    // There are some ids not present in the repository, fetch them in bulk from the data mapper
    if (idsNotInRepo.size() > 0)
    {
        QVector<typename T::SPtr> fetchedEntities = toSmartPointers(entityMapper_->find(idsNotInRepo.toList().toVector()));
        ASSERT(fetchedEntities.size() == idsNotInRepo.size());
        this->add(fetchedEntities, true); // Ignoring null pointers here
        for (int i=0, z=indicesOfIdsNotInRepo.size(); i<z; ++i)
        {
            int entityId = ids.at(indicesOfIdsNotInRepo.at(i));
            ASSERT(this->identityHash_.value(entityId) != nullptr);
            entities[indicesOfIdsNotInRepo.at(i)] = this->identityHash_.value(entityId);
        }
    }

    return entities;
}

/**
  * No attempt is made to call the save(T::SPtr) method because it will incur additional overhead creating a vector copy
  * of pointers.
  *
  * @param id [const int]
  * @returns bool
  */
template<typename T>
inline
bool GenericRepository<T>::save(const QVector<int> &ids)
{
    // New in this case indicates new entity ids
    QVector<T *> entitiesToSave;
    QVector<int> transientIds;

    foreach (const int id, ids)
    {
        const typename T::SPtr &entity = this->identityHash_.value(id);
        if (!entity)
            return false;

        // Save old id for updating purposes
        if (entity->isNew() || entity->isDirty())
        {
            entitiesToSave << boost::shared_static_cast<T>(entity).get();
            if (entity->isNew())
                transientIds << entity->id();
        }
    }

    // The actual crud interface utilized by the entity mapper is responsible for setting the dirty
    // state to false after the save is successful. It is not performed by GenericRepository::save().
    if (!entityMapper_->save(entitiesToSave))
        return false;

    // Save was successful, update the identityHash with the correct ids for newly inserted entities
    foreach (int oldId, transientIds)
    {
        typename T::SPtr entity = this->identityHash_.take(oldId);
        this->identityHash_.insert(entity->id(), entity);
    }

    return true;
}

/**
  * @param entities [const QVector<T *> &]
  * @returns bool
  */
template<typename T>
inline
bool GenericRepository<T>::save(const QVector<typename T::SPtr> &entities)
{
    // New in this case indicates new entity ids
    QVector<T *> entitiesToSave;
    QVector<int> transientIds;

    for (int i = 0, z = entities.size(); i<z; ++i)
    {
        const typename T::SPtr &entity = entities.at(i);
        ASSERT(entity);
        ASSERT(this->identityHash_.contains(entity->id()));

        // Save old id for updating purposes
        if (entity->isNew() || entity->isDirty())
        {
            entitiesToSave << boost::shared_static_cast<T>(entity).get();
            if (entity->isNew())
                transientIds << entity->id();
        }
    }

    // The actual crud interface utilized by the entity mapper is responsible for setting the dirty
    // state to false after the save is successful. It is not performed by GenericRepository::save().
    if (!entityMapper_->save(entitiesToSave))
        return false;

    // Save was successful, update the identityHash with the correct ids for newly inserted entities
    foreach (int oldId, transientIds)
    {
        typename T::SPtr entity = this->identityHash_.take(oldId);
        this->identityHash_.insert(entity->id(), entity);
    }

    return true;
}

/**
  * @returns bool
  */
template<typename T>
inline
bool GenericRepository<T>::saveAll()
{
    if (!save(this->identityHash_.values().toVector()))
        return false;

    // Now persist the removal of the soft erased entries
    // NOTE: For some stupid reason, I have to use a temporary variable here. I cannot simply call:
    //       entityMapper_->erase(softErased_.values().toVector()).
    QVector<int> idsToErase;
    QVector<T *> temp;
    temp.reserve(this->softErasedIds_.size());
    foreach (const int id, this->softErasedIds_)
    {
        if (this->identityHash_.contains(id))
            temp << boost::shared_static_cast<T>(this->identityHash_.value(id)).get();
        else
            idsToErase << id;
    }

    // Delete all entries that have been erased by ids, but do not reference real objects
    if (idsToErase.size() > 0)
        if (!entityMapper_->erase(idsToErase))
            return false;

    if (temp.size() > 0)
    {
        entityMapper_->teardown(temp);
        entityMapper_->erase(temp);
    }

    MemoryOnlyRepository<T>::removeCruft();

    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param rawEntities [const QVector<T *> &]
  * @param ignoreNull [bool]
  */
template<typename T>
inline
QVector<typename T::SPtr> GenericRepository<T>::toSmartPointers(const QVector<T *> &rawEntities)
{
    // TODO: use boost::make_shared instead of initializing from constructor

    QVector<typename T::SPtr> smartPointers;
    smartPointers.reserve(rawEntities.size());
    foreach (T *entity, rawEntities)
        smartPointers << typename T::SPtr(entity);

    return smartPointers;
}

#endif // GENERICREPOSITORY_H
