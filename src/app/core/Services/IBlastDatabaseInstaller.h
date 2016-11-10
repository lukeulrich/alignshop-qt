/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IBLASTDATABASEINSTALLER_H
#define IBLASTDATABASEINSTALLER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDir>

class QNetworkAccessManager;

/**
  * IBlastDatabaseInstaller defines the common interface for installing a blast database.
  *
  * Subclasses will implement the specific logic necessary for installing a given database with its particular
  * requirements. In general, this will involve update
  */
class IBlastDatabaseInstaller : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct an instance of this interface which is owned by parent
    IBlastDatabaseInstaller(QObject *parent) : QObject(parent) {}
    virtual ~IBlastDatabaseInstaller() {}                   //!< Virtual destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual QString actionText() const = 0;                 //!< Text describing the current action
    virtual QDir destinationDirectory() const = 0;          //!< Destination directory for the BLAST database
    virtual bool isActive() const = 0;                      //!< Returns true if an install is underway; false otherwise
    //! Returns the network access manager
    virtual QNetworkAccessManager *networkAccessManager() const = 0;
    //! Sets the network access manager to qnam
    virtual void setNetworkAccessManager(QNetworkAccessManager *qnam) = 0;
    virtual QString title() const = 0;                      //!< Blast database title


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    virtual void cancel() = 0;                              //!< Cancel any current install
    virtual void cleanUp() = 0;                             //!< Clean up any leftover or intermediate files (e.g. those remaining from an errored process)
    //! Sets the destination directory to directory and returns true on success; false otherwise
    virtual bool setDestinationDirectory(const QDir &directory) = 0;
    virtual void start() = 0;                               //!< Start the install


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void actionTextChanged(const QString &actionText);      //!< Emitted when the action text has changed
    void activeChanged(bool active);                        //!< Emitted when the active state has changed
    void canceled();                                        //!< Emitted after a cancel request has completed
    void error(const QString &error);                       //!< Emitted when an error has occurred during an installation
    void finished();                                        //!< Emitted when the BLAST database has finished installing
    void progressChanged(qint64 done, qint64 total);        //!< Emitted whenever the install progress has completed done out of total steps


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Sets the action text to actionText and emits the actionTextChanged signal if appropriate
    virtual void setActionText(const QString &actionText) = 0;
    virtual void setActive(bool active) = 0;                //!< Sets the active state to active and emits the activeChanged signal if appropriate
};

#endif // IBLASTDATABASEINSTALLER_H
