/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IBASICENTITY_H
#define IBASICENTITY_H

#include <QtCore/QString>

// Technically, a IBasicEntity is also a IEntity, but to avoid the diamond problem, we do not inherit from it.

class IBasicEntity
{
public:
    virtual ~IBasicEntity() {}

    virtual QString description() const = 0;
    virtual QString name() const = 0;
    virtual QString notes() const = 0;

    virtual void setDescription(const QString &description) = 0;
    virtual void setName(const QString &name) = 0;
    virtual void setNotes(const QString &notes) = 0;
};

#endif // IBASICENTITY_H
