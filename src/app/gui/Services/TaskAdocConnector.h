/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TASKADOCCONNECTOR_H
#define TASKADOCCONNECTOR_H

#include <QtCore/QHash>
#include <QtCore/QObject>

#include "../../core/global.h"

class IEntityBuilderTask;
class ITask;
class TaskManager;

class Adoc;                           // For obtaining the repository to add newly created entities to the repo
class AdocTreeModel;                        // Needed to refresh the entire AdocTreeNode when its final entity has been completed
class AdocTreeNode;
class IRepository;
class TransientTaskColumnAdapter;     // Needed to update the transient task entity as the task changes

/**
  * TaskAdocConnector services the two-way connection between specific tasks and any associated AdocTreeNodes.
  *
  * Note any instances should be freed before their
  */
class TaskAdocConnector : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    TaskAdocConnector(TaskManager *taskManager,
                      Adoc *adoc,
                      AdocTreeModel *adocTreeModel,
                      TransientTaskColumnAdapter *transientTaskEntityColumnAdapter,
                      QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool add(IEntityBuilderTask *entityBuilderTask, AdocTreeNode *adocTreeNode);
    bool remove(AdocTreeNode *adocTreeNode);
    bool remove(IEntityBuilderTask *entityBuilderTask);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onAdocClosed();

    void onTaskAboutToStart(ITask *task);
    void onTaskAboutToBeDestroyed(ITask *task);

    void onTaskDone(ITask *task);
    void onTaskError(ITask *task);
    void onTaskProgressChanged(ITask *task);
    void onTaskStatusChanged(ITask *task);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void unwatch(ITask *task);
    void watch(ITask *task);


    // ------------------------------------------------------------------------------------------------
    // Private members
    AdocTreeModel *adocTreeModel_;
    TransientTaskColumnAdapter *transientTaskEntityColumnAdapter_;
    Adoc *adoc_;

    // Together these comprise a symmetric , bidirectional hash
    QHash<ITask *, AdocTreeNode *> taskToNode_;                 // Only IEntityBuilderTask pointers are stored here, but
                                                                // use ITask * for lookup purposes (onTaskXXXX signals)
    QHash<AdocTreeNode *, IEntityBuilderTask *> nodeToTask_;
};

#endif // TASKADOCCONNECTOR_H
