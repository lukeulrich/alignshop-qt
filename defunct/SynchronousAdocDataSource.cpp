/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SynchronousAdocDataSource.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QScopedPointer>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QStringBuilder>
#include <QtCore/QTime>

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "AdocTreeNode.h"
#include "DbAnonSeqFactory.h"
#include "MpttTreeConverter.h"
#include "MpttNode.h"
#include "Msa.h"
#include "Subseq.h"
#include "SqlBuilder.h"

#include "exceptions/InvalidMpttNodeError.h"
#include "global.h"

#include <QtDebug>
//#include <QThread>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
SynchronousAdocDataSource::SynchronousAdocDataSource(QObject *parent) : AbstractAdocDataSource(parent)
{
}

/**
  * Close down the data source and free any allocated memory associated with the anonSeqFactories.
  */
SynchronousAdocDataSource::~SynchronousAdocDataSource()
{
    close();

    qDeleteAll(anonSeqFactories_);
    anonSeqFactories_.clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Ownership remains with this class, so it is vital that external classes do not attempt to delete
  * these pointers or store them for long-term use unless they can guarantee safe usage.
  *
  * @returns QHash<Alphabet, AnonSeqFactory *>
  */
QHash<Alphabet, DbAnonSeqFactory *> SynchronousAdocDataSource::anonSeqFactories() const
{
    return anonSeqFactories_;
}

/**
  * @returns QSqlDatabase
  */
const QSqlDatabase SynchronousAdocDataSource::getDatabase() const
{
    return database_;
}

/**
  * Simply returns whether the database connection is open.
  *
  * @returns bool
  */
bool SynchronousAdocDataSource::isOpen() const
{
    return database_.isOpen();
}

/**
  * @returns QString
  */
QString SynchronousAdocDataSource::sourceFile() const
{
    return sourceFile_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * If an active connection is already established, this method does nothing and emits openError().
  *
  * @param file [const QString &]
  */
void SynchronousAdocDataSource::open(const QString &file)
{
//    qDebug() << QString("[%1] Opening file").arg(QThread::currentThreadId()) << file;

    if (isOpen())
    {
        emit openError(file, QString("Unable to open file '%1' because another document is already open").arg(file));
        return;
    }

    if (file.isEmpty())
    {
        emit openError(file, "Unable to open file '[NULL]': please provide a valid filename");
        return;
    }

    if (!QFile::exists(file))
    {
        emit openError(file, QString("Unable to open file '%1': file does not exist").arg(file));
        return;
    }

    QFile fileObj(file);
    if (fileObj.size() == 0)
    {
        emit openError(file, QString("Unable to open file '%1': empty file").arg(file));
        return;
    }

    // State:
    // o Not already open
    // o Have a valid file argument
    // o File exists and has a non-zero size

    // Attempt to open database with connection name {filename}-{X} where X = 1-10. If connection name is
    // already in use, increment X and try again. If after 10 tries, no connection could be established
    // emit openError.
    QString connectionName = unusedConnectionNameFromFileName(file, 10);
    if (connectionName.isEmpty())
    {
        emit openError(file, QString("Unable to open file '%1': exceeded maximum number of database connections for this name").arg(file));
        return;
    }

    // Attempt to open the database
    database_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    database_.setDatabaseName(file);
    if (!database_.open())
    {
        // Reset private QSqlDatabase member to invalid state without connection name so that
        // connectionName may be removed without throwing an error
        database_ = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);

        emit openError(file, QString("Unable to open file '%1': %2").arg(file, database_.lastError().text()));
        return;
    }

    // Successfully opening the database file does not indicate that it is indeed a valid database file.
    // Adoc table should have at least one table
    if (database_.tables().isEmpty())
    {
        // Reset private QSqlDatabase member to invalid state without connection name so that
        // connectionName may be removed without throwing an error
        database_ = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);

        emit openError(file, QString("File '%1' does not contain any tables. Please check that it is a valid database").arg(file));
        return;
    }

    sourceFile_ = file;
    emit opened();
}

/**
  * Does nothing if a database connection has not previously been opened.
  */
void SynchronousAdocDataSource::close()
{
    if (!isOpen())
        return;

    database_.close();
    sourceFile_ = QString();

    // Save the connection name so that we can remove it after resetting
    // the database_ member variable
    QString connectionName = database_.connectionName();

    // Reset the database to a null state
    database_ = QSqlDatabase();

    // Remove the database connection name
    QSqlDatabase::removeDatabase(connectionName);

    emit closed();
}

/**
  * Create utilizes a completely independent and local QSqlDatabase instance to create the new
  * SQLite database file and then runs each query contained in ddlQueries on this new instance.
  * In the event of an error, the file is removed and createError is emitted. The following
  * conditions will trigger an error:
  * o Another file with the requested file name already exists
  * o Unable to open database
  * o Any error executing any of the ddlQueries
  *
  * If the database was successfully created and all queries were performed successfully, the
  * database is closed and createDone() is emitted.
  *
  * This method will work regardless of the classes open/closed state.
  *
  * @param file [const QString &]
  * @param ddlQueries [const QStringList &]
  */
