/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TRANSIENTTASK_H
#define TRANSIENTTASK_H

#include "AbstractBasicEntity.h"
#include "../enums.h"
#include "../macros.h"

#include "../../gui/Services/Tasks/ITask.h"

// Because these are not saved no need to manage the dirty status
//
// Duplicates the relevant fields from the Task object (name, note, progress, status) so that this entity is not
// strictly dependent on the Task object existing. For example, if the Task is killed and removed from the owning
// TaskManger, then this entity should still be able to report its killed status without relying on the Task object
// being valid.
//
// Ids are not defined in the constructor but carried over from the Task itself
class TransientTask : public AbstractBasicEntity
{
public:
    typedef boost::shared_ptr<TransientTask> SPtr;
    static const int kType = eTransientTaskEntity;

    TransientTask(const QString &name, const QString &description, const QString &notes, ITask *task)
        : AbstractBasicEntity(0, name, description, notes),
          task_(task)
    {
        setId(task_->id());
    }

    TransientTask(const QString &name, ITask *task)
        : AbstractBasicEntity(0, name, "", ""),
          task_(task)
    {
        setId(task_->id());
    }

//    int id() const
//    {
//        if (task_ != nullptr)
//            return task_->id();

//        return 0;
//    }

    double progress() const
    {
        return progress_;
    }

    void setProgress(const double progress)
    {
        ASSERT(progress >= 0 && progress <= 1.);

        progress_ = progress;
    }

    void setStatus(const Ag::TaskStatus status)
    {
        status_ = status;
    }

    void setTask(ITask *task)
    {
        task_ = task;
    }

    Ag::TaskStatus status() const
    {
        return status_;
    }

    ITask *task() const
    {
        return task_;
    }

    int type() const
    {
        return eTransientTaskEntity;
    }

private:
    ITask *task_;
    Ag::TaskStatus status_;
    double progress_;
};

typedef boost::shared_ptr<TransientTask> TransientTaskSPtr;

#endif // TRANSIENTTASK_H
