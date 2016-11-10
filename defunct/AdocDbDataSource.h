/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCDBDATASOURCE_H
#define ADOCDBDATASOURCE_H

#include "AdocTreeNode.h"
#include "DbDataSource.h"


// ------------------------------------------------------------------------------------------------
// Forward declarations
struct MpttNode;
class RichMsa;

/**
  * AdocDbDataSource extends DbDataSource with several additional methods for I/O of Adoc-specific
  * data from the database.
  *
  * Current responsibilities include:
  * o I/O of the data tree
  * o AnonSeq generation
  *
  * Future responsibilities:
  * o reading MSA subseqs
  */
class AdocDbDataSource : public DbDataSource
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    AdocDbDataSource(const QString &connectionName = QString());    //!< Construct an instance of this object with the database connection, connectionName

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Reads the data tree information from tableName (default constants::kTableDataTree) and returns an unvalidated MPTT-encoded list representation
    QList<MpttNode *> readDataTree(const QString &tableName) const;
    RichMsa *readMsa(int id, Alphabet alphabet) const;              //!< Creates and returns a RichMsa object containing the msa data for the given id and alphabet; returns zero on error
    //! Truncates tableName and inserts a row for each item in mpttNodeList and returns true on success or false otherwise
    void saveDataTree(const QList<MpttNode *> &mpttNodeList, const QString &tableName) const;
    bool saveMsa(const RichMsa &richMsa) const;                     //!< Writes richMsa to the database and returns true on success or false otherwise
    //! Erases those records contained in nodeTypeRecords
    void eraseRecords(QHash<AdocTreeNode::NodeType, QStringList> nodeTypeRecords) const;

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    RichMsa *readAminoMsa(int id) const;
    RichMsa *readDnaMsa(int id) const;
    RichMsa *readRnaMsa(int id) const;

    QHash<AdocTreeNode::NodeType, QList<QString> > deleteSqlQueries_;
};

Q_DECLARE_METATYPE(AdocDbDataSource)

#endif // ADOCDBDATASOURCE_H