void SynchronousAdocDataSource::create(const QString &file, const QStringList &ddlQueries)
{
    if (file.isEmpty())
    {
        emit createError(file, "Invalid file name");
        return;
    }

    if (QFile::exists(file))
    {
        emit createError(file, "Another file with the same name already exists on the file system");
        return;
    }

    // It is not desirable to pollute the QSqlDatabase connection name space if possible; regardless
    // of the outcome of this method, care is taken to remove the database connection name before
    // emitting any signals or returning. To achieve this goal, the local variable, error, is used to
    // capture any error message and also serve as indicator of any error that has occurred. At the
    // end of this method after releasing the database name, it is then possible to emit the appropriate
    // signals
    QString error;
    QString connectionName = unusedConnectionNameFromFileName(file);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(file);
    if (!db.open())
    {
        error = QString("Database error: %1").arg(db.lastError().text());
    }
    else
    {
        // Successfully opened the database
        // Call each of the queries
        foreach (const QString &ddlQuery, ddlQueries)
        {
            QSqlQuery query = db.exec(ddlQuery);
            if (!query.isActive())
            {
                // There was an error executing this query
                error = QString("Database query error: %1").arg(db.lastError().text());
                break;
            }
        }
    }

    // Free up the database connection name
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);

    // Check for any error
    if (error.isEmpty())
    {
        emit createDone(file);
        return;
    }

    // We have an error
    QFile::remove(file);
    emit createError(file, error);
}

/**
  * Read the arbitrarily deep data tree from the database table, constants::kTableDataTree, and transform its MPTT
  * representation into a tree comprised of AdocTreeNodes.
  *
  * Signals emitted:
  * o dataTreeReady(AdocTreeNode *) [success]
  * o dataTreeError(QString)   [failure]
  *
  * If the data tree table (constants::kTableDataTree) does not contain any rows, returns a single root node
  * to the dataTreeReady signal
  */
void SynchronousAdocDataSource::readDataTree()
{
//    qDebug() << QString("[%1] %2").arg(QThread::currentThreadId()).arg(__FUNCTION__);

    // ------------------------------------------------------------------------
    // Make sure we have a valid connection
    if (!isOpen())
    {
        emit dataTreeError("Valid data source has not yet been opened");
        return;
    }

    // ------------------------------------------------------------------------
    // Prepare and execute the database queries
    QString select_sql = "SELECT type, fk_id, label, lft, rgt FROM " % QString(constants::kTableDataTree) % " ORDER BY lft";
    QSqlQuery s_data_tree(database_);
    s_data_tree.setForwardOnly(true);
    if (!s_data_tree.prepare(select_sql))
    {
        emit dataTreeError(QString("Database error: %1").arg(s_data_tree.lastError().databaseText()));
        return;
    }

    if (!s_data_tree.exec())
    {
        emit dataTreeError(QString("Database error: %1").arg(s_data_tree.lastError().databaseText()));
        return;
    }

    // ------------------------------------------------------------------------
    // Fetch the data and store in a QList<MpttNode *>
    int rowNumber = 0;
    QList<MpttNode *> dataTreeMpttNodes;
    while (s_data_tree.next())
    {
        ++rowNumber;

        AdocTreeNode::NodeType nodeType = AdocTreeNode::nodeTypeEnum(s_data_tree.value(0).toString());
        ASSERT_X(nodeType != AdocTreeNode::UndefinedType, "undefined node type not allowed");
        if (nodeType == AdocTreeNode::UndefinedType)    // Release mode guard
        {
            freeMpttList(dataTreeMpttNodes);

            emit dataTreeError(QString("Database error: table %1, row %2 contains an unrecognized node type").arg(constants::kTableDataTree, rowNumber));
            return;
        }

        AdocTreeNode *node = new AdocTreeNode(nodeType, s_data_tree.value(2).toString(), s_data_tree.value(1).toInt());
        dataTreeMpttNodes.append(new MpttNode(node, s_data_tree.value(3).toInt(), s_data_tree.value(4).toInt()));
    }

    // ------------------------------------------------------------------------
    // If there are no mpttNodes, then there were no rows in the table, simply create a root AdocTreeNode and return
    if (dataTreeMpttNodes.isEmpty())
    {
        AdocTreeNode *root = new AdocTreeNode(AdocTreeNode::RootType, "Root", 0);
        emit dataTreeReady(root);
        return;
    }

    // ------------------------------------------------------------------------
    // Otherwise, attempt to convert the mptt node list to an actual tree structure
    // Must catch any InvalidMpttNodeError to properly de-allocate dataTreeRows
    AdocTreeNode *root = 0;
    try
    {
        root = static_cast<AdocTreeNode *>(MpttTreeConverter::fromMpttList(dataTreeMpttNodes));

        // Free up the memory used by the QList<MpttNode *>, but not the TreeNode pointers associated
        // with each of these nodes (they comprise are newly constructed tree)
        qDeleteAll(dataTreeMpttNodes);
        dataTreeMpttNodes.clear();      // Safeguard against dangling pointers

        // Send out the success
        emit dataTreeReady(root);
        return;
    }
    catch (InvalidMpttNodeError &e)
    {
        // Get the label of the node that failed
        QString nodeLabel = static_cast<AdocTreeNode *>(e.mpttNode()->treeNode_)->label_;
        if (nodeLabel.isEmpty())
            nodeLabel = "NULL";
        freeMpttList(dataTreeMpttNodes);
        emit dataTreeError(QString("Invalid MPTT tree data (Error %1): %2 || Node: %3").arg(QString::number(e.errorNumber()), e.what(), nodeLabel));
        return;
    }
    catch (...)
    {
        freeMpttList(dataTreeMpttNodes);
        emit dataTreeError("An unexpected error occurred while reconstructing the tree");
        return;
    }
}

