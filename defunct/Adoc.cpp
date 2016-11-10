/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "Adoc.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <QtSql/QSqlError>

#include "exceptions/DatabaseError.h"
#include "exceptions/FatalError.h"
#include "exceptions/InvalidMpttNodeError.h"
#include "exceptions/RuntimeError.h"
#include "models/AdocTreeModel.h"

#include "AdocDataSource.h"
#include "AdocTreeNode.h"
#include "AnonSeqFactory.h"
#include "CrudSqlRecord.h"
#include "DbSpec.h"
#include "MpttTreeConverter.h"

#include "global.h"

#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static variable initialization
QString Adoc::databaseConnectionPrefix_ = "adoc-";


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Set modified_ here (rather than calling setModified()) because we do not want the modified
  * signal to be emitted during initialization.
  */
Adoc::Adoc(QObject *parent) : QObject(parent), modified_(false)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
/**
  * All cleanup is handled by the close function.
  */
Adoc::~Adoc()
{
    close();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods

/**
  * Close any open database connection, release memory, and reset all internal private members. Clear
  * object instance to an uninitialized state. Manually reset modified here because we do not want to
  * emit the modified signal when it is closing.
  *
  * Changes are not saved.
  *
  * @TODO: emit signal!
  *
  * @returns bool
  */
bool Adoc::close()
{
    if (!isOpen())
        return false;

    dataTreeModel_.reset();

    // Rollback any unsaved changes
    adocDataSource_.database().rollback();

    // Close the database connection
    adocDataSource_.database().close();

    adocDataSource_.setConnectionName("");

    tempAdocFile_.reset();
    adocFile_ = QString();

    modified_ = false;

    return true;
}


/**
  * Create a new Adoc document that is physically stored at file and with a SQLite structure defined
  * by spec. If file is NULL, then a temporary file is automatically used until the user saves the
  * document for the first time, at which point he will be prompted for a permanent file.
  *
  * If a temporary file is created (if file is NULL), then in essence, the Adoc is modified (i.e. unsaved)
  * because it will be deleted unless saved to a permanent file. On the other hand, if file is a
  * specific name, then modified is false because the file is a permanent file containing the basic
  * DDL structure, which if closed would still exist.
  *
  * Begins a new transaction for managing all database interactions.
  *
  * Direct throws:
  * o RuntimeError - document already open, invalid specification, unable to create temporary file
  * o DatabaseError - unable to open database connection
  *
  * Subthrows
  * o InvalidConnectionError - setting the AdocDbDataSource
  * o DatabaseError - unable to create database file (e.g. bad path, etc), unable to initiate transaction, executing query
  *
  * @param file [const QString &]
  * @param spec [const DbSpec &]
  *
  * @throws RuntimeError, DatabaseError
  */
void Adoc::create(const QString &file, const DbSpec &spec)
{
    if (isOpen())
        throw RuntimeError(QString("Unable to create database file '%1': another document is already open. Either close the current document first or create a new document").arg(!file.isEmpty() ? file : "[undefined]"));

    if (!spec.isLoaded())
        throw RuntimeError(QString("Unable to create database file '%1': database specification was not loaded").arg(!file.isEmpty() ? file : "[undefined]"));

    if (!spec.isLogicallyValid())
        throw RuntimeError(QString("Unable to create database file '%1': database specification is not logically valid").arg(!file.isEmpty() ? file : "[undefined]"));

    if (!file.isEmpty())
    {
        if (QFile::exists(file))
            throw RuntimeError(QString("Unable to create database file '%1': file already exists").arg(file));

        adocFile_ = file;
    }
    else
    {
        tempAdocFile_.reset(new QTemporaryFile("adoc"));

        // The actual temporary file is not created until we call open
        if (!tempAdocFile_->open())
        {
            tempAdocFile_.reset();

            throw RuntimeError("Unable to create temporary database file");
        }

        // Since we do not need the open filehandle, close the temporary file. The temporary file will only
        // be deleted when it is destroyed, thus it is safe to go ahead and close this file.
        tempAdocFile_->close();
    }

    // Initialize the database connection
    QString connectionName = uniqueConnectionName();
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    database.setDatabaseName(databaseFile());

    // Open the database
    if (!database.open())
    {
        // Cleanup
        if (isTemporary())
            tempAdocFile_.reset();
        else
        {
            QFile::remove(adocFile_);
            adocFile_ = QString();
        }

        throw DatabaseError(QString("Unable to create and open database file '%1'").arg(databaseFile()), database.lastError());
    }

    // Make the data source valid by providing it with the connection name
    // THROW: InvalidConnectionError
    adocDataSource_.setConnectionName(connectionName);

    // Execute the SQL
    transaction();
    foreach (QString query_string, spec.sqlQueryList())
    {
        QSqlQuery query = database.exec(query_string);
        if (query.lastError().type())
        {
            // Cleanup
            database.close();
            adocDataSource_.setConnectionName("");

            if (isTemporary())
                tempAdocFile_.reset();
            else
            {
                QFile::remove(adocFile_);
                adocFile_ = QString();
            }

            throw DatabaseError(constants::kMessageErrorExecutingQuery, database.lastError(), query_string);
        }
    }
    database.commit();
    transaction();

    // See function documentation for why a temporary file is considered modified
    setModified(isTemporary());
}

/**
  * @returns AdocDbDataSource
  */
AdocDbDataSource Adoc::dataSource() const
{
    return adocDataSource_;
}

/**
  * @returns AdocTreeModel *
  */
AdocTreeModel *Adoc::dataTreeModel() const
{
    return dataTreeModel_.data();
}

/**
  * Return the temporary file name if in use, otherwise return the the permanent file name even if empty.
  *
  * @returns const QString &
  */
QString Adoc::databaseFile() const
{
    if (tempAdocFile_.isNull() == false)
        return tempAdocFile_->fileName();

    return adocFile_;
}

/**
  * @returns bool
  */
bool Adoc::isModified() const
{
    return modified_;
}

/**
  * @returns bool
  */
bool Adoc::isOpen() const
{
    // Technically, if a document is open, database_ should always be open
    return (!adocFile_.isEmpty() || !tempAdocFile_.isNull() || adocDataSource_.databaseReady());
}

/**
  * An Adoc is temporary if it has been created with a NULL file. Similar to isModified(), returns false
  * if a document has not been opened or created.
  *
  * @returns bool
  */
bool Adoc::isTemporary() const
{
    return (tempAdocFile_.isNull()) ? false : true;
}

/**
  * Read the arbitrarily deep data tree from the database table, constants::kTableDataTree, transform its MPTT representation
  * into an AdocTreeNode structure managed by dataTreeModel_.
  *
  * Direct throws:
  * o RuntimeError - document not open, tree already loaded
  *
  * Subthrows:
  * o InvalidConnectionError/DatabaseError - check table validity
  * o RuntimeError - unrecognized Mptt node
  *
  * Rethrows:
  * o InvalidMpttNodeError - data tree contained invalid mptt node
  *
  * @throws InvalidConnectionError, DatabaseError, InvalidMpttNodeError, RuntimeError
  */
void Adoc::loadTreeFromDatabase()
{
    if (!isOpen())
        throw RuntimeError("Unable to load tree because no document has been loaded");

    if (dataTreeModel_.isNull() == false)
        throw RuntimeError("Unable to load tree because one has already been loaded for this instance");

    // Will throw an InvalidConnectionError if the datasource has not been configured with a valid connection name (
    // this should be set in the open method).
    QList<MpttNode *> dataTreeRows = adocDataSource_.readDataTree(constants::kTableDataTree);

    // Must catch any InvalidMpttNodeError to properly de-allocate dataTreeRows
    AdocTreeNode *root = 0;
    try
    {
        root = static_cast<AdocTreeNode *>(MpttTreeConverter::fromMpttList(dataTreeRows));
    }
    catch (InvalidMpttNodeError &e)
    {
        freeMpttList(dataTreeRows);
        dataTreeModel_.reset();
        throw;
    }

    // We do not want to call freeMpttList because it will also destroy the allocated TreeNode pointers; however,
    // we do want to release the memory used by the MpttNode pointers in the dataTreeRows list
    qDeleteAll(dataTreeRows);
    dataTreeRows.clear();

    dataTreeModel_.reset(new AdocTreeModel());  // The constructor of AdocTreeModel creates a bare root node
    if (root)  // Only replace AdocTreeModel root if we were able to transform the set of MpttNodes adequately
        dataTreeModel_->setRoot(root);

    // Update the model data source
    dataTreeModel_->adocDbDataSource_ = adocDataSource_;
}

/**
  * If file is successfully opened, initiates an immediate transaction for two purposes: 1) obtaining
  * exclusive write access and 2) providing mechanism for discarding all changes. Returns false if file
  * is empty or another SQLite database file is already open in this instance.
  *
  * After opening file, nothing is done to validate that this is an acceptably structured data file. That
  * is the responsibility of a separate validator class. Thus, it is possible to open any SQLite database
  * file with this function.
  *
  * After opening the database file, initializes a begin immediate transaction which should lock down the
  * database for writing and will fail if another process has it locked down. If this should occur, the
  * database will be closed and this method will return false.
  *
  * DRY: Does not initialize the tree from the database.
  *
  * Direct throws:
  * o RuntimeError - empty file, file does not exist, document already open
  * o DatabaseError - opening database error
  *
  * Subthrows:
  * o InvalidConnectionError - cannot set AdocDbDataSource connection name
  * o DatabaseError - starting transaction
  *
  * @param file [const QString &]
  * @throws RuntimeError, DatabaseError, InvalidConnectionError
  */
void Adoc::open(const QString &file)
{
    if (file.isEmpty())
        throw RuntimeError("Unable to open database file '[NULL]': please provide a valid SQLite file");

    if (!QFile::exists(file))
        throw RuntimeError(QString("Unable to open database file '%1': file does not exist").arg(file));

    if (isOpen())
        throw RuntimeError(QString("Unable to open database file '%1': another document is already open. Either close the current document first or create a new document").arg(file));

    // Initialize the database connection
    QString connectionName = uniqueConnectionName();
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    database.setDatabaseName(file);

    // Open the database
    if (!database.open())
        throw DatabaseError(QString("Unable to open database file '%1'").arg(file), database.lastError());

    // Make the data source valid by providing it with the connection name
    adocDataSource_.setConnectionName(connectionName);            // <-- Can throw InvalidConnectionError

    // The open operation is a little different than operations on an already opened file, thus we
    // catch any transactional error here to ensure that this method remains exception safe.
    try
    {
        database.exec("PRAGMA synchronous=off");
        transaction();
    }
    catch (DatabaseError &e)
    {
        database.close();
        adocDataSource_.setConnectionName("");
        throw;
    }

    adocFile_ = file;
    setModified(false);
}

/**
  * Saving an Adoc currently does 5 things:
  * 0. Commit and begin new transaction
  * 1. Truncates the data_tree table within the database
  * 2. Write the data_tree in memory to data_tree table
  * 3. Commit the transaction and begins new transaction
  * 4. Set database to an unmodified state
  * 5. Begin a new transaction to retain the locked status
  *
  * These are performed regardless of the modified_ variable. If any of these steps fail, an error message
  * is sent to the message handler and false is returned. If the Adoc is not already open, returns false.
  *
  * Originally, if isTemporary() then would redirect to saveAs; however, that should be handled at a higher
  * level in the application. Thus, this will work for both temporary and permanent database files.
  *
  * Future changes to be implemented with regard to the database validation
  * - delete checksum from __info
  * - crank SHA1 digest
  * - write digest to the __info
  * - commit the transaction
  *
  * Direct throws:
  * o RuntimeError - database file is not open
  *
  * Subthrows:
  * o DatabaseError - unable to initiate transactions, missing table, executing/preparing queries
  * o InvalidConnectionError
  *
  * @throws RuntimeError
  */
void Adoc::save()
{
    if (!isOpen())
        throw RuntimeError("No database file has been opened");

    // Commit any oustanding database updates
    adocDataSource_.database().commit();

    // Begin a new transaction containing the data tree table modifications
    transaction();
    adocDataSource_.truncate(constants::kTableDataTree);
    adocDataSource_.resetSqliteSequence(constants::kTableDataTree);
    if (dataTreeModel_)
    {
        Q_ASSERT_X(dataTreeModel_->root(), "Adoc::save", "Valid data tree model should have valid root");
        QList<MpttNode *> mpttNodes = MpttTreeConverter::toMpttList(dataTreeModel_->root());
        adocDataSource_.saveDataTree(mpttNodes, constants::kTableDataTree);
    }
    adocDataSource_.database().commit();

    // Begin new transaction for capturing all future modifications
    transaction();

    setModified(false);
}


/**
  * Closes the SQLite connection, copies the current adocFile_ to file and then reopens it. Throws a RuntimeError
  * if an existing document is not already open or if file already exists.
  *
  * The saveAs method takes the following steps:
  * o Copy adocFile_ to temporary file
  * o save()
  * o close()
  * o rename current document to file
  * o move temporary_file to old file (this presents potential issues with currently opened external
  *   connections)
  * o open(file)
  *
  * The major issue is dealing with any changes that are contained within the current transaction. It is
  * essential that these are captured into the new file to be saved, but not committed to the old (current)
  * open file. This works mostly as expected; however, on Linux, any previously opened, external connections
  * will operate on new database file and not the original file as might be expected.
  *
  * Another caveat here, if we call close(), that will reset the dataTreeModel and this will cascade to the
  * various views accordingly. Additionally, close() emits the close signal which will shut down all relevant
  * windows (e.g. alignment editor, etc.). Therefore, to avoid this side-effects, we simply close/open the
  * relevant database connections here and update the datasource variables. This should work because the in-memory
  * structure will be perfectly in sync with the latest database snapshot which we are simply copying.
  *
  * Direct throws:
  * o RuntimeError - empty file, document not open, file exists, unable to create temporary file, ...
  * o FatalError - unable to re-open database after it has been closed
  *
  * @param file [const QString &]
  *
  * ASSUME: file is valid! Will have to change this :)
  * @TODO: Validate file
  */
void Adoc::saveAs(const QString &file)
{
    if (file.isEmpty())
        throw RuntimeError("Please provide a valid filename");

    if (!isOpen())
        throw RuntimeError(QString("Unable to save to '%1': no database file has previously been opened").arg(file));

    if (QFile::exists(file))
        throw RuntimeError(QString("Unable to save to '%1': file already exists").arg(file));

    // Sanity check: can we create this file?
    // A quick file creation test is a roundabout way of validating that file is a valid usable filename
    // without resorting to all sorts of cross-platform compatible checks
    {
        QFile testFile(file);
        if (!testFile.open(QIODevice::WriteOnly))
            throw RuntimeError(QString("Unable to create file '%1': %2").arg(file, testFile.errorString()));

        if (!testFile.remove())
            throw RuntimeError(QString("Unable to create file '%1'"));
    }

    // Create temporary file to hold old version of the database
    QTemporaryFile temp_file;
    if (!temp_file.open())
        throw RuntimeError("Unable to create temporary file");
    temp_file.close();

    // Copy database with uncommitted changes to the temporary file; because the copy will fail if the destination file
    // exists, remove the temporary file first
    QFile::remove(temp_file.fileName());
    if (!QFile::copy(adocFile_, temp_file.fileName()))
        throw RuntimeError(QString("Error copying current document (%1) to temporary file (%2)").arg(adocFile_).arg(temp_file.fileName()));

    save();

    // Stealthily close the current database file. The goal is to transparently update the database connection to a new file
    // without triggering a visual update.
    // A. Close the current database connection
    adocDataSource_.database().close();

    // Now move the latest database to its final destination
    if (!QFile::rename(adocFile_, file))
    {
        // Re-open the database connection so that it is still usable even though the saveAs operation failed.
        if (!adocDataSource_.database().open())
        {
            throw FatalError(FatalError::eReopenDatabaseCode,
                             QString("Unable to re-open database while saving to alternate file '%1'. Error: %2").arg(file, adocDataSource_.database().lastError().text()));
        }

        throw RuntimeError(QString("Unable to rename (move) %1 to %2").arg(adocFile_, file));
    }

    // Update the file and database name to reflect its new status
    QString old_file = adocFile_;
    adocFile_ = file;

    // And move the previous version to the original file
    QFile myTempFile(temp_file.fileName());
    if (!myTempFile.rename(old_file))
    {
        adocDataSource_.database().setDatabaseName(adocFile_);
        // Attempt to open the database connection to the new file, so that user has workable setup
        if (!adocDataSource_.database().open())
        {
            // Put application in a default state
            dataTreeModel_.reset();
            adocDataSource_.setConnectionName("");
            tempAdocFile_.reset();
            adocFile_ = QString();
            modified_ = false;

            throw DatabaseError(QString("File successfully saved as '%1', but unable to open. Please try again").arg(file), adocDataSource_.database().lastError());
        }

        throw RuntimeError(QString("Unable to rename (move) temporary file %1 to %2: %3").arg(temp_file.fileName(), old_file, myTempFile.errorString()));
    }

    // Finally re-open the saved version of the database
    adocDataSource_.database().setDatabaseName(adocFile_);
    if (!adocDataSource_.database().open())
    {
        // Put application in a default state
        dataTreeModel_.reset();
        adocDataSource_.setConnectionName("");
        tempAdocFile_.reset();
        adocFile_ = QString();
        modified_ = false;

        throw DatabaseError(QString("File successfully saved as '%1', but unable to open. Please try again").arg(file), adocDataSource_.database().lastError());
    }
}

/**
  * Declare that the document has been changed
  *
  * @param modified [bool]
  */
void Adoc::setModified(bool modified)
{
    if (modified != modified_)
    {
        modified_ = modified;
        emit modifiedChanged(modified);
    }
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * While QSqlDatabase::transaction() begins a new transaction for the given database, the extent of this transaction is not
  * known. Therefore, isolating the opening of transactions to this function ensures that we are performing a locked transaction.
  *
  * @throws DatabaseError
  */
void Adoc::transaction() const
{
    if (adocDataSource_.database().exec("BEGIN IMMEDIATE TRANSACTION").lastError().type() != QSqlError::NoError)
    {
        qDebug() << adocDataSource_.database().lastError().text();

        throw DatabaseError("Unable to begin transaction: please check if another process is using this database", adocDataSource_.database().lastError());
    }
}
