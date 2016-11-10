/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASEMODEL_H
#define BLASTDATABASEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QQueue>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "AbstractBaseTreeModel.h"
#include "../../core/ValueTreeNode.h"               // Needed to properly define the blast tree node typedef
#include "../../core/PODs/BlastDatabaseMetaPod.h"
#include "../../core/global.h"

class BlastDatabaseFinder;
struct BlastDatabaseSpec;

typedef ValueTreeNode<BlastDatabaseMetaPod> BlastDatabaseTreeNode;

/**
  * BlastDatabaseModel provides a two-level tree model of user-defined file system paths and any blast databases located
  * in each path (not recursive).
  *
  * Currently, the only modifications that may be made are to add or remove paths, and refresh the paths. Available
  * blast databases are determined via BlastDatabaseFinder. Directly removing of the BLAST databases from the filesystem
  * is not currently supported, although this should be implemented at a later point. Relevant concerns include what
  * should happen if attempt to remove a BLAST database that is currently in use.
  *
  * Duplicate paths are not allowed.
  *
  * It is not possible to queue multiple refreshes.
  */
class BlastDatabaseModel : public AbstractBaseTreeModel<BlastDatabaseTreeNode>
{
    Q_OBJECT

public:
    enum Columns
    {
        ePathColumn = 0,
        eMoleculeTypeColumn,
        eTitleColumn,
        eNumberOfLettersColumn,
        eNumberOfSequencesColumn,
        eNumberOfBytesColumn,
        eNoteColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    BlastDatabaseModel(QObject *parent = nullptr);                  //!< Trivial constructor
    ~BlastDatabaseModel();                                          //!< Destructor that kills all outstanding requests


    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //!< Returns the number of columns for the rows beneath parent
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    //! Returns the data for index and role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    //! Returns the flags for index
    Qt::ItemFlags flags(const QModelIndex &index = QModelIndex()) const;
    //! Returns the header data for section, orientation, and role
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QVector<QPersistentModelIndex> aminoBlastDatabases() const;     //!< Returns a vector of indices of all available amino blast databases
    QStringList blastPaths() const;                                 //!< Returns a QStringList of all the top level paths
    QModelIndex find(const BlastDatabaseSpec &spec) const;          //!< Returns the nameIndex of the BlastDatabase that corresponds to spec; otherwise an invalid model index is returned
    QString fullDatabasePath(const QModelIndex &index) const;       //!< Returns the full absolute path to the database referenced by index or an empty QString if index is a top level path or an invalid index
    bool hasPath(const QString &path) const;                        //!< Returns true if path is contained by this model; false otherwise
    QModelIndex indexFromPath(const QString &path) const;           //!< Returns the index that corresponds to path or an invalid QModelIndex if path is not found
    bool isTopLevelPath(const QModelIndex &index) const;            //!< Returns true if index is a top level path index; false otherwise
    void setBlastDbCmdPath(const QString &path);                    //!< Sets the blastdbcmd path of the internal BlastDatabaseFinder to path
    void setBlastPaths(const QStringList &paths);                   //!< Resets the model to contain paths
    BlastDatabaseSpec specFor(const QModelIndex &index) const;      //!< Returns the blast database specification associated with index


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    bool addBlastPath(const QString &path);                         //!< Returns true if path was successfully added to this model; false otherwise
    void cancelRefresh();                                           //!< Cancels any active refresh request; does nothing if a refresh request has not been initiated
    void refresh();                                                 //!< Update all blast database sets
    void refresh(const QModelIndex &index);                         //!< Update the blast databases under index
    void refresh(const QString &path);                              //!< Update the blast databases under path if it is present


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void canceled();                                                //!< Emitted after a cancel request has successfully finished


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    //! Called when blastDatabaseFinder_ has successfully completed finding blast databases in path
    void onFoundBlastDatabases(int id, const QString &path, const BlastDatabaseMetaPodVector &blastDatabaseMetaPods);
    void onFindError(int id, const QString &error);                 //!< Called whenever blastDatabaseFinder emits an error


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int nextFindId() const;                                         //!< Returns a unique, monotonically increasing sequence integer starting with 1
    bool pathExists(const QModelIndex &index) const;                //!< Returns true if the file_ associated with index exists on the filesystem; false otherwise
    void processNextFindRequest();                                  //!< Sends the next find request
    //! Updates the BlastDatabaseSet at pathIndex to blastDatabaseMetaPods
    void update(const QModelIndex &pathIndex, const QVector<BlastDatabaseMetaPod> &blastDatabaseMetaPods);


    // ------------------------------------------------------------------------------------------------
    // Private members
    BlastDatabaseFinder *blastDatabaseFinder_;                      //!< Internal instance of a blast database finder
    QMap<int, QQueue<QPersistentModelIndex> > findRequests_;        //!< Outstanding find requests by id
    bool refreshing_;                                               //!< Indicates whether this model is currently being refreshed
    int refreshId_;                                                 //!< Id of refresh request
};

#endif // BLASTDATABASEMODEL_H
