/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOVEADOCTREENODESCOMMAND_H
#define MOVEADOCTREENODESCOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>

#include "../../core/global.h"

class QModelIndex;
class AdocTreeModel;
class AdocTreeNode;

class MoveAdocTreeNodesCommand : public QUndoCommand
{
public:
    explicit MoveAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                                      int srcRow,
                                      int count,
                                      AdocTreeNode *srcParentNode,
                                      AdocTreeNode *dstParentNode,
                                      QUndoCommand *parentCommand = nullptr);

    void redo();
    void undo();

private:
    AdocTreeModel *adocTreeModel_;
    int srcRow_;
    int count_;
    AdocTreeNode *srcParentNode_;
    AdocTreeNode *dstParentNode_;
};

#endif // MOVEADOCTREENODESCOMMAND_H
