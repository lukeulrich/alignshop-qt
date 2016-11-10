/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "TableModel.h"

#include "AbstractAdocDataSource.h"
#include "TagGenerator.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
TableModel::TableModel(QObject *parent) : QObject(parent)
{
    adocDataSource_ = 0;
    nColumns_ = 0;
}

/**
  */
TableModel::~TableModel()
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int TableModel::columnCount() const
{
    return nColumns_;
}

/**
  * @param fieldName [const QString &fieldName]
  * @returns int
  */
int TableModel::fieldColumn(const QString &fieldName) const
{
    return fields_.indexOf(fieldName);
}

/**
  * @param id [int]
  * @param fieldName [const QString &]
  * @returns QVariant
  */
QVariant TableModel::data(int id, const QString &fieldName) const
{
    if (!idLookup_.contains(id))
        return QVariant();

    return records_.at(idLookup_.value(id)).value(fieldName);
}

/**
  * @param id [int]
  * @param column [int]
  * @returns QVariant
  */
QVariant TableModel::data(int id, int column) const
{
    if (column < 0 || column >= nColumns_)
        return QVariant();

    if (!idLookup_.contains(id))
        return QVariant();

    return records_.at(idLookup_.value(id)).value(column);
}

/**
  *
  * @returns QStringList
  */
QStringList TableModel::fields() const
{
    return fields_;
}

/**
  * @param column [int]
  * @returns QString
  */
QString TableModel::friendlyFieldName(int column) const
{
    if (column < 0 || column >= nColumns_)
        return QString();

    return friendlyFieldNames_.at(column);
}

/**
  * Note: This does not return the total number of records in the database table! Rather it returns
  * the number of records in memory.
  *
  * @returns int
  */
int TableModel::rowCount() const
{
    return records_.count();
}

/**
  * Beginning with the first field and up to min(columnCount(), friendlyFieldNames.count()), sets
  * the friendly field names to those contained in friendlyFieldNames.
  *
  * Emits friendlyFieldNamesChanged if there is at least one friendly name to change.
  *
  * @param friendlyFieldNames [const QStringList &]
  */
void TableModel::setFriendlyFieldNames(const QStringList &friendlyFieldNames)
{
    int max = qMin(nColumns_, friendlyFieldNames.count());
    for (int i=0; i< max; ++i)
        friendlyFieldNames_[i] = friendlyFieldNames.at(i);

    if (max != 0)
        emit friendlyFieldNamesChanged(0, max-1);
}

/**
  * This method initializes the table model by providing an underlying AdocDataSource which provides
  * the actual data from tableName. Only fields will be acessible via the data model.
  *
  * If either tableName or fields is empty, all future load requests will be ignored (because there is
  * no source table or there are no fields requested).
  *
  * If fields is not empty and does not contain an id field, it will be prepended to the list of fields
  * to select. If fields is not empty and contains id, it will be moved to the front of the list of fields
  *
  * If any of the source parameters have changed, a sourceChanged signal will be emitted, all previously
  * data will be cleared and the reset signal emitted. The sourceChanged signal is useful for classes such
  * as RelatedTableModel which may modify any relations to this TableModel instance.
  *
  * @param adocDataSource [AdocDataSource *]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  */
void TableModel::setSource(AbstractAdocDataSource *adocDataSource, const QString &tableName, const QStringList &fields)
{
    // Make sure "id" is in the first position if it is not already there (only if fields is not-empty)
    QStringList newFields = normalizeFields(fields);

    // If nothing has changed regarding the source, simply return
    if (adocDataSource == adocDataSource_ && tableName == tableName_ && newFields == fields_)
        return;

    adocDataSource_ = adocDataSource;
    tableName_ = tableName;
    fields_ = newFields;
    friendlyFieldNames_ = newFields;            // Friendly names are by default the names of the fields
    nColumns_ = fields_.count();

    clear();

    emit sourceChanged(this);
}

/**
  * @returns QString
  */
