/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocDbDataSource.h"

#include <QtCore/QStringBuilder>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "exceptions/DatabaseError.h"

#include "AdocTreeNode.h"
#include "MpttNode.h"
#include "RichMsa.h"
#include "RichSubseq.h"


#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param connectionName [const QString &]
  *
  * @throws InvalidConnectionError
  */
AdocDbDataSource::AdocDbDataSource(const QString &connectionName) : DbDataSource(connectionName)
{
    deleteSqlQueries_[AdocTreeNode::SubseqAminoType].append(QString("DELETE FROM %1 WHERE id IN (%2)").arg(constants::kTableAminoSubseqs, "%1"));
    deleteSqlQueries_[AdocTreeNode::SubseqDnaType].append(QString("DELETE FROM %1 WHERE id IN (%2)").arg(constants::kTableDnaSubseqs, "%1"));
    deleteSqlQueries_[AdocTreeNode::SubseqRnaType].append(QString("DELETE FROM %1 WHERE id IN (%2)").arg(constants::kTableRnaSubseqs, "%1"));

    deleteSqlQueries_[AdocTreeNode::MsaAminoType].append(QString("DELETE FROM %1 WHERE id IN (SELECT amino_subseq_id FROM %2 WHERE amino_msa_id = %3)").arg(constants::kTableAminoSubseqs, constants::kTableAminoMsaSubseqs, "%1"));
    deleteSqlQueries_[AdocTreeNode::MsaAminoType].append(QString("DELETE FROM %1 WHERE id = %2").arg(constants::kTableAminoMsas, "%1"));

    deleteSqlQueries_[AdocTreeNode::MsaDnaType].append(QString("DELETE FROM %1 WHERE id IN (SELECT dna_subseq_id FROM %2 WHERE dna_msa_id = %3)").arg(constants::kTableDnaSubseqs, constants::kTableDnaMsaSubseqs, "%1"));
    deleteSqlQueries_[AdocTreeNode::MsaDnaType].append(QString("DELETE FROM %1 WHERE id = %2").arg(constants::kTableDnaMsas, "%1"));

    deleteSqlQueries_[AdocTreeNode::MsaRnaType].append(QString("DELETE FROM %1 WHERE id IN (SELECT rna_subseq_id FROM %2 WHERE rna_msa_id = %3)").arg(constants::kTableRnaSubseqs, constants::kTableRnaMsaSubseqs, "%1"));
    deleteSqlQueries_[AdocTreeNode::MsaRnaType].append(QString("DELETE FROM %1 WHERE id = %2").arg(constants::kTableRnaMsas, "%1"));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Returns a list containing a dynamically allocated MpttNode and AdocTreeNode (stored polymorphically
  * as TreeNode *) for each row in tableName. The returned list will be sorted by the mpttNodes left
  * value.
  *
  * A RuntimeError is thrown if there is a row with an unrecognized node type (only in release compile,
  * asserts in debug mode). If this is the case, all heap-allocated memory is freed before throwing an
  * error.
  *
  * Direct throws:
  * o DatabaseError - executing queries
  * o RuntimeError - see note above
  *
  * Subthrows:
  * o InvalidConnectionError - checking table validity
  * o DatabaseError - checking table validity
  *
  * @param tableName [const QString &]
  * @returns QList<MpttNode *>
  *
  * @throws InvalidConnectionError, DatabaseError, RuntimeError
  */
QList<MpttNode *> AdocDbDataSource::readDataTree(const QString &tableName) const
{
    checkTable(tableName);

    // tableName exists, read in all rows; could use DbDataSource::readAll, but that would incur the penalty
    // of another list (which could potentially be very large) and other steps that could be consolidated
    // in a single run through the database results.
    QString select_sql = "SELECT type, fk_id, label, lft, rgt FROM " % tableName % " ORDER BY lft";
    QSqlQuery s_data_tree(database());
    s_data_tree.setForwardOnly(true);
    if (!s_data_tree.exec(select_sql))
        throw DatabaseError(constants::kMessageErrorExecutingQuery, s_data_tree.lastError(), select_sql);

    int rowNumber = 0;
    QList<MpttNode *> mpttNodes;
    while (s_data_tree.next())
    {
        ++rowNumber;

        AdocTreeNode::NodeType nodeType = AdocTreeNode::nodeTypeEnum(s_data_tree.value(0).toString());
        Q_ASSERT_X(nodeType != AdocTreeNode::UndefinedType, "AdocDbDataSource::readDataTree", "undefined Node type not allowed");
        // While the assert will prevent this from occurring in debug code, the following is for gracefully handling this error
        // in release code.
        if (nodeType == AdocTreeNode::UndefinedType)
        {
            foreach (MpttNode *mpttNode, mpttNodes)
            {
                delete mpttNode->treeNode_;
                mpttNode->treeNode_ = 0;
            }
            qDeleteAll(mpttNodes);
            mpttNodes.clear();

            throw RuntimeError(QString("Table %1, row %2 contains an unrecognized node type").arg(tableName, rowNumber));
        }

        AdocTreeNode *node = new AdocTreeNode(nodeType, s_data_tree.value(2).toString(), s_data_tree.value(1).toInt());
        mpttNodes.append(new MpttNode(node, s_data_tree.value(3).toInt(), s_data_tree.value(4).toInt()));
    }

    return mpttNodes;
}

/**
  *
  *
  * @param id [int]
  * @param alphabet [Alphabet]
  * @returns RichMsa *
  */
RichMsa *AdocDbDataSource::readMsa(int id, Alphabet alphabet) const
{
    switch (alphabet)
    {
    case eAminoAlphabet:
        return readAminoMsa(id);
    case eDnaAlphabet:
        return readDnaMsa(id);
    case eRnaAlphabet:
        return readRnaMsa(id);

    default:
        return 0;
    }
}

/**
  * Only one tree may be stored in tableName and thus before this operation proceeds all data is removed. Thus, it is vital
  * that all relevant data has been retrieved from the table before calling this method or that it is wrapped in a database
  * transaction in the event of an error.
  *
  * Will throw a DatabaseError if tableName does not have an appropriate data structure regardless of the contents in mpttNodeList.
  *
  * @param root [AdocTreeNode *]
  * @param tableName [const QString &]
  *
  * @throws InvalidConnectionError, DatabaseError
  */
void AdocDbDataSource::saveDataTree(const QList<MpttNode *> &mpttNodeList, const QString &tableName) const
{
    // The truncate method also ensures that tableName is valid and will throw the appropriate
    // exception if a) there is no valid database connection or b) the table does not exist
    DbDataSource::truncate(tableName);

    // We could shortcut and check if mpttNodeList is empty and simply return at that point; however,
    // this may provide a false sense that everything is working correctly if mpttNodeList is empty.
    // By removing this and actually preparing the SQL, we will throw a DatabaseError if the table
    // has an invalid structure - regardless of the contents of mpttNodeList. This is what we want.

    QString insert_sql = "insert into " % tableName % " (type, fk_id, label, lft, rgt) values (?, ?, ?, ?, ?)";
    QSqlQuery i_data_tree(database());
    if (!i_data_tree.prepare(insert_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, i_data_tree.lastError(), insert_sql);

    foreach (MpttNode *mpttNode, mpttNodeList)
    {
        Q_ASSERT_X(mpttNode->treeNode_, "Adoc::save", "mpttNode::treeNode_ pointer must not be null");

        AdocTreeNode *atn = static_cast<AdocTreeNode *>(mpttNode->treeNode_);

        i_data_tree.bindValue(0, AdocTreeNode::nodeTypeString(static_cast<AdocTreeNode::NodeType>(atn->nodeType_)));
        i_data_tree.bindValue(1, (atn->fkId_ > 0) ? atn->fkId_ : QVariant());
        i_data_tree.bindValue(2, atn->label_);
        i_data_tree.bindValue(3, mpttNode->left_);
        i_data_tree.bindValue(4, mpttNode->right_);
        if (!i_data_tree.exec())
            throw DatabaseError(constants::kMessageErrorExecutingQuery, i_data_tree.lastError(), insert_sql);
    }
}

/**
  *
  *
  * @param richMsa [const RichMsa &]
  * @returns bool
  */
bool AdocDbDataSource::saveMsa(const RichMsa &richMsa) const
{
    return false;
}

/**
  * Sends SQL deletes to the database for all nodeTypeRecords. The particular nodeType determines which table
  * the record is deleted from.
  *
  * Currently only deletes subseq-type nodes.
  *
  * If all of the nodes cannot be successfully removed, none of them are removed.
  *
  * @param nodeTypeRecords [QHash<AdocTreeNode::NodeType, QStringList>]
  */
void AdocDbDataSource::eraseRecords(QHash<AdocTreeNode::NodeType, QStringList> nodeTypeRecords) const
{
    if (nodeTypeRecords.isEmpty())
        return;

    // For sanity purposes
    checkTable(constants::kTableAminoSubseqs);
    checkTable(constants::kTableDnaSubseqs);
    checkTable(constants::kTableRnaSubseqs);

    QString savePointName = "eraseRecords";
    savePoint(savePointName);

    QHash<AdocTreeNode::NodeType, QStringList>::const_iterator i = nodeTypeRecords.constBegin();
    QSqlQuery d_query(database());
    while (i != nodeTypeRecords.constEnd())
    {
        switch (i.key())
        {
        case AdocTreeNode::SubseqAminoType:
        case AdocTreeNode::SubseqDnaType:
        case AdocTreeNode::SubseqRnaType:
        case AdocTreeNode::MsaAminoType:
        case AdocTreeNode::MsaDnaType:
        case AdocTreeNode::MsaRnaType:
            foreach (const QString &query, deleteSqlQueries_[i.key()])
            {
                if (!d_query.prepare(query.arg(i.value().join(","))))
                {
                    rollbackToSavePoint(savePointName);
                    throw DatabaseError(constants::kMessageErrorPreparingQuery, d_query.lastError(), query.arg(i.value().join(",")));
                }
                if (!d_query.exec())
                {
                    rollbackToSavePoint(savePointName);
                    throw DatabaseError(constants::kMessageErrorExecutingQuery, d_query.lastError(), query.arg(i.value().join(",")));
                }
            }
            break;

        default:
            break;
        }

        ++i;
    }

    releaseSavePoint(savePointName);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param id [int]
  * @returns RichMsa *
  */
RichMsa *AdocDbDataSource::readAminoMsa(int id) const
{
    // Check that there is an alignment with this id
    static QString s_msa_sql = QString("SELECT name FROM %1 WHERE id = ?").arg(constants::kTableAminoMsas);
    static QString s_msa_subseqs = QString("SELECT FROM %1 a JOIN b ON (a.amino_subseq_id = b.id) JOIN c ON (b.amino_seq_id = c.id) WHERE a.amino_msa_id = ?").arg(constants::kTableAminoMsaSubseqs, constants::kTableAminoSubseqs, constants::kTableAminoSeqs);

    QSqlQuery s_msa(database());

    if (!s_msa.prepare(s_msa_sql))
        throw DatabaseError(constants::kMessageErrorPreparingQuery, s_msa.lastError(), s_msa_sql);

    s_msa.bindValue(0, id);
    if (!s_msa.exec())
        throw DatabaseError(constants::kMessageErrorExecutingQuery, s_msa.lastError(), s_msa_sql);

    // Msa not found in the database
    if (!s_msa.next())
        return 0;

    // The msa exists in the database, create a new RichMsa with its intial annotation
    RichMsa *richMsa = new RichMsa(eAminoAlphabet, id);
//    richMsa->annotation_ = DataRow(QVariant(id, QVariant::Int));
    //richMsa->annotation_.setValue("name", s_msa.value(0));

    // Now append all RichSubseqs
}

RichMsa *AdocDbDataSource::readDnaMsa(int id) const
{
    return 0;
}

RichMsa *AdocDbDataSource::readRnaMsa(int id) const
{
    return 0;
}
