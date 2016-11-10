/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SETENTITYDATACOMMAND_H
#define SETENTITYDATACOMMAND_H

#include <QtCore/QVariant>
#include <QtGui/QUndoCommand>
#include "../../core/global.h"
#include "../../core/Entities/IEntity.h"

class AbstractColumnAdapter;

class SetEntityDataCommand : public QUndoCommand
{
public:
    SetEntityDataCommand(AbstractColumnAdapter *columnAdapter, const IEntitySPtr &entity, int column, const QVariant &value, QUndoCommand *parentCommand = nullptr);

    void redo();
    void undo();

private:
    AbstractColumnAdapter *columnAdapter_;
    IEntitySPtr entity_;
    int column_;
    QVariant value_;
    QVariant oldValue_;
};

#endif // SETENTITYDATACOMMAND_H
