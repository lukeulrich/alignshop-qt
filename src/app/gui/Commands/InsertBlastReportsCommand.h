/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INSERTBLASTREPORTSCOMMAND_H
#define INSERTBLASTREPORTSCOMMAND_H

#include <QtCore/QDir>

#include "InsertTaskNodesCommand.h"

class QWidget;

class TaskManager;

class InsertBlastReportsCommand : public InsertTaskNodesCommand
{
public:
    InsertBlastReportsCommand(AdocTreeModel *adocTreeModel,
                              AdocTreeNode *adocTreeNode,
                              AdocTreeNode *parentNode,
                              TaskManager *taskManager,
                              QWidget *widget = nullptr,          // For displaying confirmation messages
                              QUndoCommand *parentCommand = nullptr);

    InsertBlastReportsCommand(AdocTreeModel *adocTreeModel,
                              const AdocTreeNodeVector &adocTreeNodeVector,
                              AdocTreeNode *parentNode,
                              TaskManager *taskManager,
                              QWidget *widget = nullptr,          // For displaying confirmation messages
                              QUndoCommand *parentCommand = nullptr);
    ~InsertBlastReportsCommand();

private:
#ifdef QT_DEBUG
    void checkNode(AdocTreeNode *adocTreeNode);
#endif

    QDir outDirectory_;
    TaskManager *taskManager_;
    QWidget *widget_;
};

#endif // INSERTBLASTREPORTSCOMMAND_H
