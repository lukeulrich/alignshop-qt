/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTBLASTDATABASEINSTALLER_H
#define ABSTRACTBLASTDATABASEINSTALLER_H

#include "IBlastDatabaseInstaller.h"

class QNetworkAccessManager;

/**
  * AbstractBlastDatabaseInstaller implements the boilerplate code for any given blast database installer.
  *
  * Common tasks to all blast database installations (e.g. checking blast paths, changing the active status) are
  * performed in this abstract class. Specifically, subclasses should override the virtual execute method which is
  * called if all pre-install checks succeed. Finally, when the subclass installation has finished or errored, the
  * teardown method should be called to provide this class an opportunity to perform post-install/post-error tasks.
  */
class AbstractBlastDatabaseInstaller : public IBlastDatabaseInstaller
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct an instance with title and owned by parent
    AbstractBlastDatabaseInstaller(const QString &title, QObject *parent);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual QString actionText() const;                     //!< Text describing the current action
    virtual QDir destinationDirectory() const;              //!< Destination directory for the BLAST database
    virtual bool isActive() const;                          //!< Returns true if an install is underway; false otherwise
    //! Returns the network access manager
    virtual QNetworkAccessManager *networkAccessManager() const;
    //! Sets the network access manager to qnam
    virtual void setNetworkAccessManager(QNetworkAccessManager *qnam);
    virtual QString title() const;                          //!< Blast database title


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    //! Sets the destination directory to directory and returns true on success; false otherwise
    virtual bool setDestinationDirectory(const QDir &directory);
    virtual void start();                                  //!< Start the install


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void execute() = 0;                             //!< Subclasses should override this method
    virtual void setActionText(const QString &actionText);  //!< Sets the action text to actionText and emits the actionTextChanged signal if appropriate
    virtual void setActive(bool active);                    //!< Sets the active state to active and emits the activeChanged signal if appropriate
    void teardown();                                        //!< Performs post-execute tasks

    QNetworkAccessManager *qnam_;       //!< Network manager instance

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QString title_;                     //!< BLAST database title
    QString actionText_;                //!< Current action text
    QDir destinationDirectory_;         //!< Destination directory
    bool active_;                       //!< Flag indicating whether an install is currently in progress
};

#endif // ABSTRACTBLASTDATABASEINSTALLER_H
