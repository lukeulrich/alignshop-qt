/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTENTITY_H
#define ABSTRACTENTITY_H

#include <QtCore/QtGlobal>

#include "IEntity.h"
#include "../global.h"
#include "../macros.h"
#include "../util/DecrementNumberGenerator.h"

struct AbstractEntityPod;

class AbstractEntity : public IEntity
{
public:
    AbstractEntity *clone() const
    {
        ASSERT_X(0, "Unimplemented assertion");

        return nullptr;
    }

    int dirtyFlags() const
    {
        return dirtyFlags_;
    }

    int id() const
    {
        return id_;
    }

    bool isDirty() const
    {
        return dirtyFlags_ > 0;
    }

    bool isDirty(const int dirtyFlag) const
    {
        return dirtyFlags_ & dirtyFlag;
    }

    bool isNew() const
    {
        return id_ < 0;
    }

    void setClean()
    {
        dirtyFlags_ = 0;
    }

    void setDirty(const int dirtyFlag, const bool dirty)
    {
        if (dirty)
            dirtyFlags_ |= dirtyFlag;
        else
            dirtyFlags_ &= ~dirtyFlag;
    }

    void setId(int id)
    {
        id_ = id;
    }

protected:
    AbstractEntity(int id) : id_(id), dirtyFlags_(0)
    {
    }

private:
    int id_;
    int dirtyFlags_;
};
Q_DECLARE_TYPEINFO(AbstractEntity, Q_MOVABLE_TYPE);


template<typename T>
int newEntityId()
{
    static DecrementNumberGenerator decrementor_(0);
    return decrementor_.nextValue();
}


struct AbstractEntityPod
{
    int id_;

    AbstractEntityPod(int id = 0) : id_(id)
    {
    }

    bool isNull() const
    {
        return id_ == 0;
    }
};
Q_DECLARE_TYPEINFO(AbstractEntityPod, Q_MOVABLE_TYPE);


#endif // ABSTRACTENTITY_H
