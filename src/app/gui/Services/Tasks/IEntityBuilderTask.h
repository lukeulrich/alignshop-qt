/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IENTITYBUILDERTASK_H
#define IENTITYBUILDERTASK_H

#include <QtCore/QString>

#include "Task.h"

class IEntity;
// class TransientTaskColumnAdapter;

class IEntityBuilderTask : public Task
{
    Q_OBJECT

public:
    IEntityBuilderTask(const Ag::TaskType type, const QString &name)
        : Task(type, name)
    {
    }

    // entityName contains the optional name for the final entity (e.g. from a TransientTask)
    virtual IEntity *createFinalEntity(const QString &entityName) const = 0;

    // Originally figured this
//    virtual TransientTaskColumnAdapter *transientTaskColumnAdapter() const = 0;
};

#endif // IENTITYBUILDERTASK_H
