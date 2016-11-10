/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DbRowCacheManager.h"

#include <climits>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * Upon construction, starts the timer and sets the most recent access time to -1. No checking is
  * done at this point; however, if an invalid dataSource and/or sourceTable is provided, the core
  * fetch functionality will throw a InvalidConnectionError/DatabaseError.
  *
  * @param dataSource [const DbDataSource &]
  * @param sourceTable [const QString &]
  */
DbRowCacheManager::DbRowCacheManager(const DbDataSource &dataSource, const QString &sourceTable)
    : dataSource_(dataSource), sourceTable_(sourceTable)
{
    counter_ = 0;
    baseSize_ = 0;
    overflowSize_ = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int DbRowCacheManager::capacity() const
{
    return baseSize_ + overflowSize_;
}

/**
  * Additionally, restarts the timer.
  */
void DbRowCacheManager::clear()
{
    // Clear the cache
    cachedDataRows_.clear();
    cachedTime_.clear();

    // Reset the access timer
    counter_ = 0;
}

/**
  * @returns int
  */
int DbRowCacheManager::baseSize() const
{
    return baseSize_;
}

/**
  * If the DataRow identified by id is present in the cache return it and update its access time.
  * On the other hand, if it is not present in the cache, attempt to fetch it from the datasource.
  * If the result is not found in the database return an empty row; however, if it is valid, then
  * check that the cache size is not exceeded and remove overflow + one (for the currently requested
  * sequence) oldest DataRows if it is. The recently retrieved DataRow is then added to the cache
  * and returned.
  *
  * Additionally, if the timer has wrapped around (as determined by comparing the most recent access
  * time with the currently elapsed time), then reset the access times for all cached DataRows to
  * zero.
  *
  * @param id [int]
  * @returns DataRow
  */
DataRow DbRowCacheManager::fetch(int id)
{
    if (capacity() == 0)
    {
        Q_ASSERT_X(size() == 0, "DbRowCacheManager::fetch", "size() should be zero when capacity() is zero");
        return fetchFromDatabase(id);
    }

    // For treeviews, it is possible that the same id will be requested multiple times. Thus, to slow down
    // overflowing the counter integer range when requesting the same sequence multiple times, only update
    // the counter if a different record was fetched.
    static int lastId = -1;
    if (lastId != id)
    {
        if (counter_ == ULONG_MAX)
        {
            // Counter has met the integral limit. Reassign all access counts back to zero while preserving
            // their access order
            QList<int> ids = cachedDataRows_.keys();
            qSort(ids.begin(), ids.end(), sortIdsByAccessAscendingProxy(this));
            for (int i=0, z=ids.size(); i<z; ++i)
            {
                Q_ASSERT_X(cachedTime_.contains(ids.at(i)), "DbRowCacheManager::fetch", "cachedTime_ missing cachedDataRows_ key id");
                cachedTime_.insert(ids.at(i), i+1);
            }

            counter_ = ids.size();
        }

        ++counter_;
        lastId = id;
    }

    // If in the cache, simply return the relevant DataRow after updating the access time
    if (cachedDataRows_.contains(id))
    {
        cachedTime_[id] = counter_;
        return cachedDataRows_.value(id);
    }

    // Fetch the new row from the database
    DataRow row = fetchFromDatabase(id);
    if (row.isEmpty())  // If it is empty, simply return it without modifying the cache
        return row;

    if (cacheFull())
        removeCacheOverflow();

    cachedDataRows_.insert(id, row);
    cachedTime_.insert(id, counter_);

    return row;
}

/**
  * @returns int
  */
int DbRowCacheManager::overflowSize() const
{
    return overflowSize_;
}

/**
  * Removes the DataRow identified by id from the cache if it is present. This is useful when a
  * DataRow has been updated by external means and the cached data is no longer up to date. If it
  * should be requested again, the latest version will be retrieved from the database and cached.
  *
  * @param id [int]
  */
void DbRowCacheManager::remove(int id)
{
    cachedDataRows_.remove(id);
    cachedTime_.remove(id);
}

/**
  * If baseSize is negative, throws an assert in debug mode and does nothing in release mode. If
  * baseSize is less than the current baseSize_, the cache will be reduced appropriately if the
  * current cache size is greater than the new base size plus the overflow.
  *
  * @param baseSize [int]
  */
void DbRowCacheManager::setBaseSize(int baseSize)
{
    Q_ASSERT_X(baseSize >= 0, QString("DbRowCacheManager::setBaseSize(%1)").arg(baseSize).toAscii(), "baseSize parameter must be greater than or equal to 0");
    if (baseSize < 0)   // Release mode guard
        return;

    baseSize_ = baseSize;
    if (size() > capacity())
        removeOldest(size() - capacity());
}

/**
  * If overflowSize is negative, throws an assert in debug mode and dose nothing in release mode. If
  * the current cache size exceeds that of baseSize plus the new overflowSize, the cache will be
  * reduced appropriately.
  *
  * @param overflowSize [int]
  */
void DbRowCacheManager::setOverflowSize(int overflowSize)
{
    Q_ASSERT_X(overflowSize >= 0, QString("DbRowCacheManager::setOverflowSize(%1)").arg(overflowSize).toAscii(), "overflowSize parameter must be greater than or equal to 0");
    if (overflowSize < 0)   // Release mode guard
        return;

    overflowSize_ = overflowSize;
    if (size() > capacity())
        removeOldest(size() - capacity());
}

/**
  * @returns QString
  */
QString DbRowCacheManager::sourceTable() const
{
    return sourceTable_;
}

/**
  * @returns int
  */
int DbRowCacheManager::size() const
{
    return cachedDataRows_.size();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns bool
  */
bool DbRowCacheManager::cacheFull() const
{
    return size() >= capacity();
}

/**
  * @param id [int]
  * @returns DataRow
  */
DataRow DbRowCacheManager::fetchFromDatabase(int id) const
{
    return dataSource_.read(sourceTable_, id);
}

/**
  * Removes size() - baseSize_ - 1 records from the internal cache beginning with those that were accessed
  * the longest ago.
  */
void DbRowCacheManager::removeCacheOverflow()
{
    removeOldest(size() - baseSize_ + 1);
}

/**
  * @param count [int]
  */
void DbRowCacheManager::removeOldest(int count)
{
    if (count <= 0)
        return;
    else if (count > size())
        count = size();

    // Sort the keys of the hash by their access times in ascending order (oldest to most recently
    // accessed).
    //
    // To effectively perform this sort, we need to call a function belonging to this class so it can
    // access the cachedTime_ hash. To achieve this effect, must use a private proxy class and the operator()
    // method. We pass a pointer to this in the constructor so that it can reference the cachedTime_
    // member.
    QList<int> ids = cachedDataRows_.keys();
    qSort(ids.begin(), ids.end(), sortIdsByAccessAscendingProxy(this));

    for (int i=0; i< count; ++i)
        remove(ids.at(i));
}