/**
  * For this method to succeed, the following requirements must be met:
  * o Appropriate database tables exist respective to alphabet
  * o MSA exists in the database with the id field
  * o Appropriate AnonSeqFactory has been configured for alphabet
  *
  * Signals emitted
  * o msaReady(Msa *, int tag)
  * o msaError(QString, int tag)
  *
  * Note: A memory leak will occur if a Msa is successfully created for the given input, yet no
  *       receiver has been connected to this signal to take ownership of the pointer returned
  *       via msaReady.
  *
  * @param id [int]
  * @param alphabet [Alphabet]
  * @param tag [int]
  */
void SynchronousAdocDataSource::readMsa(int id, Alphabet alphabet, int tag)
{
    // ------------------------------------------------------------------------
    // Sanity check
    if (!isOpen())
    {
        emit msaError("Valid data source has not yet been opened", tag);
        return;
    }

    if (!anonSeqFactories_.contains(alphabet))
    {
        emit msaError("No anonymous sequence factory configured for this alphabet", tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Non-polymorphic SQL construction :)
    // Regardless of the alphabet, three fields are required:
    // 1. List of subseq ids
    // 2. Subsequence sequences
    // 3. Anon sequence identifiers (e.g. astring_id, dstring_id, or rstring_id)
    //
    // Depending on the alphabet, this information will be fetched from different tables
    QString msaTable;
    QString selectSubseqData;
    switch (alphabet)
    {
    case eAminoAlphabet:
        msaTable = constants::kTableAminoMsas;
        selectSubseqData = "SELECT b.id AS subseq_id, b.sequence, c.astring_id, b.start, b.stop FROM " % QString(constants::kTableAminoMsaSubseqs) % " a JOIN "
                % QString(constants::kTableAminoSubseqs) % " b ON (a.amino_subseq_id = b.id) JOIN "
                % QString(constants::kTableAminoSeqs) % " c ON (b.amino_seq_id = c.id) WHERE a.amino_msa_id = ? ORDER BY a.position";
        break;
    case eDnaAlphabet:
        msaTable = constants::kTableDnaMsas;
        selectSubseqData = "SELECT b.id AS subseq_id, b.sequence, c.dstring_id, b.start, b.stop FROM " % QString(constants::kTableDnaMsaSubseqs) % " a JOIN "
                % QString(constants::kTableDnaSubseqs) % " b ON (a.dna_subseq_id = b.id) JOIN "
                % QString(constants::kTableDnaSeqs) % " c ON (b.dna_seq_id = c.id) WHERE a.dna_msa_id = ? ORDER BY a.position";
        break;
    case eRnaAlphabet:
        msaTable = constants::kTableRnaMsas;
        selectSubseqData = "SELECT b.id AS subseq_id, b.sequence, c.rstring_id, b.start, b.stop FROM " % QString(constants::kTableRnaMsaSubseqs) % " a JOIN "
                % QString(constants::kTableRnaSubseqs) % " b ON (a.rna_subseq_id = b.id) JOIN "
                % QString(constants::kTableRnaSeqs) % " c ON (b.rna_seq_id = c.id) WHERE a.rna_msa_id = ? ORDER BY a.position";
        break;

    default:
        emit msaError("Support for the requested alphabet type does not exist", tag);
        return;
    }

    QString msaDataSql = "SELECT name, description FROM " % msaTable % " WHERE id = ?";
    QSqlQuery msaDataQuery(database_);
    if (!msaDataQuery.prepare(msaDataSql))
    {
        emit msaError(QString("Database error: %1").arg(msaDataQuery.lastError().databaseText()), tag);
        return;
    }

    msaDataQuery.addBindValue(id);
    if (!msaDataQuery.exec())
    {
        emit msaError(QString("Database error: %1").arg(msaDataQuery.lastError().databaseText()), tag);
        return;
    }

    if (!msaDataQuery.next())
    {
        emit msaError(QString("No msa found in the table, %1, with the id: %2").arg(msaTable).arg(id), tag);
        return;
    }

    // We found a record with this id - wait until we have successfully queried for the subseq data before
    // constructing a Msa object

    QSqlQuery subseqDataQuery(database_);
    subseqDataQuery.setForwardOnly(true);
    if (!subseqDataQuery.prepare(selectSubseqData))
    {
        qDebug() << "Database prep error:" <<  subseqDataQuery.lastError().text();

        emit msaError(QString("Database error: %1").arg(subseqDataQuery.lastError().databaseText()), tag);
        return;
    }

    subseqDataQuery.addBindValue(id);
    if (!subseqDataQuery.exec())
    {
        qDebug() << "Database exec error:" <<  subseqDataQuery.lastError().text();

        emit msaError(QString("Database error: %1").arg(subseqDataQuery.lastError().databaseText()), tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Build the Msa object with all its data
    //
    // Use a scoped pointer in case of any error, it will automatically be freed; however, if everything
    // should succeed, then we extract the pointer from the scoped pointer before emitting our completed
    // signal. Do the same for newly created subseqs
    QScopedPointer<Msa> pMsa(new Msa(alphabet, id));
    pMsa->setName(msaDataQuery.value(0).toString());
    pMsa->setDescription(msaDataQuery.value(1).toString());
    msaDataQuery.finish();

    AnonSeqFactory *anonSeqFactory = anonSeqFactories().value(alphabet);
    ASSERT(anonSeqFactory);
    while (subseqDataQuery.next())
    {
        int anonSeqId = subseqDataQuery.value(2).toInt();
        AnonSeq anonSeq = anonSeqFactory->fetch(anonSeqId);
        ASSERT(anonSeq.isValid());
        if (!anonSeq.isValid()) // Release mode guard
        {
            emit msaError(QString("AnonSeq data missing from database for id: %1").arg(anonSeqId), tag);
            return;
        }

        int subseqId = subseqDataQuery.value(0).toInt();
        QScopedPointer<Subseq> pSubseq(new Subseq(anonSeq, subseqId));

        QString subseqSequence = subseqDataQuery.value(1).toString();
        if (!pSubseq->setBioString(subseqSequence)
            || pSubseq->start() != subseqDataQuery.value(3).toInt()
            || pSubseq->stop() != subseqDataQuery.value(4).toInt())
        {
            emit msaError(QString("Subsequence not found in source AnonSeq or out-of-sync start/stop values (subseq id: %1, anon seq id: ").arg(subseqId, anonSeqId), tag);
            return;
        }

        // Append the subseq
        Subseq *subseq = pSubseq.take();
        if (!pMsa->append(subseq))
        {
            // Release the subseq
            delete subseq;
            subseq = 0;

            emit msaError(QString("Unable to append subseq to msa (subseq id: %1)").arg(subseqId), tag);
            return;
        }
    }

    // Finished reading in the data, emit the success signal and leave. Note we "take" the raw pointer
    // controlled by the scoped pointer so that the memory pointer to by pMsa won't be deleted when
    // the scoped pointer goes out of scope.
    emit msaReady(pMsa.take(), tag);
}

/**
  * If an anonSeqFactory is already associated with alphabet, it will be replaced with this anonSeqFactory.
  * If anonSeqFactory is zero, then that alphabet will be removed from the internally managed list. If anonSeqFactory
  * is non-zero, assign its data source to this instance.
  *
  * Logical use error: setting the same anonSeqFactory for multiple alphabets - this would lead to a segfault
  *                    when the memory was attempted to be freed. If this occurs, will assert in debug code
  *                    and do nothing in release mode.
  *
  * @param alphabet [Alphabet]
  * @param anonSeqFactory [DbAnonSeqFactory *]
  */
void SynchronousAdocDataSource::setAnonSeqFactory(Alphabet alphabet, DbAnonSeqFactory *anonSeqFactory)
{
    if (!anonSeqFactory)
    {
        if (!anonSeqFactories_.contains(alphabet))
            return;

        delete anonSeqFactories_.take(alphabet);
        return;
    }

    ASSERT_X(anonSeqFactories_.values().contains(anonSeqFactory) == false, "Cannot add the same anonSeqFactory pointer multiple times");
    if (anonSeqFactories_.values().contains(anonSeqFactory)) // Release mode guard
        return;

    if (anonSeqFactories_.contains(alphabet))
        delete anonSeqFactories_.take(alphabet);

    anonSeqFactory->setAdocDataSource(this);
    anonSeqFactories_[alphabet] = anonSeqFactory;
}

/**
  * Closes the open SQLite connection, copies sourceFile() to file and then reopens file.
  *
  * The saveAs method takes the following steps:
  * o Copy sourceFile() to a temporary file
  * o commit any open transaction
  * o stealth close current database (stealth to avoid sending out signals)
  * o rename current document to file
  * o move temporary_file to old file (this presents potential issues with currently opened external connections)
  * o stealth open the newly saved database file
  *
  * The major issue is dealing with any changes that are contained within the current transaction. It is
  * essential that these are captured into the new file to be saved, but not committed to the old (current)
  * open file. This works mostly as expected; however, on Linux, any previously opened, external connections
  * will operate on the new database file and not the original file as might be expected.
  *
  * Another caveat here: if we use existing methods in this process (e.g. close() or open()), these will emit
  * unexpected signals that will most likely produce unexpected behavior (depending on the calling application)
  * Therefore, to avoid these side-effects, we simply close/open the relevant database connections inside this
  * method. This should work because the in-memory structure will be perfectly in sync with the latest database
  * snapshot which we are simply copying.
  *
  * On success, emits the saveAsDone signal; otherwise, the saveAsError signal is emitted when one of the
  * following conditions occurs:
  * o file variable is empty
  * o database is not open
  * o a file already exists with the same name as fileName
  * o the file cannot be created (e.g. permissions issue)
  *
  * @param file [const QString &]
  */
void SynchronousAdocDataSource::saveAs(const QString &file)
{
    if (!isOpen())
    {
        emit saveAsError(file, QString("Unable to save document that has not yet been opened"));
        return;
    }

    if (file.isEmpty())
    {
        emit saveAsError(file, "Unable to save to file '[NULL]': please provide a valid filename");
        return;
    }

    if (QFile::exists(file))
    {
        emit saveAsError(file, "Another file with the same name already exists on the file system");
        return;
    }

    // Sanity check: does user have ability to create this file
    // A quick file creation test is a roundabout way of validating that file is a valid usable filename
    // and user has write permission to the target directory without resorting to all sorts of cross-platform
    // compatible checks
    {
        QFile testFile(file);
        if (!testFile.open(QIODevice::WriteOnly))
        {
            emit saveAsError(file, QString("Unable to create file '%1': %2").arg(file, testFile.errorString()));
            return;
        }

        if (!testFile.remove())
        {
            emit saveAsError(file, QString("An unexpected error occurred while checking ability to write file"));
            return;
        }
    }

    // Create temporary file to hold old version (without any transactions) of the database
    QString tempFileName = temporaryFileName();

    // Copy database with uncommitted changes to the temporary file. Must read from the source file and write
    // to the temp file using QIODevice (http://www.qtcentre.org/archive/index.php/t-20431.html). Originally,
    // I had tried removing the temp_file and then using QFile::Copy; however, on 19 Oct 2010, the QFile::Copy
    // command always returned false. I presume it is due to the issue noted in the above link.
    //
    // Changed mind again. Performing a raw copy of sourceFile() -> tempFileName is more work than I care to
    // deal with right now and it is very unlikely to present any serious problems. Will revisit in future
    // if it is a problem.
    //
    // Thus, simply creating an "unsafe" unique temp file name with the temporaryFileName here.
    // TODO: safely create and utilize temporary file
    if (!QFile::copy(sourceFile(), tempFileName))
    {
        emit saveAsError(file, QString("Error copying current document to temporary file (%1)").arg(tempFileName));
        return;
    }

    // Having copied the database file to a temporary location, now commit any open transaction
    database_.commit();

    // Stealthily close the current database file. The goal is to transparently update the database connection to a new file
    // without triggering a visual update.
    // A. Close the current database connection
    database_.close();

    // Move the current database to its final destination
    if (!QFile::rename(sourceFile(), file))
    {
        // Move failed, move the old copy back and reopen; of course if this fails, we're screwed.
        // Probably should have some kind of fatalSignal that we could send...
        QFile::rename(tempFileName, sourceFile());
        database_.open();

        emit saveAsError(file, QString("Unable to copy document to %1").arg(file));
        return;
    }

    // Update the file and database name to reflect its new status
    QString old_file = sourceFile();
    sourceFile_ = file;

    // And move the previous version to the original file
    QFile tempFile(tempFileName);
    if (!tempFile.rename(old_file))
    {
        // This time, cannot move temporary file back to our original location
        // Attempt to open the database connection to the new file, so that user has workable setup
        database_.setDatabaseName(sourceFile());
        if (!database_.open())
        {
            emit saveAsError(file, QString("File successfully saved as '%1', but unable to open. Please try again.").arg(file));
            close();
            return;
        }

        emit saveAsError(file, QString("Unable to rename (move) temporary file %1 to %2: %3").arg(tempFileName, old_file, tempFile.errorString()));
        return;
    }

    // Finally re-open the saved version of the database
    database_.setDatabaseName(sourceFile());
    if (!database_.open())
    {
        emit saveAsError(file, QString("File successfully saved as '%1', but unable to open. Please try again.").arg(file));
        close();
        return;
    }

    // Success!
    emit saveAsDone(file);
}

/**
  * This method provides a high level interface for performing the same select query multiple
  * times with one varying parameter. Accordingly, sql must contain a valid SQL statement for
  * the associated database connection and contain a single placeholder marked with a '?'. For
  * each value of input, the query will be performed and the results appended to a QList.
  *
  * To provide a generic mechanism for clients to receive data from this single interface, the
  * caller must supply a QObject receiver and a method to call on success (method) and a method
  * to call on error (errorMethod). These methods are called via the QMetaObject::invokeMethod
  * function.
  *
  * An optional tag may be supplied with each request for identifying each request.
  *
  * For method and errorMethod to be appropriately called, they require the following arguments:
  * >> method : (const QList<DataRow> &, int tag)
  * >> errorMethod : (const QString &, int tag)
  *
  * The following conditions do not perform any operation on the database nor invoke any methods:
  * o receiver is not defined
  * o method and errorMethod are not defined
  *
  * The following conditions will result in errorMethod being called:
  * o data source is not open
  * o sql is empty
  * o input is empty
  * o database error preparing or executing sql
  *
  * If id is included in the list of fields to select, the DataRow_.id member will be set.
  *
  * @param sql [const QString &]
  * @param input [const QList<QVariant> &]
  * @param receiver [QObject *]
  * @param method [const char *]
  * @param errorMethod [const char *]
  * @param tag [int]
  */
void SynchronousAdocDataSource::select(const QString &sql, const QList<QVariant> &input, QObject *receiver, const char *method, const char *errorMethod, int tag)
{
//    qDebug() << QString("[%1] %2:").arg(QThread::currentThreadId()).arg(__FUNCTION__) << sql;

    // Do nothing if there is no receiving object
    if (!receiver)
        return;

    // If neither the method nor the errorMethod are defined, same result as above
    if (!qstrlen(method) && !qstrlen(errorMethod))
        return;

    if (!isOpen())
    {
        // No need to check if errorMethod is defined because invokeMethod will return if that is the case
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "Data source file has not been opened"), Q_ARG(int, tag));
        return;
    }

    if (sql.isEmpty())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "Empty sql command"), Q_ARG(int, tag));
        return;
    }

    if (input.isEmpty())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "No parameters provided to SQL query"), Q_ARG(int, tag));
        return;
    }

    // Prepare the statement
    QSqlQuery selectQuery(database_);
    if (!selectQuery.prepare(sql))
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, QString("Database error: %1").arg(database_.lastError().text())), Q_ARG(int, tag));
        return;
    }

    QList<DataRow> results;
    foreach (const QVariant &parameter, input)
    {
        selectQuery.addBindValue(parameter);
        if (!selectQuery.exec())
        {
            QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, QString("Query execution error: %1").arg(selectQuery.lastError().text())), Q_ARG(int, tag));
            return;
        }

        while (selectQuery.next())
        {
            results << selectQuery.record();
            if (results.at(0).contains("id"))
                results.last().id_ = results.last().value("id");
        }
    }

    // Success!
    QMetaObject::invokeMethod(receiver, method, Q_ARG(QList<DataRow>, results), Q_ARG(int, tag));
}

