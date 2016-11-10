/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MEMORYONLYREPOSITORY_H
#define MEMORYONLYREPOSITORY_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVector>

#include "IRepository.h"
#include "../global.h"
#include "../macros.h"

#include <QtDebug>

typedef QPair<int, int> IntPair;

/**
  * Issues:
  * o If an entity is erased (really marked for deletion and moved out of the local cache), what happens if an add
  *   method with this pointer is called?
  *   >> Answer: Should not be added and add returns false
  *
  * o What happens if entity is erased (stored in the softErased array) and then "found" again via find?
  *   >> Answer: The find method must return a nullptr pointer because while it still exists in the data store, it does
  *              not exist at the application level.
  *
  * o What happens if an entity is removed that contains a pointer to another entity contained in another repository?
  *   >> Answer: the entry in the related repository must be "unfound" so that it may properly manage its in-memory
  *              collection and update its reference count. This should be performed via the dataMapper in its teardown
  *              method.
  *
  * o Should an entity managed by a repository ever be deleted outside of the repository?
  *   >> Answer: NEVER! Not even for a data mapper. That violates a major principle behind repositories. Namely, that
  *              they are the strict owners of an entity - regardless of its associations with other entities.
  *
  * o What happens when an entity is "erased" (softly) but still contains external references?
  *   >> Answer: the object pointers are still valid; however, increasing (find) or decreasing (unfind) the reference
  *              count is no longer possible. If saveAll is called, all object pointers are freed and no longer valid.
  *              Thus, the best strategy is to treat a pointer as null once it has been erased. Of course, unerasing
  *              it without the pointer instance is not possible.
  *
  * o What happens if an object is freed yet still has external references?
  *
  * o Is it possible to unfind an instance after it has been "erased"?
  *   >> Answer: Yes, along as it's reference count is not zero
  *
  * o Does the successful addition of an entity increase its reference count?
  *   >> Answer: Yes, because the calling code still has a reference to that instance. If it is to be released, it must
  *              be unfound.
  *
  * o What happens when an entity is erased with a zero reference count but the objects still exist elsewhere? In
  *   particular, for the data tree? The data tree has a sequence which in turn references an astring; however, this
  *   sequence has not yet been loaded, but the astring has. Therefore, it appears safe to remove the astring because
  *   it has a zero-reference count. Then the user attempts to load the sequence which no longer has a valid astring. Is
  *   this just application controlled?
  *   >> Answer: make the erase stub for the IEntityCrud do nothing.
  *
  * o When an entity is added to the repository, should its reference count be increased to one at this time?
  *   >> Answer: Yes, because the calling code that added it currently has a reference to it. Thus, its reference count
  *      is 1. May seem unintuitive in the calling code to add some entities and then immediately unfind them if it does
  *      not need them right away.
  *
  * o When an entity is unerased, should its reference count be incremented?
  *   >> Answer: No. Erase/Unerase does not have any impact on the reference count (only applies for softerase).
  *              Currently, this is a problem because unerase utilizes the add method which increases the reference
  *              count (see previous question). Using add is also advantageous because it provides for virtual actions
  *              performed by more specialized repositories (e.g. AnonSeqRepository).
  *
  *      Solution: simply decrement the reference count of all entities to be unerased before they are "added" back into
  *                the available pool of entities to be found. See notes to the unerase method.
  *
  * Dangling entity = entity that has been erased from the data source, yet at least one reference in memory to this
  *                   object remains in memory.
  *
  * TODO: Garbage collection - remove all entities with reference count of zero. Currently they remain in memory
  */