QString TableModel::tableName() const
{
    return tableName_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * Releases all records stored in memory.
  */
void TableModel::clear()
{
    emit modelAboutToBeReset();
    records_.clear();
    idLookup_.clear();
    emit modelReset();
}

/**
  * Requests to load each record identified by ids via the associated data source. The specific fields that will
  * be fetched is determined by the fields_ member which is specified when initializing the TableModel via the
  * setSource method. Optionally, this load request may be tagged with the tag parameter. The tag is not utilized
  * internally, but merely passed along for convenience.
  *
  * On success, the loadDone() signal will be emitted. Just because a loadDone signal has been emitted does not
  * indicate that the records identified by ids exist in the data source. Rather, it means any matched records
  * have been properly requesterd and loaded into memory without triggering an error condition.
  *
  * A number of conditions may result in a loadError signal being triggered:
  * o Invalid AdocDataSource
  * o Empty source table
  * o Downstream AdocDataSource error
  *
  * If no fields have been defined (see @setSource) or ids is empty, then no request will be made to the data
  * source and a loadDone signal will be emitted immediately. This depends upon adocDataSource_ being opened prior
  * to calling this method.
  *
  * @param ids [const QList<int> &]
  * @param tag [int]
  * @see setSource()
  */
void TableModel::load(const QList<int> &ids, int tag)
{
    if (!adocDataSource_)
    {
        emit loadError("Invalid data source", tag);
        return;
    }

    if (tableName_.isEmpty())
    {
        emit loadError("No table has been defined", tag);
        return;
    }

    if (ids.isEmpty() || fields_.isEmpty())
    {
        emit loadDone(tag);
        return;
    }

    // Check for new ids not already loaded
    QList<QVariant> newIds;
    foreach (int id, ids)
        if (!idLookup_.contains(id))
            newIds << id;

    if (newIds.isEmpty())
    {
        emit loadDone(tag);
        return;
    }

    // Build the request SQL
    QString fieldage = fields_.join(",");
    QString selectSql = QString("SELECT %1 FROM %2 WHERE id = ?").arg(fieldage, tableName_);

    // A response will be returned via either the __selectReady or __selectError slots
    adocDataSource_->select(selectSql, newIds, this, "__selectReady", "__selectError", tag);
}

/**
  * Technically, foreignKey does not have to be a foreign key; rather, it can be any field that
  * supports searching via integers. Also, foreignKey does not necessarily need to exist in the
  * set of fields modeled here, but it must be present in the database table to avoid triggering
  * a database error.
  *
  * All of these requests always hit the database because a foreign key does not uniquely identify
  * a single row. Thus, it is not possible to know if all rows have been added into memory apart
  * from application specific programming (currently not implemented).
  *
  * @param foreignKey [const QString &]
  * @param fkIds [const QList<int> &]
  * @param tag
  */
void TableModel::loadWithForeignKey(const QString &foreignKey, const QList<int> &fkIds, int tag)
{
    if (!adocDataSource_)
    {
        emit loadError("Invalid data source", tag);
        return;
    }

    if (tableName_.isEmpty())
    {
        emit loadError("No table has been defined", tag);
        return;
    }

    if (fkIds.isEmpty() || fields_.isEmpty())
    {
        emit loadDone(tag);
        return;
    }

    // Build the request SQL
    QString fieldage = fields_.join(",");
    QString selectSql = QString("SELECT %1 FROM %2 WHERE %3 = ?").arg(fieldage, tableName_, foreignKey);

    // Convert the list of integers to QVariants because the AdocDataSource only accepts QVariantLists
    QList<QVariant> variantFkIds;
    foreach (int fkId, fkIds)
        variantFkIds << fkId;

    // A response will be returned via either the __selectReady or __selectError slots
    adocDataSource_->select(selectSql, variantFkIds, this, "__selectReady", "__selectError", tag);
}

/**
  * Will immediately return false if the row identified by id is not loaded in memory (via load). Because
  * all updates of this nature consist of changing the value of one field, the database update will most
  * likely complete in a trivial amount of time (although I suppose that locks could effectively delay
  * this). Regardless, to provide a rapid update mechanism, the field is immediately updated in memory
  * if possible and triggers a database update request. Should that fail, the value in memory will be reset
  * to its previous value and another dataChanged signal emitted.
  *
  * Due to the pivotal nature of the id field as the primary key and its role in defining relationships, it
  * is not possible to update the id field with this method.
  *
  * @param id [int]
  * @param fieldName [const QString &]
  * @param newValue [const QVariant &]
  * @returns bool
  */
bool TableModel::setData(int id, const QString &fieldName, const QVariant &newValue)
{
    // Translate fieldName into its equivalent column number
    int columnIndex = fields_.indexOf(fieldName);
    if (columnIndex == -1)
        return false;

    // And return the result specified by the column version of setData
    return setData(id, columnIndex, newValue);
}

/**
  * Will immediately return false if the row identified by id is not loaded in memory (via load). Because
  * all updates of this nature consist of changing the value of one field, the database update will most
  * likely complete in a trivial amount of time (although I suppose that locks could effectively delay
  * this). Regardless, to provide a rapid update mechanism, the field is immediately updated in memory
  * if possible and triggers a database update request. Should that fail, the value in memory will be reset
  * to its previous value and another dataChanged signal emitted.
  *
  * Due to the pivotal nature of the id field as the primary key and its role in defining relationships, it
  * is not possible to update the id field with this method.
  *
  * @param id [int]
  * @param column [int]
  * @param newValue [const QVariant &]
  * @returns bool
  */
bool TableModel::setData(int id, int column, const QVariant &newValue)
{
    if (!adocDataSource_)
        return false;

    if (!idLookup_.contains(id))
        return false;

    if (column < 0 || column >= nColumns_)
        return false;

    if (fields_.at(column) == "id")
        return false;

    int rowIndex = idLookup_.value(id);
    int tag = TagGenerator::nextValue();

    // Save old value in case updating the data source fails and it is necessary to revert to the previous value
    oldCellValues_.append(CellData(tag, id, column, records_.at(rowIndex).value(column)));

    // Update the value in memory and notify connected components that its value has changed
    records_[rowIndex].setValue(column, newValue);
    emit dataChanged(id, column);

    // Update the data source
    adocDataSource_->update(records_[rowIndex], tableName_, QStringList() << fields_.at(column), this, "__setDataDone", "__setDataError", tag);

    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * This private slot is called in response to the select query issued by the load method and contains
  * the results of new sequences to load.
  *
  * @param dataRows [const QList<DataRow> &]
  * @param tag [int]
  * @see load()
  */
void TableModel::__selectReady(const QList<DataRow> &dataRows, int tag)
{
    appendRecords(dataRows);

    emit loadDone(tag);
}

/**
  * This private slot is called in response to the select query issued by the load method and is the
  * result of an error that occurred within the data source.
  *
  * @param error [const QString &]
  * @param tag [int]
  * @see load()
  */
void TableModel::__selectError(const QString &error, int tag)
{
    emit loadError(error, tag);
}

/**
  * Because the dataChanged event has already been triggered after updating the item in memory (see
  * setData), it is not necessary to do that here. We simply remove the old cell value tagged with
  * tag from the oldCellValues_ structure.
  *
  * @param dataRow [const DataRow &]
  * @param rowAffected [bool]
  * @param tag [int]
  * @see setData()
  */
void TableModel::__setDataDone(const DataRow &/* dataRow */, bool /* rowAffected */, int tag)
{
    // Identify the old cell where tag_ == tag
    int index = -1;
    for (int i=0, z= oldCellValues_.count(); i<z; ++i)
    {
        if (oldCellValues_.at(i).tag_ == tag)
        {
            index = i;
            break;
        }
    }

    ASSERT(index != -1);
    if (index == -1)        // Release mode guard
        return;

    oldCellValues_.takeAt(index);
}

/**
  * Updating the data source failed, restore the old value and emit setDataError.
  *
  * @param error [const QString &]
  * @param tag [int]
  * @see setData()
  */
void TableModel::__setDataError(const QString &error, int tag)
{
    // Identify the old cell where tag_ == tag
    int index = -1;
    for (int i=0, z= oldCellValues_.count(); i<z; ++i)
    {
        if (oldCellValues_.at(i).tag_ == tag)
        {
            index = i;
            break;
        }
    }

    ASSERT(index != -1);
    if (index == -1)        // Release mode guard
        return;

    CellData oldCellData = oldCellValues_.takeAt(index);

    ASSERT(idLookup_.contains(oldCellData.id_));
    if (!idLookup_.contains(oldCellData.id_)) // Release mode guard
        return;

    records_[idLookup_.value(oldCellData.id_)].setValue(oldCellData.column_, oldCellData.value_);
    emit dataChanged(oldCellData.id_, oldCellData.column_);

    emit setDataError(oldCellData.id_, oldCellData.column_, error);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Inserts all dataRows that have not already been loaded into memory (as determined by its id)
  * into the records_ data structure and updates the lookup table.
  *
  * @param dataRows [const QList<DataRow> &]
  * @see __selectReady()
  */
void TableModel::appendRecords(const QList<DataRow> &dataRows)
{
    foreach (const DataRow &dataRow, dataRows)
    {
        ASSERT(dataRow.id_.isValid());
        if (!dataRow.id_.isValid())     // Release mode guard
            continue;

        if (idLookup_.contains(dataRow.id_.toInt()))
            continue;

        records_ << dataRow;
        idLookup_.insert(dataRow.id_.toInt(), records_.count() - 1);
    }
}

/**
  * If fields is not empty and contains an element labeled, "id", moves it to the first position if it is not
  * already at that position. If "id" does not exist in fields, it is added at the first index.
  *
  * @param fields [const QStringList &]
  * @returns QStringList
  */
QStringList TableModel::normalizeFields(const QStringList &fields) const
{
    if (fields.isEmpty())
        return fields;

    QStringList newFields = fields;

    // Add the id to the fields if a non-empty list of fields has been provided
    int idPosition = newFields.indexOf("id");
    if (idPosition == -1)
        newFields.prepend("id");
    else if (idPosition != 0)
        newFields.move(idPosition, 0);

    return newFields;
}
