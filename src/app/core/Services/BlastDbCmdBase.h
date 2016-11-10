/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDBCMDBASE_H
#define BLASTDBCMDBASE_H

#include <QtCore/QString>

#include "AbstractProcessWrapper.h"

class BlastDbCmdBase : public AbstractProcessWrapper
{
    Q_OBJECT

public:
    explicit BlastDbCmdBase(QObject *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString blastDbCmdPath() const;                             //!< Returns the current path of the blastdbcmd program
    int id() const;                                             //!< Returns the id of the current find request


public Q_SLOTS:
    void setBlastDbCmdPath(const QString &path);                //!< Set the location of the blastdbcmd executable to path


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void preStartCheck();

    // ------------------------------------------------------------------------------------------------
    // Protected members
    int id_;                            //!< Current id of path being searched


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void checkBlastDbCmdPath(const QString &path) const;        //!< Checks if path points to a valid executable and throws an exception if it does not

    // ------------------------------------------------------------------------------------------------
    // Private members
    QString blastDbCmdPath_;            //!< Path to blastdbcmd executable
};

#endif // BLASTDBCMDBASE_H
