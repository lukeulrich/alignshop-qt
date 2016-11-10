/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractBlastDatabaseInstaller.h"
#include "../global.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param title [const QString &]
  * @param parent [QObject *]
  */
AbstractBlastDatabaseInstaller::AbstractBlastDatabaseInstaller(const QString &title, QObject *parent)
    : IBlastDatabaseInstaller(parent),
      title_(title),
      active_(false)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString AbstractBlastDatabaseInstaller::actionText() const
{
    return actionText_;
}

/**
  * @returns QDir
  */
QDir AbstractBlastDatabaseInstaller::destinationDirectory() const
{
    return destinationDirectory_;
}

/**
  * @returns bool
  */
bool AbstractBlastDatabaseInstaller::isActive() const
{
    return active_;
}

/**
  * @returns QNetworkAccessManager *
  */
QNetworkAccessManager *AbstractBlastDatabaseInstaller::networkAccessManager() const
{
    return qnam_;
}

/**
  * @param qname [QNetworkAccessManager *]
  */
void AbstractBlastDatabaseInstaller::setNetworkAccessManager(QNetworkAccessManager *qnam)
{
    ASSERT(qnam_ == nullptr || !active_);

    qnam_ = qnam;
}

/**
  * @returns QString
  */
QString AbstractBlastDatabaseInstaller::title() const
{
    return title_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Do not permit changing the destination directory if an install is underway.
  *
  * @param directory [const QDir &]
  * @returns bool
  */
bool AbstractBlastDatabaseInstaller::setDestinationDirectory(const QDir &directory)
{
    if (!isActive() && directory.exists())
    {
        destinationDirectory_ = directory;
        return true;
    }

    return false;
}

/**
  */
void AbstractBlastDatabaseInstaller::start()
{
    if (active_)
        return;

    // Check that the destination directory is not empty and exists
    if (destinationDirectory_.path().isEmpty())
    {
        emit error("No destination directory has been defined.");
        return;
    }

    if (!destinationDirectory_.exists())
    {
        emit error(QString("Destination directory, %1, does not exist.").arg(destinationDirectory_.path()));
        return;
    }

    // Everything looks good - kick off the install
    setActive(true);
    setActionText("Initializing...");
    execute();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param actionText [const QString &]
  */
void AbstractBlastDatabaseInstaller::setActionText(const QString &actionText)
{
    if (actionText_ == actionText)
        return;

    actionText_ = actionText;
    emit actionTextChanged(actionText_);
}

/**
  * @param active [bool]
  */
void AbstractBlastDatabaseInstaller::setActive(bool active)
{
    if (active_ == active)
        return;

    active_ = active;
    emit activeChanged(active_);
}

/**
  */
void AbstractBlastDatabaseInstaller::teardown()
{
    setActive(false);
}
