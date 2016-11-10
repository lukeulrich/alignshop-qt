/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "RelatedTableModel.h"

#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "TagGenerator.h"

#include "global.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
RelatedTableModel::RelatedTableModel(QObject *parent) : TableModel(parent)
{
}

/**
  * Free up any leftover RelatedLoadRequests
  */
RelatedTableModel::~RelatedTableModel()
{
    qDeleteAll(relationRequests_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  *
  *
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  * @returns bool
  */
bool RelatedTableModel::hasRelation(Relation type, TableModel *targetTableModel) const
{
    ASSERT(targetTableModel);

    switch (type)
    {
    case eRelationBelongsTo:
        return belongsTo_.contains(targetTableModel);
    case eRelationHasMany:
        return hasMany_.contains(targetTableModel);

    default:
        return false;
    }
}

/**
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  * @returns QString
  */
QString RelatedTableModel::relatedField(Relation type, TableModel *targetTableModel) const
{
    ASSERT(targetTableModel);

    switch (type)
    {
    case eRelationBelongsTo:
        if (belongsTo_.contains(targetTableModel))
            return belongsTo_.value(targetTableModel).relatedField_;
        break;

    case eRelationHasMany:
        if (hasMany_.contains(targetTableModel))
            return hasMany_.value(targetTableModel).relatedField_;
        break;
    }

    return QString();
}


/**
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  */
void RelatedTableModel::removeRelation(Relation type, TableModel *targetTableModel)
{
    ASSERT(targetTableModel);

    switch (type)
    {
    case eRelationBelongsTo:
        belongsTo_.remove(targetTableModel);
        break;
    case eRelationHasMany:
        hasMany_.remove(targetTableModel);
        break;
    }

    // Remove any sourceChagned signal
    QObject::disconnect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));
}

/**
  *
  *
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  * @returns bool
  */
bool RelatedTableModel::isAutoLoadedRelation(Relation type, TableModel *targetTableModel) const
{
    ASSERT(targetTableModel);

    switch (type)
    {
    case eRelationBelongsTo:
        if (belongsTo_.contains(targetTableModel))
            return belongsTo_.value(targetTableModel).autoLoad_;
        break;
    case eRelationHasMany:
        if (hasMany_.contains(targetTableModel))
            return hasMany_.value(targetTableModel).autoLoad_;
        break;
    }

    return false;
}

/**
  * If the relationship does not exist, it does nothing. Also, connects/disconnects the loadDone and
  * loadError signals to the private slots, __relationLoadDone and __relationLoadError, depending on
  * the value of autoload.
  *
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  * @param autoLoad [bool]
  */
void RelatedTableModel::setAutoLoadRelation(Relation type, TableModel *targetTableModel, bool autoLoad)
{
    ASSERT(targetTableModel);

    switch (type)
    {
    case eRelationBelongsTo:
        if (!belongsTo_.contains(targetTableModel))
            return;

        belongsTo_[targetTableModel].autoLoad_ = autoLoad;
        break;
    case eRelationHasMany:
        if (!hasMany_.contains(targetTableModel))
            return;

        hasMany_[targetTableModel].autoLoad_ = autoLoad;
        break;
    }

    // Connect the signals for when the relation finishes loading
    if (autoLoad)
    {
        QObject::connect(targetTableModel, SIGNAL(loadDone(int)), this, SLOT(__relationLoadDone(int)));
        QObject::connect(targetTableModel, SIGNAL(loadError(QString,int)), this, SLOT(__relationLoadError(QString,int)));
    }
    else
    {
        QObject::disconnect(targetTableModel, SIGNAL(loadDone(int)), this, SLOT(__relationLoadDone(int)));
        QObject::disconnect(targetTableModel, SIGNAL(loadError(QString,int)), this, SLOT(__relationLoadError(QString,int)));
    }
}