/**
  * This method provides a high level interface for performing a single update of fields in
  * tableName and identified by dataRow.id_. If fields is empty, then all fields in dataRow
  * will be saved to the database. An error occurs if one or more fields in dataRow is not
  * present within the database table (because the query will fail). Thus, it is important to
  * specify at least one field if only some of the fields in dataRow should be considered for
  * updating.
  *
  * An error occurs if any of the following conditions are true:
  * -- dataRow.id_ is null
  * -- dataRow does not contain any fields
  * -- database error while executing query (e.g. table does not exist)
  *
  * To provide a generic mechanism for clients to receive data from this single interface, the
  * caller should supply a QObject receiver and a method to call on success (method) and a method
  * to call on error (errorMethod). These methods are called via the QMetaObject::invokeMethod
  * function. Only the dataRow and tableName parameters are strictly required though.
  *
  * An optional tag may be supplied with each request for identifying each request.
  *
  * For method and errorMethod to be appropriately called, they require the following arguments:
  * >> method : (const DataRow dataRow_, bool rowAffected, int tag)
  * >> errorMethod : (const QString &error, int tag)
  *
  * In method, rowAffected is set to true if a record was changed within the database or false otherwise.
  * For instance, if dataRow.id_ identifies a record not present in the database, the update will succeed,
  * but rowAffected will be false.
  *
  * @param dataRow [const DataRow &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param receiver [QObject *]
  * @param method [const char *]
  * @param errorMethod [const char *]
  * @param tag [int]
  */
