/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "EntityDeselectedTransition.h"
#include "../events/CustomEventTypes.h"
#include "../events/CurrentRowChangedEvent.h"
#include "../../models/MultiSeqTableModel.h"
#include "../../../core/macros.h"
#include "../../../core/TreeNode.h"
#include "../../../core/AdocTreeNode_fwd.h"
#include "../../../core/AdocNodeData.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param model [MultiSeqTableModel *]
  */
EntityDeselectedTransition::EntityDeselectedTransition(MultiSeqTableModel *model)
    : model_(model)
{
    ASSERT(model != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param event [QEvent *]
  * @returns bool
  */
bool EntityDeselectedTransition::eventTest(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;

    if (event->type() != QEvent::Type(constants::kCurrentRowChangedType))
        return false;

    CurrentRowChangedEvent *rowChangeEvent = static_cast<CurrentRowChangedEvent *>(event);

    qDebug() << rowChangeEvent->current_.data() << rowChangeEvent->previous_.data();

    if (!rowChangeEvent->current_.isValid())
        return true;

    AdocTreeNode *node = model_->nodeFromIndex(rowChangeEvent->current_);
    if (node == nullptr)
        return false;

    switch (node->data_.nodeType_)
    {
    case eRootNode:
    case eGroupNode:
        return true;

    default:
        return false;
    }
}
