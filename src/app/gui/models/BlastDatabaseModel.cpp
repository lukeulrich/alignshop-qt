/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSet>

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "BlastDatabaseModel.h"
#include "CustomRoles.h"
#include "../../core/PODs/BlastDatabaseSpec.h"
#include "../../core/Services/BlastDatabaseFinder.h"
#include "../../core/constants.h"
#include "../../core/macros.h"
#include "../../core/misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * Configure the internal blast database finder by specifying the location of the blastdbcmd program.
  *
  * @param parent [QObject *]
  */
BlastDatabaseModel::BlastDatabaseModel(QObject *parent)
    : AbstractBaseTreeModel<BlastDatabaseTreeNode>(parent),
      blastDatabaseFinder_(nullptr),
      refreshing_(false),
      refreshId_(0)
{
    blastDatabaseFinder_ = new BlastDatabaseFinder(this);
    connect(blastDatabaseFinder_, SIGNAL(foundBlastDatabases(int,QString,BlastDatabaseMetaPodVector)), SLOT(onFoundBlastDatabases(int,QString,BlastDatabaseMetaPodVector)));
    connect(blastDatabaseFinder_, SIGNAL(error(int,QString)), SLOT(onFindError(int,QString)));

    root_ = new BlastDatabaseTreeNode;
}

/**
  */
