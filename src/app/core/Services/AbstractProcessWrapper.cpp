/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "AbstractProcessWrapper.h"
#include "../macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Destructor
/**
  */
AbstractProcessWrapper::~AbstractProcessWrapper()
{
    kill();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QByteArray
  */
QByteArray AbstractProcessWrapper::errorOutput() const
{
    return errorOutput_;
}

/**
  * The base implementation simply returns the program name.
  *
  * @returns QString
  */
QString AbstractProcessWrapper::friendlyProgramName() const
{
    return program_;
}

/**
  * A convenient way of checking that the input is properly configured is to rely on a non-empty options vector, which
  * will always correlate to the last value returned by the setOptions method.
  *
  * @returns bool
  */
bool AbstractProcessWrapper::hasValidOptions() const
{
    return !options_.isEmpty();
}

/**
  * @returns int
  */
int AbstractProcessWrapper::id() const
{
    return 0;
}

/**
  * @returns bool
  */
bool AbstractProcessWrapper::isRunning() const
{
    return process_->state() == QProcess::Running ||
           process_->state() == QProcess::Starting;
}

/**
  * @returns QByteArray
  */
QByteArray AbstractProcessWrapper::output() const
{
    return output_;
}

/**
  * @returns QString
  */
QString AbstractProcessWrapper::program() const
{
    return program_;
}

/**
  * @param newOptions [const OptionSet &]
  * @returns bool
  */
bool AbstractProcessWrapper::setOptions(const OptionSet &newOptions)
{
    if (optionProfile_.validOptions(newOptions.asVector()))
    {
        options_ = newOptions;
        return true;
    }

    options_.clear();
    return false;
}

/**
  * @returns bool
  */
bool AbstractProcessWrapper::wasKilled() const
{
    return killed_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Does nothing if process is not currently running, otherwise, a kill signal is sent to the process.
  *
  * @param killMessage [const QString &]
  */
void AbstractProcessWrapper::kill(const QString &killMessage)
{
    if (process_->state() == QProcess::NotRunning)
        return;

    killed_ = true;
    killMessage_ = killMessage;
    process_->kill();

    // ISSUE? Perhaps the user wants to asynchronously kill this process?
    process_->waitForFinished();
}

/**
  * @param program [const QString &]
  */
void AbstractProcessWrapper::setProgram(const QString &newProgram)
{
    ASSERT(isRunning() == false);

    program_ = newProgram;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  * Transforms process errors into more user-friendly error signals.
  *
  * @param error [QProcess::ProcessError]
  */
void AbstractProcessWrapper::onProcessError(QProcess::ProcessError processError)
{
    // When a process is killed, this method will first be called and then the onProcessFinished method, which is
    // where we handle emitting the appropriate error signal.
    if (killed_)
    {
        if (killMessage_.size())
            emit error(id(), killMessage_);
        return;
    }

    QString errorMessage = "Process was unable to be executed - ";
    switch (processError)
    {
    case QProcess::FailedToStart:
        errorMessage += "The process failed to start. Either the executable is missing or you may have "
                        "insufficient privileges to execute this program.";
        break;
    case QProcess::Crashed:
        errorMessage += "crashed during execution.";
        break;
    case QProcess::Timedout:
        errorMessage += "timed out.";
        break;
    case QProcess::WriteError:
        errorMessage += "an error occurred while sending data to the program.";
        break;
    case QProcess::ReadError:
        errorMessage += "an error occurred while reading data from the program.";
        break;

    default:
        errorMessage += "an unknown error occurred.";
    }

    emit error(id(), errorMessage);
}

/**
  * @param exitCode [int]
  * @param exitStatus [QProcess::ExitStatus]
  */
void AbstractProcessWrapper::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (killed_)
    {
        emit error(id(), killMessage_);
        return;
    }

    if (exitStatus == QProcess::NormalExit)
    {
        handleExitCode(exitCode);
        return;
    }

    // ISSUE? Does onProcessError get called when exitStatus == QProcess::CrashExit?
    ASSERT(exitStatus == QProcess::CrashExit);    // Because this is the only other status code :)
    emit error(id(), QString("%1 has crashed").arg(friendlyProgramName()));
}

/**
  * Empty stub that does nothing.
  */
void AbstractProcessWrapper::onProcessStarted()
{
    ASSERT(isRunning());
}

/**
  * Aggregate all error data into the errorOutput member variable.
  */
void AbstractProcessWrapper::onReadyReadStandardError()
{
    process_->setReadChannel(QProcess::StandardError);
    while (process_->canReadLine())
        errorOutput_.append(process_->readLine());
}

/**
  * Aggregate all output data into the output member variable.
  */
void AbstractProcessWrapper::onReadyReadStandardOutput()
{
    process_->setReadChannel(QProcess::StandardOutput);
    while (process_->canReadLine())
        output_.append(process_->readLine());
}

/**
  * This should not be called by subclasses when the process is running. Rather, most subclasses should call this before
  * the process is started.
  */
void AbstractProcessWrapper::reset()
{
    errorOutput_.clear();
    output_.clear();
    killed_ = false;
    killMessage_.clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param parent [QObject *]
  */
AbstractProcessWrapper::AbstractProcessWrapper(QObject *parent)
    : QObject(parent),
      process_(nullptr),
      killed_(false)
{
    process_ = new QProcess(this);

    connect(process_, SIGNAL(readyReadStandardError()), SLOT(onReadyReadStandardError()));
    connect(process_, SIGNAL(readyReadStandardOutput()), SLOT(onReadyReadStandardOutput()));
    connect(process_, SIGNAL(error(QProcess::ProcessError)), SLOT(onProcessError(QProcess::ProcessError)));
    connect(process_, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onProcessFinished(int,QProcess::ExitStatus)));
    connect(process_, SIGNAL(started()), SLOT(onProcessStarted()));
}

/**
  * @param openMode [QProcess::OpenMode]
  */
void AbstractProcessWrapper::execute(QProcess::OpenMode openMode)
{
    if (isRunning())
        throw QString("%1 is already running with this instance.").arg(friendlyProgramName());

    if (program_.isEmpty())
        throw QString("No executable program has been defined.");

    if (!QFile::exists(program_))
        throw QString("%1 program, %2, was not found.").arg(friendlyProgramName()).arg(program_);

    QFileInfo fileInfo(program_);
    if (!fileInfo.isFile() || !fileInfo.isExecutable())
        throw QString("%1 program, %2, is not a valid executable.").arg(friendlyProgramName()).arg(program_);

    if (!hasValidOptions())
        throw QString("One or more program options is invalid.");

    qDebug() << Q_FUNC_INFO << program_ << optionProfile_.argumentList(options_.asVector());

    process_->start(program_, optionProfile_.argumentList(options_.asVector()), openMode);
}

/**
  * @param exitCode [int]
  */
void AbstractProcessWrapper::handleExitCode(int exitCode)
{
    if (exitCode == 0)
        handleFinished();
    else
        handleError();
}

/**
  */
void AbstractProcessWrapper::handleError()
{
    emit error(id(), errorOutput_);
}

/**
  */
void AbstractProcessWrapper::handleFinished()
{
    emit finished(id(), output_);
}

/**
  * @param optionProfile [const OptionProfile &]
  */
void AbstractProcessWrapper::setOptionProfile(const OptionProfile &optionProfile)
{
    optionProfile_ = optionProfile;
}