void SynchronousAdocDataSource::update(DataRow &dataRow, const QString &tableName, const QStringList &fields, QObject *receiver, const char *method, const char *errorMethod, int tag) const
{
    // Note: no checking is done for whether receiver, method, or errorMethod are defined
    // because the invokeMethod will simply do nothing and return if these are null.

    // ------------------------------------------------
    // Validate the database state and user input
    if (!isOpen())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "Data source file has not been opened"), Q_ARG(int, tag));
        return;
    }

    if (!dataRow.id_.isValid())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "dataRow does not have a valid id_ member"), Q_ARG(int, tag));
        return;
    }

    if (dataRow.isEmpty())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "dataRow does not have any fields"), Q_ARG(int, tag));
        return;
    }

    if (tableName.trimmed().isEmpty())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "tableName must contain at least one non-whitespace character"), Q_ARG(int, tag));
        return;
    }

    // ------------------------------------------------
    // Build the update sql
    QStringList updateFields = (fields.isEmpty()) ? dataRow.fieldNames() : fields;
    QString updateSql = SqlBuilder::preparedUpdateSql(tableName, updateFields, "id = ?");
    if (updateSql.isEmpty())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, "Error building SQL update query. Check table name and fields"), Q_ARG(int, tag));
        return;
    }

    // ------------------------------------------------
    // Prepare the update sql
    QSqlQuery updateQuery(database_);
    if (!updateQuery.prepare(updateSql))
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, QString("Database error: %1").arg(database_.lastError().text())), Q_ARG(int, tag));
        return;
    }

    // ------------------------------------------------
    // Bind the parameters
    for (int i=0, z=updateFields.count(); i< z; ++i)
        updateQuery.bindValue(i, dataRow.value(updateFields.at(i)));
    updateQuery.bindValue(updateFields.count(), dataRow.id_);

    // ------------------------------------------------
    // Execute the SQL
    if (!updateQuery.exec())
    {
        QMetaObject::invokeMethod(receiver, errorMethod, Q_ARG(QString, QString("Database error: %1").arg(database_.lastError().text())), Q_ARG(int, tag));
        return;
    }

    if (updateFields.contains("id"))
        dataRow.id_ = dataRow.value("id");

    QMetaObject::invokeMethod(receiver, method, Q_ARG(DataRow, dataRow), Q_ARG(bool, updateQuery.numRowsAffected() > 0), Q_ARG(int, tag));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Iterates through potential connection names using the complete file name of file (without path)
  * and a suffix of the form -X, where X begins with 1 and monotonically increases by one up to
  * max inclusive. Once an available database connection name is determined, it is returned. Otherwise,
  * an empty QString is returned.
  *
  * @param file [const QString &]
  * @param max [int]
  * @returns QString
  */