/**
  * Defines a relationship of type between this model and targetTableModel using relatedField. If a
  * relationship of this type already exists, it is overwritten. For this operation to succeed, both
  * this table and targetTable model must have been initialized with setSource to have a non-empty table
  * and at least one field. Moreover, relatedField must be a valid entity and depending on type, has
  * the following constraints:
  * <<Type>>            <<Constraint>>
  * eRelationBelongsTo: relatedField must be present in this model for success
  * eRelationHasMany:   relatedField must exist within targetTableModel
  * eRelationHasOne:    relatedField must exist within targetTableModel (not implemented)
  *
  * If autoLoad is true, automatically load related records.
  *
  * Other notes:
  * >> Self relations are not permitted
  * >> To prevent dangling relations resulting from an incompatible setSource operation, the targetTableModel's
  *    sourceChanged() signals is connected to the private slot, __relationSourceChanged, which ensures that
  *    all relationships to this model are removed.
  *
  * Returns true if the relationship was defined or false otherwise. If false will be returned, any pre-exsting
  * valid relationship is preserved.
  *
  * @param type [Relation]
  * @param targetTableModel [TableModel *]
  * @param relatedField [const QString &]
  * @param autoLoad [bool]
  * @returns bool
  */
bool RelatedTableModel::setRelation(Relation type, TableModel *targetTableModel, const QString &relatedField, bool autoLoad)
{
    ASSERT(targetTableModel);

    if (!targetTableModel || relatedField.isEmpty() || targetTableModel == this)
        return false;

    // Additional contraints:
    if (tableName().isEmpty() || targetTableModel->tableName().isEmpty()
        || fields().isEmpty() || targetTableModel->fields().isEmpty())
    {
        return false;
    }

    if (type == eRelationBelongsTo)
    {
        // relatedField should be present in our fields
        if (!fields().contains(relatedField))
            return false;

        if (!belongsTo_.contains(targetTableModel))
            QObject::connect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));

        // Add to the belongsTo relationship hash, overwriting if already present
        belongsTo_.insert(targetTableModel, RelationInfo(relatedField));
        if (autoLoad)
            setAutoLoadRelation(type, targetTableModel, autoLoad);

        return true;
    }
    else if (type == eRelationHasMany)
    {
        // We should have at least one field defined for this model and that should be its id
        if (!fields().contains("id"))
            return false;

        // relatedField should be present in targetTableModel
        if (!targetTableModel->fields().contains(relatedField))
            return false;

        // Connect the sourceChanged signal for this targetTableModel if it is a new relationship
        // This prevents making the connection multiple times (which would call our slot multiple
        // times per source change).
        if (!hasMany_.contains(targetTableModel))
            QObject::connect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));

        // Add to the hasMany relationship hash, overwriting if already present
        hasMany_.insert(targetTableModel, RelationInfo(relatedField));
        if (autoLoad)
            setAutoLoadRelation(type, targetTableModel, autoLoad);

        return true;
    }

    return false;
}

/**
  * Whenever any of the source parameters change, remove all relationships and emit the sourceChanged signal.
  * To permit setting this model to an empty state, no constraints are placed on the value for adocDataSource,
  * tableName, or fields.
  *
  * @param adocDataSource [AdocDataSource *]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  */
void RelatedTableModel::setSource(AbstractAdocDataSource *adocDataSource, const QString &tableName, const QStringList &fields)
{
    // Flag to remember if the source has changed
    if (adocDataSource != adocDataSource_ || tableName != TableModel::tableName() || normalizeFields(fields) != TableModel::fields())
        clearRelations();

    TableModel::setSource(adocDataSource, tableName, fields);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Removes all belongsTo and hasMany relationships. Additionally, it is necessary to disconnect all
  * sourceChanged connections.
  */
void RelatedTableModel::clearRelations()
{
    foreach(TableModel *targetTableModel, belongsTo_.keys())
        QObject::disconnect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));
    belongsTo_.clear();

    foreach(TableModel *targetTableModel, hasMany_.keys())
        QObject::disconnect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));
    hasMany_.clear();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * Only relationships that are set to autoload will trigger associative loads. Before loading related
  * records, we first append the rows to this models internal data store. Should a downstream error
  * occur (e.g. fetching related records), this table will still have its data.
  *
  * @param dataRows [const QList<DataRow> &]
  * @param tag [int]
  */
