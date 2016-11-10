/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DBROWCACHEMANAGER_H
#define DBROWCACHEMANAGER_H

#include "DataRow.h"
#include "DbDataSource.h"

/**
  * DbRowCacheManager transparently fetches and caches DataRows associated with a single database table.
  *
  * By default, baseSize and overflow size is zero and no caching is performed until the user supplies a
  * positive baseSize value. DbRowCacheManager caches up to baseSize DataRows after which point it will
  * continue to cache overflowSize additional sequences. Once the number of cached sequences exceeds the
  * baseSize plus the overflowSize, overflowSize "oldest" (those DataRows that have not been accessed
  * recently) will be removed from the cache.
  *
  * Overflowing which reduces the internal cache back to basesize is only possible when fetching DataRows.
  * Changing the base size and/or overflow size will not cause this overflow reduction (in the event that
  * the previous contents now exceed the new limit). Rather, this will merely reduce the total size of the
  * cache to the new imposed limit (oldest accessed cached DataRows being discarded).
  *
  * A simple integer counter is utilized to track the access of each DataRow. Each time a cached DataRow is
  * fetched, the counter is incremented by one and this value associated with the DataRow. Thus, when the
  * overflow is purged, overflow sequences with the lowest access number are removed.
  *
  * The access counter is constrained to the numerical limit of an unsigned long at which point the counter
  * will wrap around to zero and all cache entries will have their access variable reassigned beginning with
  * zero and incrementing by one (while preserving the access count ordering).
  *
  * Because this class talks to the database it is possible for the data retrieval functions to indirectly
  * throw an InvalidConnectionError and/or DatabaseError.
  */
class DbRowCacheManager
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance of this class with the given dataSource and sourceTable
    DbRowCacheManager(const DbDataSource &dataSource, const QString &sourceTable);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int capacity() const;                       //!< Returns the cache capacity (base size plus the overflow size)
    void clear();                               //!< Clear all cached DataRows
    int baseSize() const;                       //!< Returns the base cache size
    DataRow fetch(int id);                      //!< Return the DataRow with id, an empty DataRow is returned if no record exists in the datasource with that id
    int overflowSize() const;                   //!< Returns the overflow size
    void remove(int id);                        //!< Remove the DataRow identified by id from the cache
    void setBaseSize(int baseSize);             //!< Sets the base cache size to baseSize; reduces the existing cache to baseSize if it exceeds baseSize
    void setOverflowSize(int overflowSize);     //!< Sets the overflow cache size to overflowSize
    QString sourceTable() const;                //!< Returns the associated source table
    int size() const;                           //!< Returns the number of cached DataRows

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool cacheFull() const;                     //!< Returns true if the cache size is greater than or equal to the base size plus the overflow size; false otherwise
    DataRow fetchFromDatabase(int id) const;    //!< Queries the database sourceTable for the DataRow whose primary key is id and returns the corresponding DataRow
    void removeCacheOverflow();                 //!< Removes overflowSize_ oldest entries from the cache
    void removeOldest(int count);               //!< Removes count oldest entries from the cache
    bool sortIdsByAccessAscending(const int &a, const int &b);

    // ------------------------------------------------------------------------------------------------
    // Private members
    DbDataSource dataSource_;                   //!< Datasource powering the cache
    QString sourceTable_;                       //!< Database table that contains the individual data rows

    int baseSize_;                              //!< Base number of DataRows to cache
    unsigned long counter_;
    QHash<int, DataRow> cachedDataRows_;        //!< Internal cache of DataRow's associated with their identifiers
    QHash<int, unsigned long> cachedTime_;      //!< {id} -> integer count since this DataRow was last accessed
    int overflowSize_;                          //!< Number of additional DataRows to cache before reducing the cache back to the base size

    //!< Orders the cachedDataRow keys by their cachedTime in ascending order
    class sortIdsByAccessAscendingProxy
    {
    public:
        sortIdsByAccessAscendingProxy(const DbRowCacheManager *self) : self_(self) {}

        bool operator()(const int &a, const int &b) const
        {
            return self_->cachedTime_.value(a) < self_->cachedTime_.value(b);
        }

    private:
        const DbRowCacheManager *self_;
    };

#ifdef TESTING
    // Provide access to the internal state of this class if we are testing
    friend class TestDbRowCacheManager;
#endif
};

#endif // DBROWCACHEMANAGER_H
