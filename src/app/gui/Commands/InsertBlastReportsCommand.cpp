/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>

#include <QtGui/QAbstractButton>
#include <QtGui/QMessageBox>

#include "InsertBlastReportsCommand.h"

#include "../Services/TaskManager.h"
#include "../Services/Tasks/ITask.h"
#include "../Services/Tasks/BlastTask.h"

#include "../../core/Entities/BlastReport.h"
#include "../../core/Entities/TransientTask.h"

#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNode [AdocTreeNode *]
  * @param parentNode [AdocTreeNode *]
  * @param taskManager [TaskManager *]
  * @param widget [QWidget *]
  * @param parentCommand [QUndoCommand *]
  */
InsertBlastReportsCommand::InsertBlastReportsCommand(AdocTreeModel *adocTreeModel,
                                                     AdocTreeNode *adocTreeNode,
                                                     AdocTreeNode *parentNode,
                                                     TaskManager *taskManager,
                                                     QWidget *widget,
                                                     QUndoCommand *parentCommand)
    : InsertTaskNodesCommand(adocTreeModel, adocTreeNode, parentNode, taskManager, widget, parentCommand)
{
#ifdef QT_DEBUG
    checkNode(adocTreeNode);
#endif

    const TransientTaskSPtr &transientTask = boost::shared_static_cast<TransientTask>(adocTreeNode->entity());
    outDirectory_ = static_cast<BlastTask *>(transientTask->task())->outDirectory();
}

/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNodeVector [const AdocTreeNodeVector &]
  * @param parentNode [AdocTreeNode *]
  * @param taskManager [TaskManager *]
  * @param widget [QWidget *]
  * @param parentCommand [QUndoCommand *]
  */
InsertBlastReportsCommand::InsertBlastReportsCommand(AdocTreeModel *adocTreeModel,
                                                     const AdocTreeNodeVector &adocTreeNodeVector,
                                                     AdocTreeNode *parentNode,
                                                     TaskManager *taskManager,
                                                     QWidget *widget, QUndoCommand *parentCommand)
    : InsertTaskNodesCommand(adocTreeModel, adocTreeNodeVector, parentNode, taskManager, widget, parentCommand)
{
#ifdef QT_DEBUG
    foreach (AdocTreeNode *adocTreeNode, adocTreeNodeVector)
        checkNode(adocTreeNode);
#endif

    const TransientTaskSPtr &transientTask = boost::shared_static_cast<TransientTask>(adocTreeNodeVector.first()->entity());
    outDirectory_ = static_cast<BlastTask *>(transientTask->task())->outDirectory();
}

/**
  * Check for any completed blast report entities that should have their files removed. For instance, the user runs a
  * BLAST task (which completes), then undoes the operation, then performs another command. This will have left an
  * orphan BLAST file on the file system. This can be handled here.
  */
InsertBlastReportsCommand::~InsertBlastReportsCommand()
{
    foreach (AdocTreeNode *adocTreeNode, adocTreeNodeVector_)
    {
        if (adocTreeNode->nodeType_ == eTransientTaskNode)
            continue;

#ifdef QT_DEBUG
        checkNode(adocTreeNode);
#endif

        const BlastReportSPtr &blastReport = boost::shared_static_cast<BlastReport>(adocTreeNode->entity());

        if (blastReport->isNew())
            // Remove the file!
            QFile::remove(outDirectory_.filePath(blastReport->sourceFile()));
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
#ifdef QT_DEBUG
/**
  * @param adocTreeNode [AdocTreeNode *]
  */
void InsertBlastReportsCommand::checkNode(AdocTreeNode *adocTreeNode)
{
    ASSERT(adocTreeNode != nullptr);
    ASSERT(adocTreeNode->nodeType_ == eTransientTaskNode || adocTreeNode->nodeType_ == eBlastReportNode);
    ASSERT(adocTreeNode->entity() != nullptr);

    if (adocTreeNode->nodeType_ == eTransientTaskNode)
    {
        ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);
        ASSERT(boost::shared_dynamic_cast<TransientTask>(adocTreeNode->entity()));
        ASSERT(dynamic_cast<BlastTask *>(boost::shared_static_cast<TransientTask>(adocTreeNode->entity())->task()));
    }
    else
    {
        ASSERT(adocTreeNode->entity()->type() == eBlastReportEntity);
        ASSERT(boost::shared_dynamic_cast<BlastReport>(adocTreeNode->entity()));
    }
}
#endif