template<typename T>
class MemoryOnlyRepository : public virtual IRepository
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    ~MemoryOnlyRepository();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    // Succeed only if all entities are:
    // 1) Not already present in repo based on its id
    // 2) Not erased
    // Action is atomic - meaning all are added or none are added (if there is an error)
    bool addGeneric(const IEntitySPtr &entity, bool ignoreNull);
    virtual bool add(const typename T::SPtr &entity, bool ignoreNull);
    bool addGeneric(const QVector<IEntitySPtr> &entities, bool ignoreNull);
    virtual bool add(const QVector<typename T::SPtr> &entities, bool ignoreNull);

    void erase(const int id);
    void erase(const QVector<int> &ids);
    bool eraseGeneric(const IEntitySPtr &entity);
    bool erase(const typename T::SPtr &entity);
    bool eraseGeneric(const QVector<IEntitySPtr> &entities);
    bool erase(const QVector<typename T::SPtr> &entities);

    // The following two find methods are declared virtual here because they are not virtual in IRepository. The reason
    // for this is that the non-virtual interface idiom is used in relation to the base class; however, they need to be
    // virtual here so that derived classes (e.g. GenericRepository) may perform additional processing as needed.
    virtual typename T::SPtr find(const int id);
    // Duplicate ids are permitted
    virtual QVector<typename T::SPtr> find(const QVector<int> &ids);

    virtual void removeCruft();

    bool unerase(const int id);
    bool unerase(const QVector<int> &ids);
    bool uneraseGeneric(const IEntitySPtr &entity);
    bool unerase(const typename T::SPtr &entity);
    bool uneraseGeneric(const QVector<IEntitySPtr> &entities);
    bool unerase(const QVector<typename T::SPtr> &entities);


protected:
    // Not allowed to save with this class version; all these methods simply return false
    bool save(const int id);
    bool save(const QVector<int> &ids);
    bool saveGeneric(const IEntitySPtr &entity);
    virtual bool save(const typename T::SPtr &entity);
    bool saveGeneric(const QVector<IEntitySPtr> &entities);
    virtual bool save(const QVector<typename T::SPtr> &entities);
    bool saveAll();

    QHash<int, typename T::SPtr> identityHash_;     // {entity id => entity pointer}
    QSet<int> softErasedIds_;                       // Set of all ids that have been erased


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    IEntitySPtr vFind(const int id);
    QVector<IEntitySPtr> vFind(const QVector<int> &ids);

    bool localAdd(const typename T::SPtr &entity, bool ignoreNull);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  */
