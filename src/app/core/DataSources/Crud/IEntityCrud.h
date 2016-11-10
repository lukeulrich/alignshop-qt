/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IENTITYCRUD_H
#define IENTITYCRUD_H

#include <QtCore/QVector>

// After saving an entry, it is the responsibility of the crud to set the entity's dirty state to false.
template<typename T, typename PodT>
class IEntityCrud
{
public:
    virtual ~IEntityCrud() {}

    virtual void save(const QVector<T *> &entities) = 0;
    virtual void erase(const QVector<T *> &entities) = 0;
    virtual void eraseByIds(const QVector<int> &ids) = 0;
    virtual QVector<PodT> read(const QVector<int> &ids) = 0;
};

#endif // IENTITYCRUD_H
