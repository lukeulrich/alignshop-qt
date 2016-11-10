/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INSERTTASKNODESCOMMAND_H
#define INSERTTASKNODESCOMMAND_H

#include "InsertAdocTreeNodesCommand.h"

class QWidget;

class TaskManager;

class InsertTaskNodesCommand : public InsertAdocTreeNodesCommand
{
public:
    InsertTaskNodesCommand(AdocTreeModel *adocTreeModel,
                           AdocTreeNode *adocTreeNode,
                           AdocTreeNode *parentNode,
                           TaskManager *taskManager,
                           QWidget *widget = nullptr,          // For displaying confirmation messages
                           QUndoCommand *parentCommand = nullptr);
    InsertTaskNodesCommand(AdocTreeModel *adocTreeModel,
                           const AdocTreeNodeVector &adocTreeNodeVector,
                           AdocTreeNode *parentNode,
                           TaskManager *taskManager,
                           QWidget *widget = nullptr,          // For displaying confirmation messages
                           QUndoCommand *parentCommand = nullptr);

    bool acceptUndo() const;
    void undo();

private:
    TaskManager *taskManager_;
    QWidget *widget_;
};

#endif // INSERTTASKNODESCOMMAND_H