QString SynchronousAdocDataSource::unusedConnectionNameFromFileName(const QString &file, int max) const
{
    ASSERT(file.length());
    ASSERT(max > 0);

    QString baseName = QFileInfo(file).fileName() + "-";
    int i = 1;
    while (QSqlDatabase::connectionNames().contains(baseName + QString::number(i)))
    {
        ++i;
        if (i > max)
            return QString();
    }

    return baseName + QString::number(i);
}

/**
  * @returns QString
  */
QString SynchronousAdocDataSource::temporaryFileName() const
{
    ASSERT(isOpen());

    QFileInfo fileInfo(sourceFile());
    QString path = QDir::tempPath();
    if (path.right(1) != "/")
        path += "/";
    QString base = path + fileInfo.baseName();

    // Generate a random number
    QString temp_name = base + QString::number(randomInteger(10000, 99999)) + ".tmp";
    while (QFile::exists(temp_name))
        temp_name = base + QString::number(randomInteger(10000, 99999)) + ".tmp";

    return temp_name;
}




// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Obsolete!!
/**
  * Reads all the sequence records from the sequence table linked to alphabet (see below) where id
  * is contained in ids. The optional tag is simply passed through as a mechanism for identifying
  * the query by the caller.
  *
  * The table that is queried depends strictly on the alphabet:
  * eAminoAlphabet -> constants::kTableAminoSeqs
  * eDnaAlphabet ->   constants::kTableDnaSeqs
  * eRnaAlphabet ->   constants::kTableRnaSeqs
  *
  * Conditions which return an error via the seqsError signal:
  * o Data source has not been previously opened
  * o Alphabet is not one of the above accepted alphabets
  * o DatabaseError
  *
  * Notes:
  * >> No results will be returned if an error is found at any point even if some rows were previously
  *    identified successfully.
  * >> It is assumed that only one record will be returned for each id. If the corresponding seq table
  *    contains multiple records for the same id, only the first record selected by the database will
  *    be returned.
  *
  * Otherwise, the seqsReady signal will be emitted with any found dataRows and the optional tag. Note,
  * success will be returned for those requests in which no rows are identified via the ids parameter.
  *
  * @param alphabet [Alphabet]
  * @param ids [const QList<int> &]
  * @param tag [int]
  * @see readSubseqs()
  */
