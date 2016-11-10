/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GENERICENTITYMAPPER_H
#define GENERICENTITYMAPPER_H

#include "AbstractEntityMapper.h"
#include "../DataSources/IAdocSource.h"
#include "../global.h"

template<typename T, typename PodT>
class GenericEntityMapper : public AbstractEntityMapper<T, PodT>
{
public:
    GenericEntityMapper(IAdocSource *adocSource) : AbstractEntityMapper<T, PodT>(adocSource) {}

    virtual bool erase(const QVector<T *> &entities) const
    {
        try
        {
            AbstractEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->erase(entities);
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    virtual bool erase(const QVector<int> &ids) const
    {
        try
        {
            AbstractEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->eraseByIds(ids);
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    virtual QVector<T *> find(const QVector<int> &ids) const
    {
        try
        {
            AbstractEntityMapper<T, PodT>::adocSource_->begin();
            QVector<PodT> pods = AbstractEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->read(ids);
            QVector<T *> entities = this->convertPodsToEntities(pods);
            AbstractEntityMapper<T, PodT>::adocSource_->end();

            return entities;
        }
        catch(...)
        {
            return QVector<T *>();
        }
    }

    virtual bool save(const QVector<T *> &entities) const
    {
        try
        {
            AbstractEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->save(entities);
            return true;
        }
        catch(...)
        {
            return false;
        }
    }
};

#endif // GENERICENTITYMAPPER_H
