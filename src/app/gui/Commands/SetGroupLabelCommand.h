/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SETGROUPLABELCOMMAND_H
#define SETGROUPLABELCOMMAND_H

#include <QtCore/QVariant>
#include <QtGui/QUndoCommand>
#include "../../core/global.h"

class AdocTreeModel;
class AdocTreeNode;

class SetGroupLabelCommand : public QUndoCommand
{
public:
    SetGroupLabelCommand(AdocTreeModel *treeModel, AdocTreeNode *groupNode, const QVariant &value, QUndoCommand *parentCommand = nullptr);

    void redo();
    void undo();

private:
    AdocTreeModel *treeModel_;
    AdocTreeNode *groupNode_;
    QVariant value_;
    QVariant oldValue_;
};

#endif // SETGROUPLABELCOMMAND_H