/*
void SynchronousAdocDataSource::readSeqs(Alphabet alphabet, const QList<int> &ids, int tag)
{
    // ------------------------------------------------------------------------
    // Make sure we have a valid connection
    if (!isOpen())
    {
        emit seqsError(alphabet, "Valid data source has not yet been opened", tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Check for valid alphabet
    if (constants::kTableHash.contains(alphabet) == false
        || constants::kTableHash[alphabet].contains(eSeqGroup) == false)
    {
        emit seqsError(alphabet, "Unrecognized alphabet", tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Optimize: do nothing if ids is empty
    if (ids.isEmpty())
    {
        emit seqsReady(alphabet, QList<DataRow>(), tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Prepare and execute the database queries
    QString select_sql = "SELECT * FROM " % QString(constants::kTableHash[alphabet][eSeqGroup]) % " WHERE id = ?";
    QSqlQuery s_seqs(database_);
    s_seqs.setForwardOnly(true);
    if (!s_seqs.prepare(select_sql))
    {
        emit seqsError(alphabet, QString("Database error: %1").arg(s_seqs.lastError().databaseText()), tag);
        return;
    }

    QList<DataRow> seqs;
    foreach (int id, ids)
    {
        s_seqs.addBindValue(id);
        if (!s_seqs.exec())
        {
            emit seqsError(alphabet, QString("Database error: %1").arg(s_seqs.lastError().databaseText()), tag);
            return;
        }

        if (s_seqs.next())
            seqs << s_seqs.record();
    }

    // Finished - send user result
    emit seqsReady(alphabet, seqs, tag);
}
*/
/**
  * This function behaves very similarly to readSeqs(); however, because a subseqs should not exist apart
  * from its seq record, readSeqs is also called from this function. This is due specifically to the organization
  * of subseqs on the data tree. In most cases the user will be working from subseqs to seq data rather than
  * from a seq to all its subseqs. Thus, to ensure that the seq data is available *before* the subseq data
  * is returned, the readSeqs function is called before emitting a success signal from this method.
  *
  * If there are no subseqs found to match the requested ids, then the request to readSeqs will be skipped as
  * well. Thus, no seqsReady signal will be emitted, even though a corresponding subseqsReady signal will be.
  *
  * The table that is queried depends strictly on the alphabet:
  * eAminoAlphabet -> constants::kTableAminoSubseqs
  * eDnaAlphabet ->   constants::kTableDnaSubseqs
  * eRnaAlphabet ->   constants::kTableRnaSubseqs
  *
  * Conditions which return an error via the subseqsError signal:
  * o Data source has not been previously opened
  * o Alphabet is not one of the above accepted alphabets
  * o DatabaseError
  *
  * Notes:
  * >> No results will be returned if an error is found at any point even if some rows were previously
  *    identified successfully.
  * >> It is assumed that only one record will be returned for each id. If the corresponding subseq table
  *    contains multiple records for the same id, only the first record selected by the database will
  *    be returned.
  *
  * Otherwise, the subseqsReady signal will be emitted with any found dataRows and the optional tag. Note,
  * success will be returned for those requests in which no rows are identified via the ids parameter.
  *
  * @param alphabet [Alphabet]
  * @param ids [const QList<int> &]
  * @param tag [int]
  * @see readSeqs()
  */
