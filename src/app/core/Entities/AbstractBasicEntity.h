/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTBASICENTITY_H
#define ABSTRACTBASICENTITY_H

#include "IBasicEntity.h"
#include "AbstractEntity.h"

class AbstractBasicEntity : public AbstractEntity,
                            public IBasicEntity
{
public:
    virtual QString description() const;
    virtual QString name() const;
    virtual QString notes() const;

    virtual void setDescription(const QString &description);
    virtual void setName(const QString &name);
    virtual void setNotes(const QString &notes);

protected:
    AbstractBasicEntity(int id, const QString &name, const QString &description, const QString &notes);

    QString name_;
    QString description_;
    QString notes_;
};
Q_DECLARE_TYPEINFO(AbstractBasicEntity, Q_MOVABLE_TYPE);


struct AbstractBasicEntityPod : public AbstractEntityPod
{
    QString name_;
    QString description_;
    QString notes_;

    AbstractBasicEntityPod(int id = 0) : AbstractEntityPod(id)
    {
    }
};
Q_DECLARE_TYPEINFO(AbstractBasicEntityPod, Q_MOVABLE_TYPE);


#endif // ABSTRACTBASICENTITY_H
