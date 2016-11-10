/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REMOVEADOCTREENODESCOMMAND_H
#define REMOVEADOCTREENODESCOMMAND_H

#include <QtCore/QVector>
#include <QtGui/QUndoCommand>

#include "../../core/global.h"
#include "../../core/AdocTreeNode.h"

class QModelIndex;
class AdocTreeModel;

class RemoveAdocTreeNodesCommand : public QUndoCommand
{
public:
    RemoveAdocTreeNodesCommand(AdocTreeModel *adocTreeModel, int row, int count, AdocTreeNode *parentNode, QUndoCommand *parent = nullptr);
    ~RemoveAdocTreeNodesCommand();

    virtual void redo();
    virtual void undo();

private:
    AdocTreeModel *adocTreeModel_;
    int row_;                           // Row at which these nodes were appended
    int count_;
    AdocTreeNode *parentNode_;

    AdocTreeNodeVector adocTreeNodeVector_;
};

#endif // REMOVEADOCTREENODESCOMMAND_H