/*
void SynchronousAdocDataSource::readSubseqs(Alphabet alphabet, const QList<int> &ids, int tag)
{
    // ------------------------------------------------------------------------
    // Make sure we have a valid connection
    if (!isOpen())
    {
        emit subseqsError(alphabet, "Valid data source has not yet been opened", tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Check for valid alphabet
    if (constants::kTableHash.contains(alphabet) == false
        || constants::kTableHash[alphabet].contains(eSubseqGroup) == false)
    {
        emit subseqsError(alphabet, "Unrecognized alphabet", tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Optimize: do nothing if ids is empty
    if (ids.isEmpty())
    {
        emit subseqsReady(alphabet, QList<DataRow>(), tag);
        return;
    }

    // ------------------------------------------------------------------------
    // Prepare and execute the database queries
    QString select_sql = "SELECT * FROM " % QString(constants::kTableHash[alphabet][eSubseqGroup]) % " WHERE id = ?";
    QSqlQuery s_subseqs(database_);
    s_subseqs.setForwardOnly(true);
    if (!s_subseqs.prepare(select_sql))
    {
        emit subseqsError(alphabet, QString("Database error: %1").arg(s_subseqs.lastError().databaseText()), tag);
        return;
    }

    // Build unique list of sequence identifiers while capturing the subseq results
    QString seq_id_field;
    switch (alphabet)
    {
    case eAminoAlphabet:
        seq_id_field = "amino_seq_id";
        break;
    case eDnaAlphabet:
        seq_id_field = "dna_seq_id";
        break;
    case eRnaAlphabet:
        seq_id_field = "rna_seq_id";
        break;
    default:
        ASSERT(0);

        // Release mode guard
        emit subseqsError(alphabet, "Unable to build database query", tag);
        return;
    }

    QSet<int> seq_ids;
    QList<DataRow> subseqs;
    foreach (int id, ids)
    {
        s_subseqs.addBindValue(id);
        if (!s_subseqs.exec())
        {
            emit subseqsError(alphabet, QString("Database error: %1").arg(s_subseqs.lastError().databaseText()), tag);
            return;
        }

        if (s_subseqs.next())
        {
            subseqs << s_subseqs.record();

            ASSERT_X(subseqs.last().contains(seq_id_field), QString("Source table %1 is missing the field, %2").arg(constants::kTableHash[alphabet][eSubseqGroup], seq_id_field).toAscii());

            seq_ids.insert(subseqs.last().value(seq_id_field).toInt());
        }
    }

    // Before returning the subseqs, request the relevant seqs with the same tag
    if (seq_ids.count())
        readSeqs(alphabet, seq_ids.toList(), tag);

    // Finished - send user result
    emit subseqsReady(alphabet, subseqs, tag);
}
*/
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
