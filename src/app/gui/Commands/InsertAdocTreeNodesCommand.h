/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INSERTADOCTREENODESCOMMAND_H
#define INSERTADOCTREENODESCOMMAND_H

#include <QtCore/QVector>

#include "ConditionalUndoCommand.h"
#include "../../core/global.h"
#include "../../core/AdocTreeNode.h"

class QModelIndex;
class QUndoCommand;

class AdocTreeModel;


class InsertAdocTreeNodesCommand : public ConditionalUndoCommand
{
public:
    InsertAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                               AdocTreeNode *adocTreeNode,
                               AdocTreeNode *parentNode,
                               QUndoCommand *parentCommand = nullptr);

    InsertAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                               const AdocTreeNodeVector &adocTreeNodeVector,
                               AdocTreeNode *parentNode,
                               QUndoCommand *parentCommand = nullptr);

    ~InsertAdocTreeNodesCommand();

    void redo();
    void undo();

protected:
    AdocTreeModel *adocTreeModel_;
    AdocTreeNodeVector adocTreeNodeVector_;
    AdocTreeNode *parentNode_;
    int row_;                           // Row at which these nodes were appended
    int count_;

    bool done_;                         // True if this action has taken place and not been undone; vice versa
};

#endif // INSERTADOCTREENODESCOMMAND_H
