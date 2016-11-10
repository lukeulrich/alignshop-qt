/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASK_H
#define TASK_H

#include <QtCore/QList>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>

#include "ITask.h"
#include "../../../core/global.h"

/**
  * Task is an empty task with basic default implemented stubs.
  *
  * It is largely intended to be sub-classed for task-specific operations; however, it may also be used as a master
  * task to group together several slave tasks. Many properties of a master task may be thought of as an aggregate of
  * its sub tasks (e.g. master is active if any of its child tasks are active, time running is the sum of child task
  * time, etc.), this is not currently implemented.
  *
  * See ITask introduction for more detailed documentation.
  */
class Task : public ITask
{
    Q_OBJECT

public:
    // -------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct a task with type and name
    Task(const Ag::TaskType type = Ag::Group, const QString &name = QString());


    // -------------------------------------------------------------------------------------------------
    // Public methods
    virtual int id() const;
    virtual bool isActive() const;                      //!< Returns true if is leaf with a status of Starting, Running, or Paused
    virtual bool isOver() const;                        //!< Returns true if is group or a leaf with a status of Finished, Errored, or Killed
    virtual int maxThreads() const;
    virtual int minThreads() const;
    virtual QString name() const;
    virtual QString note() const;
    virtual int nThreads() const;
    virtual int priority() const;
    virtual double progress() const;
    virtual Ag::TaskStatus status() const;
    virtual double timeRunning() const;
    virtual Ag::TaskType type() const;


public Q_SLOTS:
    // -------------------------------------------------------------------------------------------------
    // Public slots
    virtual void kill();
//    virtual void killAndDeleteLater();

    virtual void setMaxThreads(const int maxThreads);
    virtual void setMinThreads(const int minThreads);
    virtual void setName(const QString &name);
    virtual void setNote(const QString &note);
    virtual void setPriority(const int priority);
    virtual void setProgress(const double progress);
    virtual void setStatus(const Ag::TaskStatus taskStatus);
    virtual void setThreads(const int nThreads);

    virtual void start();
    virtual void stop();


protected:
    bool shouldDeleteWhenFinished() const;


private Q_SLOTS:
    // -------------------------------------------------------------------------------------------------
    // Private slots
    void updateTimeRunning();                                   //!< Updates the time running based on the internal timer


private:
    // -------------------------------------------------------------------------------------------------
    // Static private members
    static int currentIdValue_;


    // -------------------------------------------------------------------------------------------------
    // Private members
    int id_;
    Ag::TaskType type_;
    QString name_;
    QString note_;
    Ag::TaskStatus status_;
    int priority_;
    double progress_;
    double timeRunning_;        // expressed in seconds
    int minThreads_;
    int maxThreads_;
    int nThreads_;

    bool deleteWhenFinished_;       // If true, delete this task when it has completed either by error or success

    QElapsedTimer elapsedTimer_;    // This provides for measuring the time difference between runs
    QTimer timer_;                  // The timer permits for updating the time running when it is running
};

#endif // TASK_H
