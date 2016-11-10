/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ITASK_H
#define ITASK_H

#include <QtCore/QObject>

namespace Ag
{
    enum TaskType
    {
        Group = 0,
        Leaf
    };

    enum TaskStatus
    {
        NotApplicable = 0,
        NotStarted,
        Starting,
        Running,
        Paused,
        Finished,
        Error,
        Killed
    };
}

/**
  * ITask defines the interface to describe and manage the task execution.
  *
  * Because many tasks are related and pipelining of results from one operation to another is a common process, it is
  * desirable to arrange these into a tree structure. Thus, a task may either function as a group that contains subtasks
  * or a terminal leaf that is an actual task. Rather than redundantly combine tree functionality into this class,
  * ITasks are associated with the TreeNode class in a templated fashion.
  *
  * Currently, a task has the following properties:
  * o Name - arbitrary name for user-purposes
  * o Note - descriptive information for arbitrary purposes (e.g. indicating error)
  * o Priority - general priority of this task relative to others
  * o Status - any one of several status values that denote this tasks current state
  * o Progress - value between 0 and 1 that reflects the current progress of this task
  * o Type - indicates whether this task simply groups other tasks or is a leaf task with a specific function
  * o Time running - amount of time this task has been running in seconds
  * o Number of assigned threads
  *
  * Additionally, it is possible to specify the minimum and maximum number of threads for a task manager to better
  * optimize/queue tasks (see TaskManager).
  *
  * There are 3 methods for controlling a task: start, stop, and kill. Pause functionality is task-dependent and may
  * be implemented via the stop method call.
  *
  * Because a task will usually be one of many other tasks, each time a task property changes, it emits a pointer to
  * itself for easy access.
  *
  * NOTE: Priority is currently without constraints. Meaning, the int priority has no well-defined meaning at this
  *       point.
  */
class ITask : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Destructor
    virtual ~ITask() {}                                             //!< Virtual destructor for memory purposes

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual int id() const = 0;                                     //!< Returns the id for this task
    virtual bool isActive() const = 0;                              //!< Returns true if this task has been started and is running
    virtual bool isOver() const = 0;                                //!< Returns true if this task has at least attempted to start and is in a terminal state (success or failure)
    virtual int maxThreads() const = 0;                             //!< Returns the maximum number of threads this task may utilize
    virtual int minThreads() const = 0;                             //!< Returns the minimum number of threads required
    virtual QString name() const = 0;                               //!< Returns the name
    virtual QString note() const = 0;                               //!< Returns the note
    virtual int nThreads() const = 0;                               //!< Returns the number of threads for this task to use (usually done by TaskManager) or 0 (default)
    virtual int priority() const = 0;                               //!< Returns the priority
    virtual double progress() const = 0;                            //!< Returns the progress as a fraction between 0 and 1
    virtual Ag::TaskStatus status() const = 0;                      //!< Returns the current status
    virtual double timeRunning() const = 0;                         //!< Returns the time this task has been active in seconds
    virtual Ag::TaskType type() const = 0;                          //!< Returns the type


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    virtual void kill() = 0;                                        //!< Kill the current process
//    virtual void killAndDeleteLater() = 0;                          //!< Kill the current process and queues this object for deletion after it has been killed

    virtual void setMaxThreads(const int maxThreads) = 0;           //!< Sets the maximum number of threads to maxThreads
    virtual void setMinThreads(const int minThreads) = 0;           //!< Sets the minimum number of threads to maxThreads
    virtual void setName(const QString &name) = 0;                  //!< Sets the name
    virtual void setNote(const QString &note) = 0;                  //!< Sets the note
    virtual void setThreads(const int nThreads) = 0;                //!< Set the number of threads
    virtual void setPriority(const int priority) = 0;               //!< Set the priority
    virtual void setProgress(const double progress) = 0;            //!< Set the progress
    virtual void setStatus(const Ag::TaskStatus taskStatus) = 0;    //!< Set the status

    virtual void start() = 0;                                       //!< Start the task
    virtual void stop() = 0;                                        //!< Stop or pause the task


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void done(ITask *task);
    void error(ITask *task);
    void nameChanged(ITask *task);
    void noteChanged(ITask *task);
    void priorityChanged(ITask *task);
    void progressChanged(ITask *task);
    void statusChanged(ITask *task);
    void threadsChanged(ITask *task);
    void timeRunningChanged(ITask *task);
};

#endif // ITASK_H