void RelatedTableModel::__selectReady(const QList<DataRow> &dataRows, int tag)
{
    // Called when the records for this table have been loaded successfully. Go ahead
    // and add these records to our internal data store
    appendRecords(dataRows);

    // A. Build list of belongs to and hasMany tables that have autoload = true
    QVector<TableModel *> autoLoadBelongsTo;
    QHash<TableModel *, RelationInfo>::iterator i = belongsTo_.begin();
    while (i != belongsTo_.end())
    {
        if (i.value().autoLoad_)
            autoLoadBelongsTo << i.key();

        ++i;
    }

    QVector<TableModel *> autoLoadHasMany;
    i = hasMany_.begin();
    while (i != hasMany_.end())
    {
        if (i.value().autoLoad_)
            autoLoadHasMany << i.key();

        ++i;
    }

    // If no belongsTo relationships should be autoLoaded, simply emit the loadDone signal and return
    if (autoLoadBelongsTo.isEmpty() && autoLoadHasMany.isEmpty())
    {
        emit loadDone(tag);
        return;
    }

    // If the following load requests are synchronously executed, they will all be completed before returning control
    // back to end of this code block. In such a case, the relatedLoadRequest will have been freed by
    // processRelatedLoadRequest which will have been called from either __relationLoadDone or __relationLoadError.
    // Consequently, it is not safe to continue using the relatedLoadRequest beyond this code block (and actually
    // beyond the foreach (TableModel *belongsToModel, autoLoadBelongsTo) loop.
    //
    // Another vital point is that the nOutstanding requests member variable be assigned the total before the foreach
    // loop and load requests. This is because the relatedLoadRequest pointer is freed once the number of outstanding
    // requests is decremented to zero. If we simply increment this value each time in the foreach loop, it will in
    // turn be immediately decremented back to zero if the load command is synchronously performed. At that point it
    // will be freed and cause a crash if there was another belongsTo model to be auto loaded.
    //
    // This whole section has been put in a code block to isolate the potential use of the relatedLoadRequest pointer.
    {
        // Prepare related load request with the master tag
        RelatedLoadRequest *relatedLoadRequest = new RelatedLoadRequest(tag);
        relatedLoadRequest->nOutstanding_ = autoLoadBelongsTo.count();
        foreach (TableModel *belongsToModel, autoLoadBelongsTo)
        {
            // This request should be auto loaded
            int slaveTag = TagGenerator::nextValue();

            relatedLoadRequest->requests_.insert(slaveTag, belongsToModel);

            // Tack this slave onto the list
            relationRequests_.insert(slaveTag, relatedLoadRequest);

            // Kick off the request
            belongsToModel->load(uniqueIntList(dataRows, belongsTo_.value(belongsToModel).relatedField_), slaveTag);
        }

        // Disable the use of this pointer from here on
        relatedLoadRequest = 0;
    }

    // B. Check for hasMany
    {
        // Prepare related load request with the master tag
        RelatedLoadRequest *relatedLoadRequest = new RelatedLoadRequest(tag);
        relatedLoadRequest->nOutstanding_ = autoLoadHasMany.count();
        foreach (TableModel *targetTableModel, autoLoadHasMany)
        {
            // This request should be auto loaded
            int slaveTag = TagGenerator::nextValue();

            relatedLoadRequest->requests_.insert(slaveTag, targetTableModel);

            // Tack this slave onto the list
            relationRequests_.insert(slaveTag, relatedLoadRequest);

            // Kick off the request
            targetTableModel->loadWithForeignKey(hasMany_.value(targetTableModel).relatedField_, uniqueIntList(dataRows, "id"), slaveTag);
        }

        // Disable the use of this pointer from here on
        relatedLoadRequest = 0;
    }

    // C. Check for hasOne (not implemented)
}

/**
  * setAutoLoadRelation hooks up the loadError signal of a related table to this private slot.
  * Consequently, all loadErrors regardless of the originating request will call this method. Since
  * we are only interested in load requests spawned by calling load on this object (RelatedTableModel)
  * utilize the tag to identify if it originated from the __selectReady() method.
  *
  * @param error [const QString &]
  * @param tag [int]
  * @see setAutoLoadRelation(), __selectReady()
  */
void RelatedTableModel::__relationLoadError(const QString &error, int tag)
{
    // Only concerned about relation requests originating from this class and more specifically from
    // the __selectReady method. These are all contained within the relationRequests_ data member.
    if (!relationRequests_.contains(tag))
        return;

    // Remove from the list of child tags mapping to the parent relation request
    RelatedLoadRequest *relatedLoadRequest = relationRequests_.take(tag);
    ASSERT(relatedLoadRequest);

    // Mark this request as failed and save its error information
    relatedLoadRequest->failed_.insert(tag, error);

    processRelatedLoadRequest(relatedLoadRequest);
}