BlastDatabaseModel::~BlastDatabaseModel()
{
    if (blastDatabaseFinder_->isRunning())
        blastDatabaseFinder_->kill();

    delete root_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int BlastDatabaseModel::columnCount(const QModelIndex & /* parent*/) const
{
    return eNumberOfColumns;
}

/**
  * TODO: Check for install file and add the ready data flag
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant BlastDatabaseModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return QVariant();
    ASSERT(node->isRoot() == false);

    const BlastDatabaseMetaPod &databasePod = node->data_;

    // Top level path
    if (node->parent()->isRoot())
    {
        if (role == Qt::DisplayRole && index.column() == ePathColumn)
            return databasePod.file_;
    }
    // Leaf item
    else
    {
        ASSERT(node->parent()->parent()->isRoot());
        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case ePathColumn:
                return QFileInfo(databasePod.file_).fileName();
            case eMoleculeTypeColumn:
                if (databasePod.isProtein())
                    return "Protein";
                else if (databasePod.isNucleotide())
                    return "Nucleotide";
                return "-";
            case eTitleColumn:
                if (!databasePod.missingDependency_)
                    return databasePod.title_;
                return "-";
            case eNumberOfLettersColumn:
                if (!databasePod.missingDependency_)
                    return databasePod.nLetters_;
                return "-";
            case eNumberOfSequencesColumn:
                if (!databasePod.missingDependency_)
                    return databasePod.nSequences_;
                return "-";
            case eNumberOfBytesColumn:
                if (!databasePod.missingDependency_)
                    return databasePod.nBytes_;
                return "-";
            case eNoteColumn:
                return databasePod.note_;

            default:
                return QVariant();
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            if (index.column() == eNoteColumn && !databasePod.note_.isEmpty())
                return databasePod.note_;
        }
        else if (role == Qt::ForegroundRole)
        {
            if (databasePod.missingDependency_)
                return QColor(196, 0, 0);
        }
        else if (role == CustomRoles::kFullBlastPathRole)
        {
            // Special case to return the full blast path
            if (index.column() == ePathColumn)
            {
                return databasePod.file_;
            }
        }
    }

    return QVariant();
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags BlastDatabaseModel::flags(const QModelIndex &index) const
{
    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    if (node == nullptr || node->isRoot())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant BlastDatabaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case ePathColumn:
            return "Path";
        case eMoleculeTypeColumn:
            return "Type";
        case eTitleColumn:
            return "Title";
        case eNumberOfLettersColumn:
            return "Letters";
        case eNumberOfSequencesColumn:
            return "Sequences";
        case eNumberOfBytesColumn:
            return "Size (bytes)";
        case eNoteColumn:
            return "Note";

        default:
            return QVariant();
        }
    }

    return QVariant();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<QPersistentModelIndex>
  */
QVector<QPersistentModelIndex> BlastDatabaseModel::aminoBlastDatabases() const
{
    QVector<QPersistentModelIndex> aminoDatabases_;
    for (int i=0, z=rowCount(); i<z; ++i)
    {
        QModelIndex topLevelIndex = index(i, 0);
        for (int j=0, y=rowCount(topLevelIndex); j<y; ++j)
        {
            QModelIndex nameIndex = topLevelIndex.child(j, ePathColumn);
            BlastDatabaseTreeNode *node = nodeFromIndex(nameIndex);
            if (node->data_.isProtein())
                aminoDatabases_ << nameIndex;
        }
    }

    return aminoDatabases_;
}

/**
  * @returns QStringList
  */
QStringList BlastDatabaseModel::blastPaths() const
{
    QStringList topLevelPaths;

    for (int i=0, z=rowCount(); i<z; ++i)
        topLevelPaths << index(i, ePathColumn).data().toString();

    return topLevelPaths;
}

/**
  * @param spec [const BlastDatabaseSpec &]
  * @returns QModelIndex
  */
QModelIndex BlastDatabaseModel::find(const BlastDatabaseSpec &spec) const
{
    BlastDatabaseTreeNode::ConstIterator it = root_;
    for (; it != nullptr; ++it)
    {
        if (!it->isLeaf())
            continue;

        // Check if this node corresponds to spec
        const BlastDatabaseMetaPod pod = it->data_;

        if (pod.isProtein_ == spec.isProtein_ &&
            pod.nBytes_ == spec.nBytes_ &&
            pod.nLetters_ == spec.nLetters_ &&
            pod.nSequences_ == spec.nSequences_ &&
            data(indexFromNode(it.node(), ePathColumn)).toString() == spec.databaseName_)
        {
            return indexFromNode(it.node());
        }
    }

    return QModelIndex();
}

/**
  * @param index [const QModelIndex &]
  * @returns QString
  */
QString BlastDatabaseModel::fullDatabasePath(const QModelIndex &index) const
{
    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    if (node == nullptr || !node->isLeaf())
        return QString();

    QModelIndex parent = index.parent();
    return this->index(parent.row(), ePathColumn).data().toString() +
           QDir::separator() +
           index.sibling(index.row(), ePathColumn).data().toString();
}

/**
  * If path does not exist, will return false. Otherwise compares path to all top-level paths.
  *
  * @param path [const QString &]
  * @returns bool
  */
bool BlastDatabaseModel::hasPath(const QString &path) const
{
    return indexFromPath(path).isValid();
}

/**
  * @param path [const QString &]
  * @returns QModelIndex
  */
QModelIndex BlastDatabaseModel::indexFromPath(const QString &path) const
{
    QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty())
        // This condition occurs when the supplied path points to a non-existent node on the filesystem
        return QModelIndex();

    // Loop through all the top-level nodes and compare their paths to this one
    for (int i=0, z= rowCount(); i<z; ++i)
    {
        // It is possible that the path_ no longer exists (e.g. the user deleted it from the filesystem and has not
        // refreshed the model). If this is the case, canonicalPath() will return an empty string and because we know
        // that the requested path exists, the following test will always fail.
        QString setCanonicalPath = QDir(index(i, ePathColumn).data().toString()).canonicalPath();
        if (setCanonicalPath == canonicalPath)
            return index(i, 0);
    }

    return QModelIndex();
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool BlastDatabaseModel::isTopLevelPath(const QModelIndex &index) const
{
    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return false;

    if (node->isRoot() == false && node->parent()->isRoot())
        return true;

    return false;
}

/**
  * This method provides for configuring the path to the blastdbcmd that will be used by the internal
  * BlastDatabaseFinder instance. Because BlastDatabaseFinder::setBlastDbCmdPath() can throw an exception and this
  * method simply calls that method, it is necessary to catch any exceptions when calling this method.
  *
  * @param path [const QString &]
  */
void BlastDatabaseModel::setBlastDbCmdPath(const QString &path)
{
    blastDatabaseFinder_->setBlastDbCmdPath(path);
}

/**
  * @param paths [const QStringList &]
  */
void BlastDatabaseModel::setBlastPaths(const QStringList &paths)
{
    if (blastDatabaseFinder_->isRunning())
        blastDatabaseFinder_->kill();

    beginResetModel();
    findRequests_.clear();
    refreshing_ = false;
    refreshId_ = 0;

    root_->removeChildren();

    foreach (const QString &path, paths)
    {
        QDir dir(path);
        if (!dir.exists())
            continue;

        BlastDatabaseTreeNode *node = new BlastDatabaseTreeNode;
        node->data_.file_ = dir.canonicalPath();
        root_->appendChild(node);
    }

    endResetModel();

    refresh();
}

/**
  * @param index [const QModelIndex &]
  * @returns BlastDatabaseSpec
  */
BlastDatabaseSpec BlastDatabaseModel::specFor(const QModelIndex &index) const
{
    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    if (node == nullptr || !node->isLeaf())
        return BlastDatabaseSpec();

    return BlastDatabaseSpec(node->data_.isProtein_,
                             index.sibling(index.row(), ePathColumn).data().toString(),
                             node->data_.nLetters_,
                             node->data_.nSequences_,
                             node->data_.nBytes_);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  *
  * @param path [const QString &]
  * @returns bool
  */
bool BlastDatabaseModel::addBlastPath(const QString &path)
{
    // Does this path exist?
    QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty())
        return false;

    // If path is already in the model, then simply return true without doing anything.
    if (hasPath(path))
        return true;

    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    BlastDatabaseTreeNode *newNode = new BlastDatabaseTreeNode;
    newNode->data_.file_ = canonicalPath;
    root_->appendChild(newNode);
    endInsertRows();

    // Create a find request for this path
    QQueue<QPersistentModelIndex> queue;
    queue << index(row, 0);
    findRequests_.insert(nextFindId(), queue);

    processNextFindRequest();

    return true;
}

/**
  * @see onFindError()
  */
void BlastDatabaseModel::cancelRefresh()
{
    if (!refreshing_)
        return;

    // This will then trigger the error handler: onFindError(...)
    blastDatabaseFinder_->kill();
}

/**
  * Queues up a find request for all paths in the model. Does nothing if currently refreshing or the model is empty.
  * Removes any top level paths that no longer exist.
  */
void BlastDatabaseModel::refresh()
{
    if (refreshing_)
        return;

    if (rowCount() == 0)
        return;

    refreshId_ = nextFindId();
    refreshing_ = true;

    // Build list of all indices
    QQueue<QPersistentModelIndex> queue;
    for (int i=rowCount()-1; i >= 0; --i)
    {
        QModelIndex pathIndex = index(i, 0);
        if (!pathExists(pathIndex))
        {
            beginRemoveRows(QModelIndex(), i, i);
            root_->removeChildAt(i);
            endRemoveRows();

            continue;
        }

        queue << QPersistentModelIndex(index(i, 0));
    }

    findRequests_.insert(refreshId_, queue);

    processNextFindRequest();
}

/**
  * @param index [const QModelIndex &]
  */
void BlastDatabaseModel::refresh(const QModelIndex &index)
{
    if (!isTopLevelPath(index))
        return;

    refreshId_ = nextFindId();
    refreshing_ = true;

    // Build list of all indices
    QQueue<QPersistentModelIndex> queue;
    queue << QPersistentModelIndex(index);
    findRequests_.insert(refreshId_, queue);

    processNextFindRequest();
}

/**
  * @param path [const QString &]
  */
void BlastDatabaseModel::refresh(const QString &path)
{
    refresh(indexFromPath(path));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param id [int]
  * @param path [const QString &]
  * @param blastDatabaseMetaPods [const QVector<BlastDatabaseMetaPod> &]
  */
void BlastDatabaseModel::onFoundBlastDatabases(int id, const QString &path, const BlastDatabaseMetaPodVector &blastDatabaseMetaPods)
{
    ASSERT(findRequests_.contains(id));

    QQueue<QPersistentModelIndex> &queue = findRequests_[id];
    ASSERT(queue.isEmpty() == false);
    QModelIndex pathIndex = queue.dequeue();
    if (pathIndex.isValid())
    {
        ASSERT(pathIndex.parent().isValid() == false);
        ASSERT(pathIndex.column() == ePathColumn);

        // Does the path returned by this method match the path of the first one on our queue?
        if (path == pathIndex.data().toString())
            update(pathIndex, blastDatabaseMetaPods);
    }

    processNextFindRequest();
}

/**
  * @param id [int]
  * @param error [const QString &]
  */
void BlastDatabaseModel::onFindError(int id, const QString &error)
{
    ASSERT(findRequests_.contains(id));
    QQueue<QPersistentModelIndex> &queue = findRequests_[id];
    ASSERT(queue.isEmpty() == false);

    QModelIndex pathIndex = queue.dequeue();
    BlastDatabaseTreeNode *node = nodeFromIndex(pathIndex);
    if (node == nullptr)
        // Perhaps the node was removed before this process finished
        queue.clear();
    else if (refreshing_ && blastDatabaseFinder_->wasKilled())
    {
        // The refreshing status will be reset in the next call to processNextFindRequest()
        queue.clear();

        emit canceled();
    }
    else
    {
        // Not sure why it was killed - perhaps by the operating system. Perhaps by the destructor.
        node->data_.note_ = error;
        QModelIndex changedIndex = index(pathIndex.row(), eNoteColumn);
        emit dataChanged(changedIndex, changedIndex);
    }

    processNextFindRequest();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int BlastDatabaseModel::nextFindId() const
{
    static int x = 0;
    ++x;
    return x;
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool BlastDatabaseModel::pathExists(const QModelIndex &index) const
{
    BlastDatabaseTreeNode *node = nodeFromIndex(index);
    return node != nullptr && QDir(node->data_.file_).exists();
}

/**
  * Because this method calls findBlastDatabases which depends upon executing an external program it is vital that the
  * client wrap calling methods in a try/catch block.
  *
  * @see addBlastPath(), refresh()
  */
void BlastDatabaseModel::processNextFindRequest()
{
    if (findRequests_.isEmpty())
        return;

    // Only send one path request at a time
    if (blastDatabaseFinder_->isRunning())
        return;

    int id = findRequests_.keys().first();
    QQueue<QPersistentModelIndex> &queue = findRequests_[id];
    // Discard any indices that are no longer valid. Presumably these point to entries that have been removed since
    // they were queued.
    while (queue.size() > 0 && queue.first().isValid() == false)
        queue.dequeue();

    // Are there any requests left in this queue?
    if (queue.size() > 0)
    {
        // The current find request remains at the head of the queue.
        QModelIndex pathIndex = queue.first();
        ASSERT(pathIndex.parent().isValid() == false);
        ASSERT(pathIndex.column() == ePathColumn);
        blastDatabaseFinder_->findBlastDatabases(id, pathIndex.data().toString());
        return;
    }

    // No more valid indices left in the queue, therefore remove it and proceed to the next find request
    findRequests_.remove(id);

    // Reset refreshing status if it has completed
    if (refreshing_ && id == refreshId_)
    {
        refreshing_ = false;
        refreshId_ = 0;
    }

    // Tail-recursion
    processNextFindRequest();
}

/**
  * Comparing the BlastDatabaseMetaPods in the set referred to by pathIndex and those in blastDatabaseMetaPods:
  * o if in model, but not in the update: removeRows
  * o if not in model, but in update: addRows
  * o if in both the model and update: update any differences
  *
  * @param pathIndex [const QModelIndex &]
  * @param blastDatabaseMetaPods [const QVector<BlastDatabaseMetaPod> &]
  */
void BlastDatabaseModel::update(const QModelIndex &pathIndex, const QVector<BlastDatabaseMetaPod> &blastDatabaseMetaPods)
{
    BlastDatabaseTreeNode *parentNode = nodeFromIndex(pathIndex);
    if (parentNode == nullptr)
        return;

    // Not the root index
    ASSERT(parentNode->isRoot() == false);

    // It is a valid toplevel index, that is, a top level path
    ASSERT(parentNode->parent()->isRoot());
    ASSERT(pathIndex.column() == ePathColumn);

    // Extract the set corresponding to this top level path index
    int nModelPods = parentNode->childCount();

    QVector<int> deadPods(nModelPods);     // Rows in model before update that need to be removed
    // Initially, all rows are deemed to be dead unless "recovered" during the comparison to blastDatabasesMetaPods.
    // A -1 value indicates that it exists in the update and should not be removed from the model.
    for (int i=0; i<nModelPods; ++i)
        deadPods[i] = i;

    QSet<int> newPods;                    // Indices in blastDatabaseMetaPods that are completely new. Initially empty

    for (int i=0, z=blastDatabaseMetaPods.size(); i<z; ++i)
    {
        const BlastDatabaseMetaPod &updatePod = blastDatabaseMetaPods.at(i);
        bool isNew = true;

        // Compare the file_'s to determine if they are equivalent
        for (int j=0; j<nModelPods; ++j)
        {
            // -1 indicates that this record has already been found
            if (deadPods.at(j) == -1)
                continue;

            BlastDatabaseMetaPod &modelPod = parentNode->childAt(j)->data_;
            if (modelPod.file_ == updatePod.file_)
            {
                // Tag this row as "recovered"
                deadPods[j] = -1;
                isNew = false;

                // Title
                if (modelPod.title_ != updatePod.title_)
                {
                    modelPod.title_ = updatePod.title_;
                    QModelIndex changedIndex = pathIndex.child(j, eTitleColumn);
                    emit dataChanged(changedIndex, changedIndex);
                }

                // isProtein
                if (modelPod.isProtein_ != updatePod.isProtein_)
                {
                    modelPod.isProtein_ = updatePod.isProtein_;
                    QModelIndex changedIndex = pathIndex.child(j, eMoleculeTypeColumn);
                    emit dataChanged(changedIndex, changedIndex);
                }

                // nLetters
                if (modelPod.nLetters_ != updatePod.nLetters_)
                {
                    modelPod.nLetters_ = updatePod.nLetters_;
                    QModelIndex changedIndex = pathIndex.child(j, eNumberOfLettersColumn);
                    emit dataChanged(changedIndex, changedIndex);
                }

                // nSequences
                if (modelPod.nSequences_ != updatePod.nSequences_)
                {
                    modelPod.nSequences_ = updatePod.nSequences_;
                    QModelIndex changedIndex = pathIndex.child(j, eNumberOfSequencesColumn);
                    emit dataChanged(changedIndex, changedIndex);
                }

                // nBytes
                if (modelPod.nBytes_ != updatePod.nBytes_)
                {
                    modelPod.nBytes_ = updatePod.nBytes_;
                    QModelIndex changedIndex = pathIndex.child(j, eNumberOfBytesColumn);
                    emit dataChanged(changedIndex, changedIndex);
                }

                break;
            }
        }

        if (isNew)
            newPods << i;
    }

    // ------------------------
    // Now remove any dead pods
    if (deadPods.size() > 0)
    {
        QVector<int> deadPodIndices;
        foreach (int index, deadPods)
            if (index != -1)
                deadPodIndices << index;

        // Because of the above method for finding dead pods, it should be safe to assume that deadPodIndices is
        // sorted in ascending order.
        QVector<QPair<int, int> > rangesToRemove = ::convertIntVectorToRanges(deadPodIndices);
        for (int j=rangesToRemove.size()-1; j>= 0; --j)
        {
            int row = rangesToRemove.at(j).first;
            int count = rangesToRemove.at(j).second - rangesToRemove.at(j).first + 1;
            beginRemoveRows(pathIndex, row, count);
            parentNode->removeChildren(row, count);
            endRemoveRows();
        }
    }

    // ------------------------
    // Add any new pods
    if (newPods.size() > 0)
    {
        int row = rowCount();
        int lastRow = row + newPods.size() - 1;
        beginInsertRows(pathIndex, row, lastRow);
        foreach (int newPodRow, newPods)
            parentNode->appendChild(new BlastDatabaseTreeNode(blastDatabaseMetaPods.at(newPodRow)));
        endInsertRows();
    }
}