template<typename T>
inline
MemoryOnlyRepository<T>::~MemoryOnlyRepository()
{
#ifdef QT_DEBUG
    // If we have properly shutdown this class, there should be no outstanding references.
    typename QHash<int, typename T::SPtr>::ConstIterator it = identityHash_.constBegin();
    while (it != identityHash_.constEnd())
    {
        if (!(*it).unique())
            qDebug() << Q_FUNC_INFO << (*it).use_count();
        ASSERT((*it).unique());
        ++it;
    }
#endif
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param entity [const IEntitySPtr &]
  * @param ignoreNull [bool]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::addGeneric(const IEntitySPtr &entity, bool ignoreNull)
{
    if (!entity)
        return ignoreNull;

    if (entity->type() != T::kType || softErasedIds_.contains(entity->id()))
        return false;

    ASSERT(boost::dynamic_pointer_cast<T>(entity));

    identityHash_.insert(entity->id(), boost::static_pointer_cast<T>(entity));

    return true;
}

/**
  * @param entity [const T::SPtr &]
  * @param ignoreNull [bool]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::add(const typename T::SPtr &entity, bool ignoreNull)
{
    // Note: a static vector is not used here in order to avoid hanging onto a shared_ptr reference
    QVector<typename T::SPtr> entities(1);
    entities[0] = entity;
    return this->add(entities, ignoreNull);
}

/**
  * @param entities [const QVector<IEntitySPtr> &]
  * @param ignoreNull [bool]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::addGeneric(const QVector<IEntitySPtr> &entities, bool ignoreNull)
{
    typename QVector<IEntitySPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const IEntitySPtr &entity = *it;
        if (!addGeneric(entity, ignoreNull))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                identityHash_.remove((*it)->id());
            }

            return false;
        }
    }

    return true;
}

/**
  * @param entities [const QVector<typename T::SPtr> &]
  * @param ignoreNull [bool]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::add(const QVector<typename T::SPtr> &entities, bool ignoreNull)
{
    typename QVector<typename T::SPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const typename T::SPtr &entity = *it;
        if (!localAdd(entity, ignoreNull))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                identityHash_.remove((*it)->id());
            }

            return false;
        }
    }

    return true;
}

/**
  * @param id [const int]
  */
template<typename T>
inline
void MemoryOnlyRepository<T>::erase(const int id)
{
    softErasedIds_ << id;
}

/**
  * @param ids [const QVector<int> &]
  */
template<typename T>
inline
void MemoryOnlyRepository<T>::erase(const QVector<int> &ids)
{
    foreach (const int id, ids)
        softErasedIds_ << id;
}

/**
  * @param entity [const IEntitySPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::eraseGeneric(const IEntitySPtr &entity)
{
    // 1. entity must not be null
    if (!entity)
        return false;

    ASSERT(entity->type() == T::kType);
    ASSERT(boost::dynamic_pointer_cast<T>(entity));

    // 2. entity must have been previously "found" - that is, it is present in the identityHash
    if (!identityHash_.contains(entity->id()))
        return false;

    softErasedIds_ << entity->id();

    return true;
}

/**
  * @param entity [const typename T::SPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::erase(const typename T::SPtr &entity)
{
    if (!entity)
        return false;

    if (!identityHash_.contains(entity->id()))
        return false;

    softErasedIds_ << entity->id();

    return true;
}

/**
  * @param entities [const QVector<IEntitySPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::eraseGeneric(const QVector<IEntitySPtr> &entities)
{
    typename QVector<IEntitySPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const IEntitySPtr &entity = *it;
        // If at any time erase fails, revert all changes prior to this call and return false
        if (!eraseGeneric(entity))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                softErasedIds_.remove((*it)->id());
            }

            return false;
        }
    }

    return true;
}

/**
  * @param entities [const QVector<typename T::SPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::erase(const QVector<typename T::SPtr> &entities)
{
    typename QVector<typename T::SPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const typename T::SPtr &entity = *it;
        // If at any time erase fails, revert all changes prior to this call and return false
        if (!erase(entity))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                softErasedIds_.remove((*it)->id());
            }

            return false;
        }
    }

    return true;
}

/**
  * @param id [const int]
  * @returns T::SPtr
  */
template<typename T>
inline
typename T::SPtr MemoryOnlyRepository<T>::find(const int id)
{
    if (!softErasedIds_.contains(id) && identityHash_.contains(id))
        return identityHash_.value(id);

    return typename T::SPtr();
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<T::SPtr>
  */
template<typename T>
inline
QVector<typename T::SPtr> MemoryOnlyRepository<T>::find(const QVector<int> &ids)
{
    QVector<typename T::SPtr> found;
    found.reserve(ids.size());
    foreach (const int id, ids)
        found << find(id);
    return found;
}

/**
  * Persist release of the soft erased entries.
  */
template<typename T>
inline
void MemoryOnlyRepository<T>::removeCruft()
{
    // ISSUE? Check for "dangling" entities - those that have been erased, yet still have outstanding references (ie.
    // T::SPtr.use_count() > 1
    foreach (const int id, softErasedIds_)
        identityHash_.remove(id);

    softErasedIds_.clear();
}

/**
  * @param id [const int]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::unerase(const int id)
{
    if (!softErasedIds_.contains(id))
        return false;

    softErasedIds_.remove(id);
    return true;
}

/**
  * @param ids [const QVector<int> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::unerase(const QVector<int> &ids)
{
    QVector<int>::ConstIterator it = ids.constBegin();
    for (; it != ids.constEnd(); ++it)
    {
        if (!unerase(*it))
        {
            while (it != ids.constBegin())
            {
                --it;
                erase(*it);
            }

            return false;
        }
    }
    return true;
}

/**
  * @param entity [const IEntitySPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::uneraseGeneric(const IEntitySPtr &entity)
{
    if (!entity)
        return false;

    ASSERT(entity->type() == T::kType);
    ASSERT(boost::dynamic_pointer_cast<T>(entity));

    if (!softErasedIds_.contains(entity->id()))
        return false;

    softErasedIds_.remove(entity->id());

    return true;
}

/**
  * @param entity [const typename T::SPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::unerase(const typename T::SPtr &entity)
{
    if (!entity)
        return false;

    if (!softErasedIds_.contains(entity->id()))
        return false;

    softErasedIds_.remove(entity->id());

    return true;
}

/**
  * @param entities [const QVector<IEntitySPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::uneraseGeneric(const QVector<IEntitySPtr> &entities)
{
    typename QVector<IEntitySPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const IEntitySPtr &entity = *it;
        if (!uneraseGeneric(entity))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                softErasedIds_ << (*it)->id();
            }

            return false;
        }
    }

    return true;
}

/**
  * @param entities [const QVector<typename T::SPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::unerase(const QVector<typename T::SPtr> &entities)
{
    typename QVector<typename T::SPtr>::ConstIterator it = entities.constBegin();
    for (; it != entities.constEnd(); ++it)
    {
        const typename T::SPtr &entity = *it;
        if (!unerase(entity))
        {
            while (it != entities.constBegin())
            {
                --it;
                ASSERT(*it);
                softErasedIds_ << (*it)->id();
            }

            return false;
        }
    }

    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param id [const int]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::save(const int id)
{
    static QVector<int> ids(1);
    ids[0] = id;
    return save(ids);
}

/**
  * @param id [const int]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::save(const QVector<int> & /* ids */)
{
    return false;
}

/**
  * @param entity [const IEntitySPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::saveGeneric(const IEntitySPtr &entity)
{
    ASSERT(!entity || entity->type() == T::kType);
    ASSERT(boost::dynamic_pointer_cast<T>(entity));

    return save(boost::shared_static_cast<T>(entity));
}

/**
  * @param entity [const typename T::SPtr &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::save(const typename T::SPtr &entity)
{
    static QVector<typename T::SPtr> vector(1);
    vector[0] = entity;
    return save(vector);
}

/**
  * @param entities [const QVector<IEntitySPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::saveGeneric(const QVector<IEntitySPtr> &entities)
{
    // Convert to T::SPtr
    QVector<typename T::SPtr> typeEntities;
    typeEntities.reserve(entities.size());
    foreach (const IEntitySPtr &entity, entities)
    {
        ASSERT(!entity || entity->type() == T::kType);
        ASSERT(boost::dynamic_pointer_cast<T>(entity));

        typeEntities << boost::static_pointer_cast<T>(entity);
    }

    return save(typeEntities);
}

/**
  * @param entities [const QVector<typename T::SPtr> &]
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::save(const QVector<typename T::SPtr> & /* entities */)
{
    return false;
}

/**
  * @returns bool
  */
template<typename T>
inline
bool MemoryOnlyRepository<T>::saveAll()
{
    return false;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param id [const int]
  * @returns IEntitySPtr
  */
template<typename T>
inline
IEntitySPtr MemoryOnlyRepository<T>::vFind(const int id)
{
    return find(id);
}

/**
  * @param ids [const QVector<int> &]
  * @returns QVector<IEntitySPtr>
  */
template<typename T>
inline
QVector<IEntitySPtr> MemoryOnlyRepository<T>::vFind(const QVector<int> &ids)
{
    QVector<IEntitySPtr> found;
    found.reserve(ids.size());
    foreach (int id, ids)
        found << find(id);
    return found;
}

template<typename T>
inline
bool MemoryOnlyRepository<T>::localAdd(const typename T::SPtr &entity, bool ignoreNull)
{
    if (!entity)
        return ignoreNull;

    if (softErasedIds_.contains(entity->id()))
        return false;

    identityHash_.insert(entity->id(), entity);

    return true;
}

#endif // MEMORYONLYREPOSITORY_H
