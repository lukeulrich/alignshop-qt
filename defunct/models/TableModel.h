/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QStringList>

#include "DataRow.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AbstractAdocDataSource;

/**
  * TableModel defines a abstract and concrete interface for loading records from an AdocDataSource for
  * a specific table into memory and manipulating the associated database records.
  *
  * Because database systems may contain large amounts of data and the user is usually only interested
  * in a certain slice of information, it is inefficient to simply load an entire database table into
  * memory. Thus, TableModel provides a mechanism for partially loading relevant data into memory and
  * quickly accessing this data via primary key fields. Moreover, any of the database contents may be
  * modified (update or delete) regardless if they are loaded into memory.
  *
  * All data access is handled via an AdocDataSource interface, which may or may not be asynchronous.
  * To facilitate maximum performance, updates to records that are in memory occur immediately to the
  * object in memory and a request is submitted to the AdocDataSource to update the data source. If
  * updating the data source fails, an error is returned via the setDataError signal, and the revertData
  * private slot is called which reverts the data back to its original value.
  *
  * It is important to note that data retrieval via the data method only returns a valid QVariant for
  * data stored in memory. It does not query the data source directly. Thus, it is crucial to call
  * load() with the relevant identifiers to first fetch this data before requesting specific fields.
  * Similarly, setData only operates on those records that have been loaded into memory.
  *
  * By default, the name of each column is the name of each field passed into setSource; however, to
  * provide more friendly names for use with headers, etc. user's may specify alternate "friendly"
  * names. These may be accessed either with the original field name or by column position.
  *
  * Constraints:
  * o Every table must have a primary key which is labeled, id
  * o Only belongsTo and hasMany relationships are defined at this point
  *
  * TODO: Provide mechanism for unloading data from memory. When this is tackled, it will also be necessary
  *       to provide unloaded signal for connected components to properly sync their state. For instance
  *       the SubseqSliceModel will need to remove those rows that have had records unloaded. Do not want
  *       to get into circular cycle though. What if they are removed from AdocTreeModel, that issues
  *       a rowsRemoved signal which will update the SliceProxyModel/SubseqSliceModel.
  *
  * Future notes:
  * >> Provide a refresh option that refreshes the data in memory with that contained in the database?
  */
class TableModel : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct a TableModel instance with adocDataSource that models fields_ from tableName and uniquely identifies rows via the primaryKey; associated with parent
    TableModel(QObject *parent = 0);
    virtual ~TableModel();                                                          //!< Virtual destructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int columnCount() const;                                                        //!< Returns the number of fields / columns
    int fieldColumn(const QString &fieldName) const;                                //!< Returns the integer column position of fieldName or -1 if it fieldName is not found
    QVariant data(int id, const QString &fieldName) const;                          //!< Return the field value in the row identified by id and fieldName; an invalid QVariant is returned if id is not found in memory or fieldName does not exist in the fields
    QVariant data(int id, int column) const;                                        //!< Return the field value in the row identified by id and column, which should fall between 0 and the number of fields; an invalid QVariant is returned if id is not found or column is outside the field range
    QStringList fields() const;                                                     //!< Returns the list of fields in this model
    QString friendlyFieldName(int column) const;                                    //!< Returns the friendly field name for column if it is valid or an empty string otherwise
    int rowCount() const;                                                           //!< Returns the number of records currently loaded into memory
    void setFriendlyFieldNames(const QStringList &friendlyFieldNames);              //!< Sets the human friendly names of each column to friendlyFieldNames
    //! Set the table to be modeled to tableName containing fields and is stored in adocDataSource
    virtual void setSource(AbstractAdocDataSource *adocDataSource, const QString &tableName, const QStringList &fields);
    QString tableName() const;                                                      //!< Returns the name of the table being modeled

public slots:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void clear();                                                                   //!< Clears all records from memory
    virtual void load(const QList<int> &ids, int tag = 0);                          //!< Load the rows identified by ids with the optional identification tag
    //! Load the rows where foreignKey is equal to each fkIds with the optional identification tag
    virtual void loadWithForeignKey(const QString &foreignKey, const QList<int> &fkIds, int tag = 0);
//    virtual void loadWhere(const QString &sql, const QList<QVariant> &input, int tag = 0); // Would be useful for relations other than belongsTo
    bool setData(int id, const QString &fieldName, const QVariant &newValue);       //!< Sets the value of fieldName to newValue for the row identified by id and returns true on success; false otherwise
    bool setData(int id, int column, const QVariant &newValue);                     //!< Sets the value of the field in column to newValue for the row identified by id and returns true on success; false otherwise

private slots:
    // Internal slots for working with the results of the load command
    virtual void __selectReady(const QList<DataRow> &dataRows, int tag);            //!< Called when the data source has successfully finished a requested selected identified by tag
    void __selectError(const QString &error, int tag);                              //!< Called when the data source was unable to properly perform a select identified by tag
    void __setDataDone(const DataRow &dataRow, bool rowAffected, int tag);          //!< Called when the data source was successfully updated with the request identified by tag
    void __setDataError(const QString &error, int tag);                             //!< Called when the data source was unable to properly update the data source in response to a setData request

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void dataChanged(int id, int column);                                           //!< Emitted when the data value for fieldName in the row identified by id has changed
    void friendlyFieldNamesChanged(int first, int last);                              //!< Emitted after the friendly names for various columns have changed
    void loadDone(int tag);                                                         //!< Emitted when the load request identified by tag is complete
    void loadError(const QString &error, int tag);                                  //!< Emitted when the load request identified by tag has encountered an error
    void modelAboutToBeReset();                                                     //!< Emitted when the underlying model is about to be reset
    void modelReset();                                                              //!< Emitted when the underlying model has finished being reset
    void setDataError(int id, int column, const QString &error);                    //!< Emitted when the data source was unable to update the value for fieldName in the row identified by id; error contains the error message
    void sourceChanged(TableModel *);

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void appendRecords(const QList<DataRow> &dataRows);                             //!< Utility function that appends dataRows (that are not already present) to this models internal data store
    QStringList normalizeFields(const QStringList &fields) const;                   //!< Returns fields with id in the first position if fields is not-empty; otherwise, simply returns fields unchanged (empty list)

    AbstractAdocDataSource *adocDataSource_;

private:
    // ------------------------------------------------------------------------------------------------
    // Private structures
    //! CellData stores the field data, value_, for column_ of the row identified by id; the request is identified with tag_
    struct CellData {
        int tag_;
        int id_;
        int column_;
        QVariant value_;

        CellData(int tag, int id, int column, const QVariant &value) : tag_(tag), id_(id), column_(column), value_(value) {}
    };

    QString tableName_;
    QStringList fields_;
    QStringList friendlyFieldNames_;            //!< Human friendly names of columns
    int nColumns_;

    QList<DataRow> records_;
    QHash<int, int> idLookup_;
    QList<CellData> oldCellValues_;          //!< {tag} -> [id, oldValue]; used to remember the oldValue of a cell before setData was called in case updating the data source failed

#ifdef TESTING
    friend class TestTableModel;
#endif
};

Q_DECLARE_METATYPE(TableModel *);

#endif // TABLEMODEL_H
