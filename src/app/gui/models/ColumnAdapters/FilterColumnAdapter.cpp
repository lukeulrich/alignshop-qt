/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "FilterColumnAdapter.h"

#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
FilterColumnAdapter::FilterColumnAdapter(QObject *parent)
    : IFilterColumnAdapter(parent), sourceAdapter_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int FilterColumnAdapter::columnCount() const
{
    if (sourceAdapter_ != nullptr)
        return filterSourceMapping_.size();

    return 0;
}

/**
  * @param entity [const IEntitySPtr &]
  * @param proxyColumn [int]
  * @param role [int]
  * @returns QVariant
  */
QVariant FilterColumnAdapter::data(const IEntitySPtr &entity, int proxyColumn, int role) const
{
    if (sourceAdapter_ != nullptr)
        return sourceAdapter_->data(entity, mapToSource(proxyColumn), role);

    return QVariant();
}

/**
  * @param sourceColumn [int]
  */
void FilterColumnAdapter::exclude(int sourceColumn)
{
    ASSERT_X(isValidSourceColumn(sourceColumn), "sourceColumn out of range");

    excludedColumns_ << sourceColumn;
    updateMapping();
}

/**
  * @param proxyColumn [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags FilterColumnAdapter::flags(int proxyColumn) const
{
    ASSERT_X(isValidProxyColumn(proxyColumn), "proxyColumn out of range");

    return sourceAdapter_->flags(mapToSource(proxyColumn));
}

/**
  * @param proxyColumn [int]
  * @returns QVariant
  */
QVariant FilterColumnAdapter::headerData(int proxyColumn) const
{
    ASSERT_X(isValidProxyColumn(proxyColumn), "proxyColumn out of range");

    return sourceAdapter_->headerData(mapToSource(proxyColumn));
}

/**
  * @param sourceColumn [int]
  */
void FilterColumnAdapter::include(int sourceColumn)
{
    ASSERT_X(isValidSourceColumn(sourceColumn), "sourceColumn out of range");

    if (excludedColumns_.remove(sourceColumn))
        updateMapping();
}

/**
  * Returns -1 if sourceColumn does not correspond to a colum in this proxy adapter.
  *
  * @param sourceColumn [int]
  * @returns int
  */
int FilterColumnAdapter::mapFromSource(int sourceColumn) const
{
    ASSERT_X(isValidSourceColumn(sourceColumn), "sourceColumn out of range");

    return filterSourceMapping_.indexOf(sourceColumn);
}

/**
  * @param proxyColumn [int]
  * @returns int
  */
int FilterColumnAdapter::mapToSource(int proxyColumn) const
{
    ASSERT_X(isValidProxyColumn(proxyColumn), "proxyColumn out of range");

    return filterSourceMapping_.at(proxyColumn);
}

/**
  * @param entity [IEntitySPtr &]
  * @param proxyColumn [int]
  * @param value [const QVariant &]
  * @returns bool
  */
bool FilterColumnAdapter::setData(const IEntitySPtr &entity, int proxyColumn, const QVariant &value)
{
    ASSERT_X(isValidProxyColumn(proxyColumn), "proxyColumn out of range");

    return sourceAdapter_->setData(entity, mapToSource(proxyColumn), value, true);
}

/**
  * @param entity [IEntitySPtr &]
  * @param proxyColumn [int]
  * @param value [const QVariant &]
  * @param allowUndo [bool]
  * @returns bool
  */
bool FilterColumnAdapter::setData(const IEntitySPtr &entity, int proxyColumn, const QVariant &value, bool allowUndo)
{
    ASSERT_X(isValidProxyColumn(proxyColumn), "proxyColumn out of range");

    return sourceAdapter_->setData(entity, mapToSource(proxyColumn), value, allowUndo);
}

/**
  * @param sourceAdapter [IColumnAdapter *]
  */
void FilterColumnAdapter::setSourceAdapter(IColumnAdapter *sourceAdapter)
{
    if (sourceAdapter_ != nullptr)
        disconnect(sourceAdapter_, SIGNAL(dataChanged(IEntitySPtr,int)), this, SLOT(onSourceDataChanged(IEntitySPtr,int)));

    sourceAdapter_ = sourceAdapter;

    if (sourceAdapter_ != nullptr)
        connect(sourceAdapter_, SIGNAL(dataChanged(IEntitySPtr,int)), SLOT(onSourceDataChanged(IEntitySPtr,int)));

    excludedColumns_.clear();
    updateMapping();
}

/**
  * @returns IColumnAdapter *
  */
IColumnAdapter *FilterColumnAdapter::sourceAdapter() const
{
    return sourceAdapter_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param entity [IEntitySPtr &]
  * @param sourceColumn [int]
  */
void FilterColumnAdapter::onSourceDataChanged(const IEntitySPtr &entity, int sourceColumn)
{
    int proxyColumn = mapFromSource(sourceColumn);
    if (proxyColumn != -1)
        emit dataChanged(entity, proxyColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param proxyColumn [int]
  * @returns bool
  */
bool FilterColumnAdapter::isValidProxyColumn(int proxyColumn) const
{
    if (sourceAdapter_ != nullptr)
        return proxyColumn >= 0 && proxyColumn < columnCount();

    return false;
}

/**
  * @param sourceColumn [int]
  * @returns bool
  */
bool FilterColumnAdapter::isValidSourceColumn(int sourceColumn) const
{
    if (sourceAdapter_ != nullptr)
        return sourceColumn >= 0 && sourceColumn < sourceAdapter_->columnCount();

    return false;
}

/**
  */
void FilterColumnAdapter::updateMapping()
{
    if(sourceAdapter_ == nullptr)
        return;

    int nSourceColumns = sourceAdapter_->columnCount();
    filterSourceMapping_.resize(nSourceColumns - excludedColumns_.size());
    for (int i=0, j=0; i< nSourceColumns; ++i)
    {
        if (excludedColumns_.contains(i))
            continue;

        filterSourceMapping_[j] = i;
        ++j;
    }
}
