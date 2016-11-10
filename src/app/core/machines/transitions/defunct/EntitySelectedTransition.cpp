/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPlainTextEdit>

#include "EntitySelectedTransition.h"
#include "../events/CustomEventTypes.h"
#include "../events/CurrentRowChangedEvent.h"
#include "../../models/MultiSeqTableModel.h"
#include "../../../core/macros.h"
#include "../../../core/TreeNode.h"
#include "../../../core/AdocTreeNode_fwd.h"
#include "../../../core/AdocNodeData.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param model [MultiSeqTableModel *]
  * @param plainTextEdit [QPlainTextEdit *]
  */
EntitySelectedTransition::EntitySelectedTransition(MultiSeqTableModel *model, QPlainTextEdit *plainTextEdit)
    : model_(model), plainTextEdit_(plainTextEdit)
{
    ASSERT(model != nullptr);
    ASSERT(plainTextEdit != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param event [QEvent *]
  * @returns bool
  */
bool EntitySelectedTransition::eventTest(QEvent *event)
{
    if (event->type() != QEvent::Type(constants::kCurrentRowChangedType))
        return false;

    CurrentRowChangedEvent *rowChangeEvent = static_cast<CurrentRowChangedEvent *>(event);
    if (!rowChangeEvent->current_.isValid())
        return false;

    AdocTreeNode *node = model_->nodeFromIndex(rowChangeEvent->current_);
    if (node == nullptr)
        return false;

    switch (node->data_.nodeType_)
    {
    case eRootNode:
    case eGroupNode:
        return false;

    default:
        return true;
    }
}

/**
  * @param event [QEvent *]
  */
void EntitySelectedTransition::onTransition(QEvent *event)
{
    plainTextEdit_->setEnabled(true);
    CurrentRowChangedEvent *rowChangeEvent = static_cast<CurrentRowChangedEvent *>(event);
    {
        // Deal with the previous node if any first
        if (rowChangeEvent->previous_.isValid())
        {
            QModelIndex notesIndex = model_->index(rowChangeEvent->previous_.row(), MultiSeqTableModel::eNotesColumn);
            model_->setData(notesIndex, plainTextEdit_->toPlainText());
        }

        plainTextEdit_->clear();

        // then the current node
        if (rowChangeEvent->current_.isValid())
        {
            QModelIndex notesIndex = model_->index(rowChangeEvent->current_.row(), MultiSeqTableModel::eNotesColumn);
            plainTextEdit_->setPlainText(model_->data(notesIndex).toString());
        }
    }
}
