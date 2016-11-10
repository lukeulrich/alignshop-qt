/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IVALUEOBJECTMAPPER_H
#define IVALUEOBJECTMAPPER_H

#include <QtCore/QVector>

template<typename T, typename Pod>
class IValueObjectMapper
{
public:
    virtual T mapToObject(const Pod &pod) const = 0;
    virtual Pod mapFromObject(const T &object) const = 0;

    QVector<T> mapToObjects(const QVector<Pod> &pods) const
    {
        QVector<T> objects;
        objects.reserve(pods.size());
        foreach (const Pod &pod, pods)
            objects << mapToObject(pod);

        return objects;
    }

    QVector<Pod> mapFromObjects(const QVector<T> &objects) const
    {
        QVector<Pod> pods;
        pods.reserve(objects.size());
        foreach (const T &object, objects)
            pods << mapFromObject(object);

        return pods;
    }
};

#endif // IVALUEOBJECTMAPPER_H
