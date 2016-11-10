/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MAKEBLASTDATABASEWRAPPER_H
#define MAKEBLASTDATABASEWRAPPER_H

#include <QtCore/QString>

#include "AbstractProcessWrapper.h"
#include "../global.h"

class QString;

class MakeBlastDatabaseWrapper : public AbstractProcessWrapper
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit MakeBlastDatabaseWrapper(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    int fastaTick() const;
    QString friendlyProgramName() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void formatDatabase(const QString &sequenceFile);
    bool setFastaTick(int newFastaTick);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void progressChanged(int formattedSequences);
    void formatFininshed(const QString &databaseFile);


protected Q_SLOTS:
    virtual void reset();                               //!< Resets the internal process variables to their default state


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void handleError();
    void handleFinished();
    void onReadyReadStandardError();


private:
    QString sequenceFile_;
    int fastaTick_;
    bool emittedError_;         // True if we catch and handle the error in the readyReadStandardError message
};

#endif // MAKEBLASTDATABASEWRAPPER_H
