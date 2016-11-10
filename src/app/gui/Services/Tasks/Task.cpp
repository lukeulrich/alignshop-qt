/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>                // For fabs

#include "Task.h"
#include "../../../core/macros.h"

int Task::currentIdValue_ = 0;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * Initially, the number of assigned threads is 0, because it has not yet been initialized (e.g. by TaskManager).
  *
  * @param type [const Ag::TaskType]
  * @param name [const QString &]
  */
Task::Task(const Ag::TaskType type, const QString &name)
    : id_(++currentIdValue_),
      type_(type),
      name_(name),
      status_(Ag::NotApplicable),
      priority_(0),
      progress_(0.),
      timeRunning_(0.),
      minThreads_(1),
      maxThreads_(1),
      nThreads_(0),
      deleteWhenFinished_(false)
{
    if (type_ == Ag::Leaf)
        status_ = Ag::NotStarted;

    // Update the time every second
    timer_.setInterval(1000);
    connect(&timer_, SIGNAL(timeout()), SLOT(updateTimeRunning()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int Task::id() const
{
    return id_;
}

/**
  * @returns bool
  */
bool Task::isActive() const
{
    return type_ == Ag::Leaf &&
           (status_ == Ag::Starting ||
            status_ == Ag::Running ||
            status_ == Ag::Paused);
}

/**
  * @returns bool
  */
bool Task::isOver() const
{
    return type_ == Ag::Group ||
           status_ == Ag::Finished ||
           status_ == Ag::Error ||
           status_ == Ag::Killed;
}

/**
  * @returns int
  */
int Task::maxThreads() const
{
    return maxThreads_;
}

/**
  * @returns int
  */
int Task::minThreads() const
{
    return minThreads_;
}

/**
  * @returns QString
  */
QString Task::name() const
{
    return name_;
}

/**
  * @returns QString
  */
QString Task::note() const
{
    return note_;
}

/**
  * @returns int
  */
int Task::nThreads() const
{
    return nThreads_;
}

/**
  * @returns int
  */
int Task::priority() const
{
    return priority_;
}

/**
  * @returns double
  */
double Task::progress() const
{
    return progress_;
}

/**
  * @returns Ag::TaskStatus
  */
Ag::TaskStatus Task::status() const
{
    return status_;
}

/**
  * @returns double
  */
double Task::timeRunning() const
{
    return timeRunning_;
}

/**
  * @returns Ag::TaskType
  */
Ag::TaskType Task::type() const
{
    return type_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void Task::kill()
{
    setStatus(Ag::Killed);
}

/**
  * For this to be effective, kill() must be implemented and subclasses should check this flag and call deleteLater
  * after the kill process has completed.
  */
//void Task::killAndDeleteLater()
//{
//    deleteWhenFinished_ = true;
//    kill();
//}

/**
  * @param maxThreads [const int]
  */
void Task::setMaxThreads(const int maxThreads)
{
    ASSERT(type_ == Ag::Leaf);
    ASSERT(maxThreads > 0);
    maxThreads_ = maxThreads;

    // Cap minimum threads
    if (minThreads_ > maxThreads_)
        minThreads_ = maxThreads_;
}

/**
  * @param minThreads [const int]
  */
void Task::setMinThreads(const int minThreads)
{
    ASSERT(type_ == Ag::Leaf);
    ASSERT(minThreads > 0);
    minThreads_ = minThreads;
    if (maxThreads_ < minThreads_)
        maxThreads_ = minThreads_;
}

/**
  * @param name [const QString &]
  */
void Task::setName(const QString &name)
{
    if (name_ == name)
        return;

    name_ = name;
    emit nameChanged(this);
}

/**
  * @param note [const QString &]
  */
void Task::setNote(const QString &note)
{
    if (note_ == note)
        return;

    note_ = note;
    emit noteChanged(this);
}

/**
  * @param priority [int]
  */
void Task::setPriority(const int priority)
{
    if (priority_ == priority)
        return;

    priority_ = priority;
    emit priorityChanged(this);
}

/**
  * @param progress [const double]
  */
void Task::setProgress(const double progress)
{
    ASSERT(progress >= 0 && progress <= 1.);
    if (qFuzzyCompare(progress_, progress))
        return;

    progress_ = progress;
    emit progressChanged(this);
}

/**
  * @param taskStatus [const Ag::TaskStatus]
  */
void Task::setStatus(const Ag::TaskStatus taskStatus)
{
    if (status_ == taskStatus)
        return;

    // Note: reacting to old status here
    if (status_ == Ag::Running)
    {
        timer_.stop();
        updateTimeRunning();
    }

    status_ = taskStatus;
    emit statusChanged(this);

    if (status_ == Ag::Running)
    {
        timer_.start();
        elapsedTimer_.restart();
    }
}

/**
  * Set the actual number of threads to use by this task.
  *
  * @param nThreads [const int]
  */
void Task::setThreads(const int nThreads)
{
    ASSERT(type_ == Ag::Leaf);
    ASSERT(nThreads >= minThreads_ && nThreads <= maxThreads_);
    ASSERT(status_ == Ag::NotStarted);

    if (nThreads_ == nThreads)
        return;

    nThreads_ = nThreads;

    emit threadsChanged(this);
}

/**
  */
void Task::start()
{
    setStatus(Ag::Finished);
    emit done(this);
}

/**
  */
void Task::stop()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @returns bool
  */
bool Task::shouldDeleteWhenFinished() const
{
    return deleteWhenFinished_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Crude approximation - obviously could be improved by simply referring to the elapsed amount; however, care should
  * be taken here because it is possible to pause/restart processes.
  */
void Task::updateTimeRunning()
{
    double delta = (elapsedTimer_.elapsed() / 1000.) - timeRunning_;
    timeRunning_ += delta;
    emit timeRunningChanged(this);
}
