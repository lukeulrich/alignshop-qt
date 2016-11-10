/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ICOMMONCOLUMNADAPTER_H
#define ICOMMONCOLUMNADAPTER_H

#include "IColumnAdapter.h"

class ICommonColumnAdapter : public IColumnAdapter
{
    Q_OBJECT

public:
    ICommonColumnAdapter(QObject *parent = 0) : IColumnAdapter(parent)
    {
    }

    virtual int commonColumnCount() const = 0;
    virtual int uniqueColumnCount() const = 0;

    virtual int columnCount() const
    {
        return commonColumnCount() + uniqueColumnCount();
    }

    virtual QVariant commonData(const IEntity *entity, int column) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(column);

        return QVariant();
    }

    virtual QVariant uniqueData(const IEntity *entity, int column) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(column);

        return QVariant();
    }

    QVariant data(const IEntity *entity, int column) const
    {
        if (!entity)
            return QVariant();

        if (column > 0)
        {
            if (column < commonColumnCount())
                return commonData(entity, column);
            else if (column < commonColumnCount() + uniqueColumnCount())
                return uniqueData(entity, column - commonColumnCount());
        }

        return QVariant();
    }

    virtual bool setCommonData(IEntity *entity, int column, const QVariant &value) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(column);
        Q_UNUSED(value);

        return false;
    }

    virtual bool setUniqueData(IEntity *entity, int column, const QVariant &value) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(column);
        Q_UNUSED(value);

        return false;
    }

    bool setData(IEntity *entity, int column, const QVariant &value) const
    {
        if (!entity)
            return false;

        if (column > 0)
        {
            if (column < commonColumnCount())
                return setCommonData(entity, column, value);
            else if (column < commonColumnCount() + uniqueColumnCount())
                return setUniqueData(entity, column - commonColumnCount(), value);
        }

        return false;
    }
};


#endif // ICOMMONCOLUMNADAPTER_H
