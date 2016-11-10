/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SetEntityDataCommand.h"
#include "../models/ColumnAdapters/AbstractColumnAdapter.h"
#include "../../core/Entities/IEntity.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param columnAdapter [IColumnAdapter *]
  * @param entity [IEntitySPtr &]
  * @param column [int]
  * @param value [const QVariant &]
  * @param parentCommand [QUndoCommand *]
  */
SetEntityDataCommand::SetEntityDataCommand(AbstractColumnAdapter *columnAdapter,
                                           const IEntitySPtr &entity,
                                           int column,
                                           const QVariant &value,
                                           QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      columnAdapter_(columnAdapter),
      entity_(entity),
      column_(column),
      value_(value)
{
    ASSERT(columnAdapter_ != nullptr);
    ASSERT(entity_);

    oldValue_ = columnAdapter_->data(entity_, column);
    setText(QString("Setting data entity (%1) to %2").arg(entity->id()).arg(value.toString().left(20)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SetEntityDataCommand::redo()
{
    columnAdapter_->setDataPrivate(entity_, column_, value_);
}

/**
  */
void SetEntityDataCommand::undo()
{
    columnAdapter_->setDataPrivate(entity_, column_, oldValue_);
}
