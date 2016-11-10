/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOC_H
#define ADOC_H

#include <QtCore/QHash>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>

// ------------------------------------------------------------------------------------------------
// Forward declaration
class AbstractAdocDataSource;
class AdocTreeModel;
class AnonSeqFactory;
class DbSpec;

/**
  * AlignShop document interface
  *
  * An AlignShop document primarily consists of a SQLite database and an arbitrarily organized tree that functions
  * similarly to that of windows explorer. Rather than store all application data in memory, user and project data
  * is stored in the database and retrieved on demand.
  *
  * Using a SQLite database as the application file format has several advantages:
  * o Given an understanding of the schema, it is possible for users and other programs to directly interact with this
  *   information
  * o It is not necessarily essential to build complex data structures for representing every type of data
  * o Standard and mature mechanism for reading/writing data to a predictable structure
  * o Can offload various operations to the database engine (e.g. sorting, filtering, etc.)
  *
  * And it also presents some additional challenges:
  * o Potentially more susceptible to data corruption by external programs and/or users
  * o Table updates are immediately applied and thus changes are immediately preserved. This may not be preferred in cases
  *   where users are more familiar with the notion that changes are only accepted once they save the document. Moreover,
  *   not all changes are immediately saved. For example, updating an alignment will not be written to the database until
  *   the user saves.
  *   >> A sub-challenge includes maintaining undo/redo for each of these separate functionalities.
  *
  * By design, all annotation data is immediately saved to the database. Thus, the traditional notion of all changes
  * not being saved until the user saves does not strictly apply in this case and no save method is provided here. On the
  * other hand, "Save As..." provides for saving this data to another file. Depending on the size of the application file,
  * this may take a significant amount of time.
  *
  * All data is organized into a hierarchical and arbitrarily deep tree. All objects - sequences, subsequences, primers,
  * alignments, etc. - are organized into this tree (Tree + AdocTreeNodes), which provides the underlying structure of
  * the data organizer.
  *
  * This class provides the basic functionality of opening a database file and upon destruction, properly writing the
  * data tree to and closing the database.
  *
  * Currently and for simplification, copying and assigning Adoc objects is not supported.
  *
  * Because the data file consists of a SQLite database, it is necessary to map the traditional file I/O to the relevant
  * database commands. Open simply opens the file and executes a BEGIN TRANSACTION to obtain exclusive access. Saving
  * simply commits the open transaction and then begins another. Close shuts down the database connection. This process
  * is suggested by the SQLite maintainers. Specifically, immediate transaction are used which permit external reading,
  * but completely limit all write access.
  *
  * Because database changes and interactions are distributed, configuring whether a document has been modified will
  * also be controlled in a distributed fashion. External database interactions that manipulate the database file are
  * responsible for specificying that the document has been modified. Furthermore, the modified state also changes as
  * the in-memory data_tree is manipulated in addition to purely database changes.
  *
  * Validation of an Adoc database will be performed by a separate class.
  *
  * ---------------------------------------------------------------------------------------------------
  * Future considerations:
  * o Adoc blends database application file and AlignShop-specific features (e.g. data tree commit on save). A more flexible
  *   design would separate these functionalities into distinct classes. Specifically, an abstract base class that provides
  *   an interface for opening/saving/closing a database similar to an application file. Concrete classes would derive from
  *   this ABC and implement the database-specific interactions. Then Adoc would build on this functionality and layer on
  *   AlignShop specific features as needed.
  *
  * o Fix counter-intuitive tree handling: Context - AlignShop file with a non-empty data_tree. Currently, if one opens,
  *   and then immediately follows this with a save, the data_tree will be truncated to nil. Why? Because the data_tree is
  *   not loaded into memory separately from opening the document file. Thus, when save is called, Adoc contains an empty
  *   data_tree and writes this to the database.
  *
  *   The proper way for handling this is as follows: open() -> loadTreeFromDatabase() -> {do other processing...} -> save()
  *
  *   Why separate open and loadTreeFromDatabase? More granular control and testing
  */
class Adoc : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    Adoc(QObject *parent = 0);                                  //!< Construct an uninitialized AlignShop document

    // ------------------------------------------------------------------------------------------------
    // Destructors
    ~Adoc();                                                    //!< Close the database connection and cleanup

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool close();                                               //!< Close the Adoc database file without saving. Returns true if successful, false otherwise
    void create(const QString &file, const DbSpec &spec);       //!< Create a new Adoc with the name, file, and that adheres to spec. Returns true if operation was successful, false otherwise
    const AbstractAdocDataSource *dataSource() const;                   //!< Returns a copy of the currently associated data source [UNTESTED]
    AdocTreeModel *dataTreeModel() const;                       //!< Returns the tree model currently in use [UNTESTED]
    QString databaseFile() const;                               //!< Returns the current Adoc file or the temporary file name document is temporary
    bool isModified() const;                                    //!< Returns whether this document has been modified
    bool isOpen() const;                                        //!< Returns true if Adoc is currently active and opened; othwerise returns false
    bool isTemporary() const;                                   //!< Returns whether this document is temporary
    void loadTreeFromDatabase();                                //!< Loads the data tree from the database and populates dataTreeModel_
    void saveAs(const QString &file);                           //!< Save the current SQLite database to file and return whether this operation was successful

public slots:
    void open(const QString &file);                             //!< Open the Adoc named file
    void setModified(bool modified);                            //!< Sets the modified flag for cases when the document has been changed

private slots:

signals:
    void modifiedChanged(bool newModified);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void transaction() const;                       //!< Begin an immediate transaction, rather than simply opening a transaction

    // ------------------------------------------------------------------------------------------------
    // Private variables
    bool modified_;                                 //!< Boolean flag denoting whether this document has been modified

    AbstractAdocDataSource *adocDataSource_;        //!< Internally managed data source
    QScopedPointer<AdocTreeModel> dataTreeModel_;   //!< Model for interacting with and manipulating the data tree; relies on valid Adoc
};

#endif // ADOC_H