/**
  * setAutoLoadRelation connects the loadDone signal of a related table to this private slot. Consequently,
  * all loadDone events regardless of the originating request will call this method. Since we are only
  * interested in load requests spawned by calling load on this object (RelatedTableModel), utilize the tag
  * to identify if it originated from the __selectReady() method.
  *
  * @param tag [int]
  * @see setAutoLoadRelation(), __selectReady()
  */
void RelatedTableModel::__relationLoadDone(int tag)
{
    // Only concerned about relation requests originating from this class and more specifically from
    // the __selectReady method. These are all contained within the relationRequests_ data member.
    if (!relationRequests_.contains(tag))
        return;

    // Remove from the list of child tags mapping to the parent relation request
    RelatedLoadRequest *relatedLoadRequest = relationRequests_.take(tag);
    ASSERT(relatedLoadRequest);

    // Mark this request as failed and save its error information
    relatedLoadRequest->successful_.insert(tag);

    processRelatedLoadRequest(relatedLoadRequest);
}

/**
  * This slot is called whenever a target table model has had a sourceChanged(TableModel *) signal. Removes
  * all relationships to targetTableModel from this class.
  *
  * @param targetTableModel [TableModel *]
  */
void RelatedTableModel::__relationSourceChanged(TableModel *targetTableModel)
{
    belongsTo_.remove(targetTableModel);
    hasMany_.remove(targetTableModel);
    QObject::disconnect(targetTableModel, SIGNAL(sourceChanged(TableModel *)), this, SLOT(__relationSourceChanged(TableModel *)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private method
/**
  * Utility method for dealing with a RelatedLoadRequest after one of its child requests has been
  * tagged as successful or failed (see __relationLoadDone(), __relationLoadError()).
  *
  * Actions taken:
  * o Reduce the number of outstanding child requests by one
  * o If there are no more outstanding child requests
  *   -> If no failed loads, emit loadDone with the parent tag
  *   -> If at least one load failed, emit partialLoadError with the parent tag
  * o Free up the RelatedLoadRequest
  *
  * @param relatedLoadRequest [RelatedLoadRequest *]
  */
void RelatedTableModel::processRelatedLoadRequest(RelatedLoadRequest *relatedLoadRequest)
{
    // Decrease the number of outstanding requests
    ASSERT(relatedLoadRequest->nOutstanding_ > 0);
    --relatedLoadRequest->nOutstanding_;

    // Check if this is the last request
    if (relatedLoadRequest->nOutstanding_ > 0)  // No
        return;

    if (relatedLoadRequest->failed_.isEmpty())
    {
        // All load requests were successful
        emit loadDone(relatedLoadRequest->parentTag_);
    }
    else    // There was at least one table that failed with a load error
    {
        // Construct a sorted list of all the related tables that failed
        QStringList failedTables;
        QList<int> childTags = relatedLoadRequest->failed_.keys();
        foreach (int childTag, childTags)
        {
            ASSERT(relatedLoadRequest->requests_.contains(childTag));
            failedTables << relatedLoadRequest->requests_.value(childTag)->tableName();
        }

        emit partialLoadError(QString("There was an error loading data from the following related tables: %1").arg(failedTables.join(", ")), relatedLoadRequest->parentTag_);
    }

    // Free up the memory used by this request
    delete relatedLoadRequest;
    relatedLoadRequest = 0;
}

/**
  * Only adds values for those dataRows that actually contain fieldName and can be converted to integers.
  *
  * @param dataRows [const QList<DataRow> &dataRows
  * @param fieldName [const QString &]
  * @returns QList<int>
  * @see __selectReady()
  */
QList<int> RelatedTableModel::uniqueIntList(const QList<DataRow> &dataRows, const QString &fieldName) const
{
    if (fieldName.isEmpty())
        return QList<int>();

    QSet<int> uniqueInts;
    foreach (const DataRow &dataRow, dataRows)
    {
        if (dataRow.contains(fieldName)
            && dataRow.value(fieldName).canConvert(QVariant::Int))
        {
            uniqueInts.insert(dataRow.value(fieldName).toInt());
        }
    }

    return uniqueInts.toList();
}
