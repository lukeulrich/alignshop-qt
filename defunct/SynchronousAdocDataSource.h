/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SYNCHRONOUSADOCDATASOURCE_H
#define SYNCHRONOUSADOCDATASOURCE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>

#include "AbstractAdocDataSource.h"
#include "DataRow.h"

#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class DataRow;
class DbAnonSeqFactory;

/**
  * SynchronousAdocDataSource defines a concrete implementation of the AbstractAdocDataSource interface
  * for synchronous (blocking) communication with an AlignShop SQLite data file.
  */
class SynchronousAdocDataSource : public AbstractAdocDataSource
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructors
    //! Trivial constructor
    explicit SynchronousAdocDataSource(QObject *parent = 0);
    //! Virtual destructor
    virtual ~SynchronousAdocDataSource();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns the associated anonymous sequence factories
    QHash<Alphabet, DbAnonSeqFactory *> anonSeqFactories() const;
    const QSqlDatabase getDatabase() const;                     //!< Returns the QSqlDatabase object associated with this data source
    virtual bool isOpen() const;                                //!< Return true if the data source is currently open; false otherwise
    virtual QString sourceFile() const;                         //!< Returns the filename containing the source data or an empty QString if it not currently open

public slots:
    virtual void open(const QString &file);                     //!< Initiate opening the data source contained in file; emits opened() on success or openError() otherwise
    //! Create a new SQLite database named file initialized with ddlQueries; emits createDone() on success or createError() otherwise
    virtual void create(const QString &file, const QStringList &ddlQueries = QStringList());
    virtual void close();                                       //!< Close any open data source and if so, emit closed()

    //! Import the alignment stored in parseResult; emits importAlignmentDone() on success or importAlignmentError() otherwise
//    virtual void importAlignment(const ParseResult &parseResult);
    //! Import the sequences stored in parseResult; emits importSequencesDone() on success or importSequencesError() otherwise
//    virtual void importSequences(const ParseResult &parseResult);

    virtual void readDataTree();                                //!< Read and construct the data tree; emits dataTreeReady() on success or dataTreeError() otherwise
    //!< Read the Msa identified by id for alphabet with optional tag; emits msaReady() on success or msaError() otherwise
    virtual void readMsa(int id, Alphabet alphabet, int tag = 0);
    //! Sets (or replaces) the AnonSeqFactory to utilize in conjunction with alphabet; takes ownership of anonSeqFactory
    void setAnonSeqFactory(Alphabet alphabet, DbAnonSeqFactory *anonSeqFactory);
    virtual void saveAs(const QString &file);                   //!< Transparently save the current database to fileName and reopen this file for all future interactions; emits saveAsDone() on success or saveAsError() otherwise

    // ----------
    // Basic CRUD
    //! Execute sql for each value of input and on success call method of receiver with results; otherwise call the receiver errorMethod; optional tag is passed along to both method and errorMethod
    virtual void select(const QString &sql, const QList<QVariant> &input, QObject *receiver, const char *method, const char *errorMethod, int tag = 0);
    //! Updates fields in dataRow and on success call method of receiver with number of rows affected; otherwise call the receiver errorMethod; optional tag is passed along to both method and errorMethod
    virtual void update(DataRow &dataRow, const QString &tableName, const QStringList &fields = QStringList(), QObject *receiver = 0, const char *method = 0, const char *errorMethod = 0, int tag = 0) const;

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Returns a unique, available database connection name equivalent to the complete file name of file with a -X suffix where suffix is in the range of 1..max; returns an empty string otherwise
    QString unusedConnectionNameFromFileName(const QString &file, int max = 10) const;
    QString temporaryFileName() const;                          //!< Returns a pseudo temporary filename - one that doesn't exist in the current directory; not completely safe

    QSqlDatabase database_;
    QString sourceFile_;
    QHash<Alphabet, DbAnonSeqFactory *> anonSeqFactories_;

    // Because ThreadedAdocDataSource is simply a wrapper around this class, it is necessary to provide
    // access to its private members for non-slot functions that return a value. Of course, it is essential
    // that ThreadedAdocDataSource utilizes the mutexes where necessary.
    friend class ThreadedAdocDataSource;

#ifdef TESTING
    friend class TestSynchronousAdocDataSource;
    friend class TestTableModel;
#endif

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // Obsolete!!
    /*
public slots:
    //! Read the sequence records for the given alphabet that are identified in ids with the optional tag; emits seqsReady() on success or seqsError() otherwise
    virtual void readSeqs(Alphabet alphabet, const QList<int> &ids, int tag = 0);
    //! Read the sub-sequence records for the given alphabet that are identified in ids with the optional tag; emits subseqsReady() on success or subseqsError() otherwise
    virtual void readSubseqs(Alphabet alphabet, const QList<int> &ids, int tag = 0);
    */
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
};

#endif // SYNCHRONOUSADOCDATASOURCE_H
