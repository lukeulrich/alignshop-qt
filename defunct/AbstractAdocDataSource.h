/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCDATASOURCE_H
#define ADOCDATASOURCE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "DataRow.h"
#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AdocTreeNode;
class DbAnonSeqFactory;
class Msa;
// struct ParseResult;

/**
  * AbstractAdocDataSource defines and partially implements the interface for communicating with an
  * AlignShop SQLite data file.
  *
  * This interface provides a set of methods and signals/slots for interacting with AlignShop data
  * stored within a SQLite database file on the filesystem.
  *
  * Most methods do not return a value; rather, where relevant, responses are returned via signals.
  * This approach provides a more flexible system of interaction that is amenable to threaded access
  * that circumvents blocking requests. Each slot typically has two associated signals - one for
  * success and one signifying an error occurred. Typically, slots are either requesting data (similar
  * to a select) or initiating an action. The former methods names begin with "read" and its signals
  * are suffixed with Ready (success) or Error (failure). For instance:
  *
  * SLOT: readDataTree
  * SIGNALS: dataTreeReady :: success
  *          dataTreeError :: failure
  *
  * Slot commands may be named arbitrarily; however, their signals are named with the same name and
  * the "Done" suffix to indicate success, "Ready" suffix to indicate data is completed loading, or
  * "Error" to indicate failure.
  *
  * Some slots provide for passing optional contextual data that is passed onto its cognate signals.
  * While this data is not necessarily utilized by the slot, it is oftentimes useful for the object
  * receiving this signal. For instance, some slots accept an optional identification tag.
  *
  * A critical component is that of creating and saving anonymous sequence data, which is especially
  * critical when importing sequence data, creating new sequences, or fetching alignments. Thus, one
  * or more user-specified DbAnonSeqFactories corresponding to a specific Alphabet may be supplied by
  * the user for this purpose.
  *
  * It is vital that all non-virtual functions are thread-safe. This is easily accomplished for those
  * functions that solely return a copy of an implicitly shared object (e.g. QString or QHash).
  *
  * TODO: Define and implement the CRUD interface
  * TODO: Optimize via utilizing prepared queries from global query caching object.
  */
class AbstractAdocDataSource : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructors
    //! Trivial constructor
    explicit AbstractAdocDataSource(QObject *parent = 0) : QObject(parent) {}
    //! Virtual destructor
    virtual ~AbstractAdocDataSource()                                      {}

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns the associated anonymous sequence factories
    virtual QHash<Alphabet, DbAnonSeqFactory *> anonSeqFactories() const = 0;
    virtual const QSqlDatabase getDatabase() const = 0;             //!< Returns the QSqlDatabase object associated with this data source
    virtual bool isOpen() const = 0;                                //!< Return true if the data source is currently open; false otherwise
    virtual QString sourceFile() const = 0;                         //!< Returns the filename containing the source data or an empty QString if it not currently open

public slots:
    virtual void open(const QString &file) = 0;                     //!< Initiate opening the data source contained in file; emits opened() on success or openError() otherwise
    //! Create a new SQLite database named file initialized with ddlQueries; emits createDone() on success or createError() otherwise
    virtual void create(const QString &file, const QStringList &ddlQueries) = 0;
    virtual void close() = 0;                                       //!< Close any open data source and if so, emit closed()

    //! Import the alignment stored in parseResult; emits importAlignmentDone() on success or importAlignmentError() otherwise
//    virtual void importAlignment(const ParseResult &parseResult);
    //! Import the sequences stored in parseResult; emits importSequencesDone() on success or importSequencesError() otherwise
//    virtual void importSequences(const ParseResult &parseResult);

    virtual void readDataTree() = 0;                                //!< Read and construct the data tree; emits dataTreeReady() on success or dataTreeError() otherwise
    virtual void readMsa(int id, Alphabet alphabet, int tag) = 0;   //!< Read the Msa identified by id for alphabet with optional tag; emits msaReady() on success or msaError() otherwise
    //! Sets (or replaces) the AnonSeqFactory to utilize in conjunction with alphabet; takes ownership of anonSeqFactory
    virtual void setAnonSeqFactory(Alphabet alphabet, DbAnonSeqFactory *anonSeqFactory) = 0;
    virtual void saveAs(const QString &file) = 0;                   //!< Transparently save the current database to fileName and reopen this file for all future interactions; emits saveAsDone() on success or saveAsError() otherwise

    // ----------
    // Basic CRUD
    //! Execute sql for each value of input and on success call method of receiver with results; otherwise call the receiver errorMethod; optional tag is passed along to both method and errorMethod
    virtual void select(const QString &sql, const QList<QVariant> &input, QObject *receiver, const char *method, const char *errorMethod, int tag) = 0;
    //! Updates fields in dataRow and on success call method of receiver with number of rows affected; otherwise call the receiver errorMethod; optional tag is passed along to both method and errorMethod
    virtual void update(DataRow &dataRow, const QString &tableName, const QStringList &fields, QObject *receiver, const char *method, const char *errorMethod, int tag) const = 0;

//  virtual void query(const QString &sql, QObject *receiver, const char *method, const char *errorMethod, int tag = 0);
//  virtual void query(const QString &sql, const QList<QVariant> &input, QObject *receiver, const char *method, const char *errorMethod, int tag = 0);
//  virtual void selectMulti(const QString &sql, const QList<QVariantList> &input, QObject *receiver, const char *method, const char *errorMethod);

signals:
    void opened();
    void openError(const QString &file, const QString &error);

    void createError(const QString &file, const QString &error);
    void createDone(const QString &file);

    void closed();

    void dataTreeError(const QString &error);
    void dataTreeReady(AdocTreeNode *root);

    void msaReady(Msa *, int tag);
    void msaError(const QString &error, int tag);

//    void importAlignmentDone(const ParseResult &parseResult, int msaId);
//    void importAlignmentError(const ParseResult &parseResult, const QString &error);

//    void importSequencesDone(const ParseResult &parseResult, const QList<int> &subseqIds);
//    void importSequencesError(const ParseResult &parseResult, const QString &error);

    void saveAsError(const QString &file, const QString &error);
    void saveAsDone(const QString &file);





    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // Obsolete!!
    /*
public slots:
    //! Read the sequence records for the given alphabet that are identified in ids with the optional tag; emits seqsReady() on success or seqsError() otherwise
    virtual void readSeqs(Alphabet alphabet, const QList<int> &ids, int tag) = 0;
    //! Read the sub-sequence records for the given alphabet that are identified in ids with the optional tag; emits subseqsReady() on success or subseqsError() otherwise
    virtual void readSubseqs(Alphabet alphabet, const QList<int> &ids, int tag) = 0;

signals:
    void seqsError(Alphabet alphabet, const QString &error, int tag);
    void seqsReady(Alphabet alphabet, const QList<DataRow> &dataRows, int tag);
    void subseqsError(Alphabet alphabet, const QString &error, int tag);
    void subseqsReady(Alphabet alphabet, const QList<DataRow> &dataRows, int tag);
    */
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
};

#endif // ADOCDATASOURCE_H
