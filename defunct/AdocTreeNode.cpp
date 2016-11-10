/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeNode.h"

// #include "AdocDbDataSource.h"

// #include <QtSql/QSqlError>
// #include <QtSql/QSqlQuery>

// ------------------------------------------------------------------------------------------------
// Initialize the private static members
// Because they are defined in the cpp file, they are inaccessible to the end-user and this is the
// intended effect because they should only be called once and that during startup.
/**
  * Private function intended to build list of equivalent QString identifiers for each possible NodeType.
  * It is critical that these are arranged in the same order as the corresonding enums are listed in the
  * header file, because nodeTypeString(NodeType) treats NodeType as an integer to reference the appropriate
  * index.
  *
  * @returns QList<QString>
  * @see nodeTypeString()
  */
QList<QString> __initNodeTypeStrings()
{
    QList<QString> temp;

    temp.append("undefined");       // AdocTreeNode::UndefinedType
    temp.append("root");            // AdocTreeNode::RootType
    temp.append("group");           // AdocTreeNode::GroupType
    temp.append("seqamino");        // AdocTreeNode::SeqAminoType
    temp.append("seqdna");          // AdocTreeNode::SeqDnaType
    temp.append("seqrna");          // AdocTreeNode::SeqRnaType
    temp.append("subseqamino");     // AdocTreeNode::SubseqAminoType
    temp.append("subseqdna");       // AdocTreeNode::SubseqDnaType
    temp.append("subseqrna");       // AdocTreeNode::SubseqRnaType
    temp.append("msaamino");        // AdocTreeNode::MsaAminoType
    temp.append("msadna");          // AdocTreeNode::MsaDnaType
    temp.append("msarna");          // AdocTreeNode::MsaRnaType
    temp.append("primer");          // AdocTreeNode::PrimerType

    return temp;
}
QList<QString> AdocTreeNode::nodeTypeStrings_(__initNodeTypeStrings());



/**
  * Simply initializes a static list for mapping a QString to its equivalent NodeType.
  *
  * @returns QHash<QString, AdocTreeNode::NodeType
  * @see nodeTypeEnum()
  */
QHash<QString, AdocTreeNode::NodeType> initNodeTypeEnums()
{
    QHash<QString, AdocTreeNode::NodeType> temp;

    temp["undefined"] = AdocTreeNode::UndefinedType;
    temp["root"] = AdocTreeNode::RootType;
    temp["group"] = AdocTreeNode::GroupType;
    temp["seqamino"] = AdocTreeNode::SeqAminoType;
    temp["seqdna"] = AdocTreeNode::SeqDnaType;
    temp["seqrna"] = AdocTreeNode::SeqRnaType;
    temp["subseqamino"] = AdocTreeNode::SubseqAminoType;
    temp["subseqdna"] = AdocTreeNode::SubseqDnaType;
    temp["subseqrna"] = AdocTreeNode::SubseqRnaType;
    temp["msaamino"] = AdocTreeNode::MsaAminoType;
    temp["msadna"] = AdocTreeNode::MsaDnaType;
    temp["msarna"] = AdocTreeNode::MsaRnaType;
    temp["primer"] = AdocTreeNode::PrimerType;

    return temp;
}
QHash<QString, AdocTreeNode::NodeType> AdocTreeNode::nodeTypeEnums_(initNodeTypeEnums());


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @param start [int]
  * @param end [int]
  * @returns QList<TreeNode *>
  */
