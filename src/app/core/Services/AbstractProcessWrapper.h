/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTPROCESSWRAPPER_H
#define ABSTRACTPROCESSWRAPPER_H

#include <QtCore/QProcess>      // Included here for access to its enumerated types
#include "../util/OptionProfile.h"
#include "../util/OptionSet.h"
#include "../global.h"

/**
  * AbstractProcessWrapper encapsulates the majority of the boilerplate code and interface for asynchronously executing
  * external programs.
  *
  * Only one process may be active at any time per instance and each process should be tagged with a well-defined
  * integral id (default 0). This id value is used when emitting signals to connected components.

  * Subclasses should define and implement relevant slots for starting the external program.
  */
class AbstractProcessWrapper : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    ~AbstractProcessWrapper();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QByteArray errorOutput() const;                     //!< Returns the data written to standard error
    virtual QString friendlyProgramName() const;        //!< Returns a human-friendly name of the program
    bool hasValidOptions() const;                       //!< Returns true if the last call to setOptions was successful; false otherwise
    virtual int id() const;                             //!< Returns the arbitrary user-defined id for the current process; this base class implementation returns 0
    bool isRunning() const;                             //!< Returns true if the process is currently running; false otherwise
    QByteArray output() const;                          //!< Returns the data written to standard output
    QString program() const;                            //!< Returns the currently configured program
    bool setOptions(const OptionSet &newOptions);       //!< Sets the program options to newOptions and returns true on success; false otherwise
    bool wasKilled() const;                             //!< Returns true if the most recent process (per class instance) was killed; false otherwise


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void kill(const QString &killMessage = QString());  //!< Kill process (if running)
    void setProgram(const QString &newProgram);         //!< Sets the program to execute to newProgram


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void error(int id, const QString &errorMessage);    //!< Emitted when the process has encountered an error or killed
    void finished(int id, const QByteArray &output);    //!< Emitted when the process has finished; output contains the stdout


protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Protected slots
    //!< Called when the process has encountered an error
    virtual void onProcessError(QProcess::ProcessError processError);
    //!< Called when the process has finished - either normally or because of a crash
    virtual void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    virtual void onProcessStarted();                    //!< Called when the process has started
    virtual void onReadyReadStandardError();            //!< Called when there is data available on standard error
    virtual void onReadyReadStandardOutput();           //!< Called when there is data available on standard output
    virtual void reset();                               //!< Resets the internal process variables to their default state


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    explicit AbstractProcessWrapper(QObject *parent = nullptr);
    void execute(QProcess::OpenMode openMode = QProcess::ReadWrite);
    virtual void handleExitCode(int exitCode);
    virtual void handleError();                         //!< Virtual method that is called when the process has encountered and error; base class implementation simply emits an error signal
    virtual void handleFinished();                      //!< Virtual method that is called when processing has finished successfully; base class implementation simply emits the finished signal
    void setOptionProfile(const OptionProfile &optionProfile);


    // ------------------------------------------------------------------------------------------------
    // Protected members
    QString program_;                   //!< Program to execute
    OptionProfile optionProfile_;       //!< Program option profile
    OptionSet options_;                 //!< Options to run program with
    QProcess *process_;                 //!< Process instance
    QByteArray errorOutput_;            //!< Storage of all data dumped to stderr
    QByteArray output_;                 //!< Storage of all data dumped to stdout
    bool killed_;                       //!< Set to true if the process was attempted to be killed
    QString killMessage_;               //!< Optional message to output via the error signal when a process is killed
};

#endif // ABSTRACTPROCESSWRAPPER_H