QList<AdocTreeNode *> AdocTreeNode::childrenBetween(int start, int end) const
{
    QList<AdocTreeNode *> childList;
    foreach (TreeNode *node, TreeNode::childrenBetween(start, end))
        childList.append(static_cast<AdocTreeNode *>(node));

    return childList;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Cascading erase of this and all descendant data tree nodes. This can have large-scale effects especially
  * if performed on a high-level node that contains numerous and/or varied descendants. Note that the fkId
  * will point to a non-existent record if this operation was successful.
  *
  * If the fkId_ points to a non-existent record when this function is called, it will still return true if
  * there was no other error. The logic is that the goal of this function is to erase a record and the end
  * result is the same if the fkId_ originally does not point to a record.
  *
  * The return result denotes whether the operation succeeded completely. If a failure occurs at any point
  * then the function returns false and the database is restored to its previous point. This is accomplished
  * by setting a savepoint before the cascading erase operation and then rolling back if false is received
  * at any point. The actual operation is handled via the private method, eraseRecordPrivate().
  *
  * Specialized classes may declare additional actions to be taken when eraseRecord is called although it
  * is advised to call this AdocTreeNode::eraseRecord() at some point from the derived class method.
  *
  * Conditions which return false:
  * o invalid database or unopened database
  * o foreignTable_ is not empty and fkId_ is 0
  * o fkId_ is > 0 and foreignTable_ is empty
  * o fkId_ is < 0 (assert's in debug mode)
  * o foreignTable_ is defined but does not exist in the database
  * o an error occurred while deleting the record
  *
  * @param database [const QSqlDatabase &]
  * @returns bool
  * @see eraseRecordPrivate()
  */
/*
bool AdocTreeNode::eraseRecord(const AdocDbDataSource &dataSource)
{
    if (!dataSource.databaseReady())
        return false;

    // Create savepoint
    if (dataSource.database().exec("SAVEPOINT AdocTreeNode_eraseRecord").lastError().type() != QSqlError::NoError)
        return false;

    // Run operation via eraseRecordPrivate
    bool retval = eraseRecordPrivate(dataSource);

    if (retval)
    {
        if (dataSource.database().exec("RELEASE SAVEPOINT AdocTreeNode_eraseRecord").lastError().type() != QSqlError::NoError)
            retval = false;
    }
    else
        dataSource.database().exec("ROLLBACK TO SAVEPOINT AdocTreeNode_eraseRecord");

    return retval;
}
*/
/**
  * Private function that encapsulates the actual database record deletion process as well as recursively
  * performing this operation on all its children.
  *
  * Follows same rules as eraseRecord() for dealing with fkId's and foreignTable_ values
  *
  * @param database [const QSqlDatabase &]
  * @returns bool
  * @see eraseRecord()
  */
/*
bool AdocTreeNode::eraseRecordPrivate(const AdocDbDataSource &dataSource)
{
    // Avoid all negative values for fkId_
    Q_ASSERT_X(fkId_ >= 0, "AdocTreeNode::eraseRecordPrivate", "negative values not allowed for fkId_");
    if (fkId_ < 0)
        return false;

    // Erase all children nodes first
    foreach (TreeNode *child, children_)
        if (!static_cast<AdocTreeNode *>(child)->eraseRecordPrivate(dataSource))
            return false;

    // If we have both a positive fkId_ and non-empty foreignTable_, attempt the delete
    if (fkId_ > 0 && foreignTable_.isEmpty() == false)
    {
        dataSource.erase(foreignTable_, fkId_);
        return true;

//        CrudSqlRecord record(DbTable(foreignTable(), database));

        // QSqlRecord::count() returns the number of fields for a given table. If the table is present in the database, it will
        // have at least one field and count() will evaluate to true.
//        if (record.count())
//        {
//            record.id_ = fkId_;
//            int eraseCode = record.erase();
//            if (eraseCode >= 0)
//                return true;
//        }

        // Two possible reasons for failure:
        // 1) Unable to find table in database
        // 2) Delete command failed (more information may be obtained via examining the specific eraseCode)
        return false;
    }
    else if ((fkId_ == 0 && foreignTable_.isEmpty() == false) ||
             (fkId_ > 0 && foreignTable_.isEmpty()))
    {
        return false;
    }

    // If we get here, this means, that fkId_ == 0 and foreignTable_ is empty. Thus, it is an entry
    // without any corresponding database record
    return true;
}
*/
