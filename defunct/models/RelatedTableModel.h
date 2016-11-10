/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RELATEDTABLEMODEL_H
#define RELATEDTABLEMODEL_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QString>

#include "TableModel.h"

/**
  * RelatedTableModel extends TableModel with the ability to define relationships between
  * tables.
  *
  * In SQL, there are three primary types of relationships:
  * 1) Table A "has one" Table B; 1:0,1 relationship in which Table B has a foreign key that
  *    maps to Table A's primary key.
  * 2) Table A "has many" Table B; 1:0,N relationship in which zero or more records in Table B
  *    map to Table A via a specific foreign key.
  * 3) Table B "belongs to" Table A: again a 1:1 relationship, but this time from the opposite
  *    direction. In other words, one record of Table B relates to one record of Table A.
  *
  * Relationships may be added or removed using the addRelation() and removeRelation() methods.
  * To check if a relationship has been defined, use the hasRelation() method.
  *
  * Relationships ultimately tie together tablular information and thus it is frequently desirable
  * that when one record in a table is loaded, other related records are also loaded. This may be
  * achieved by calling setAutoLoadRelation(). Depending on the specific relationship type, the
  * order of loading may vary. For instance, if Table X belongs to Table Y, the corresponding
  * records in Table Y should be loaded prior to those in Table X; however, this is not directly
  * possible without first knowing the foreign key to Y that is present in a given record of X.
  * On the other hand, if Table Y has one of Table X, then once a Y record has been loaded, it
  * can cascade the request to Table X.
  *
  * A real example is that of the Seq / Subseq relationship. One Seq "has many" Subseqs; however,
  * the data tree only contains subseq identifiers. When a subseq is loaded, it is desirable to
  * have the Seq level data loaded before the user requests any data. Yet it is impossible to know
  * which Seq we should load until after each Subseq record is loaded. Then the Seq_id foreign key
  * may be used to fetch the Seq record. RelatedTableModel automatically handles this detail, by
  * waiting to notify that a Subseq has been loaded until after its parent Seq has been loaded.
  *
  * Whenever setSource is called for this or a linked targetTableModel, it is necessary to remove
  * those relationships for which fields no longer exist. Thus, whenever a relationship is added/
  * removed, connections to the sourceChanged signal (emitted whenever setSource is performed) provide for
  * keeping relationships in sync with their source targetTables. While it is possible to manage
  * the connections with some signal/slot wiring, it is much easier to simply remove all relevant
  * relations whenever the source has changed. Thus, in its current implementation, any time the
  * source information has changed, all relevant relations are removed - even if it is possible
  * to keep them with the new configuration.
  *
  * Currently, only the "belongs to" and "has many" relationships are implemented to specifically
  * address the above scenario(s).
  *
  * >> OPTIMIZATION: Somehow index the rows that have been loaded via a foreign key and do not
  *                  request those again unless otherwise requested.
  */
class RelatedTableModel : public TableModel
{
    Q_OBJECT

public:
    enum Relation {
        eRelationBelongsTo,
        eRelationHasMany
//        eRelationHasOne,          <--- To implement on monday!
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    RelatedTableModel(QObject *parent = 0);                                 //!< Trivial constructor
    ~RelatedTableModel();                                                   //!< Basic destructor for releasing any unfreed memory

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool hasRelation(Relation type, TableModel *targetTableModel) const;    //!< Returns true if relationship defined by type and targetTableModel has been defined; false otherwise
    //! Returns the related field for a given relationship type and targetTableModel; an empty QString is returned if the relationship does not exist
    QString relatedField(Relation type, TableModel *targetTableModel) const;
    void removeRelation(Relation type, TableModel *targetTableModel);       //!< Removes any relationship defined by type and targetTableModel
    //!< Returns true if this model has a relationship defined by type and targetTableModel and is set to auto fetch those related records on load requests; false otherwise
    bool isAutoLoadedRelation(Relation type, TableModel *targetTableModel) const;
    //!< Sets load requests to automatically fetch related records defined by type and targetTabelModel to autoLoad; does nothing if the relationship has not been previously defined
    void setAutoLoadRelation(Relation type, TableModel *targetTableModel, bool autoLoad);
    //!< Adds the relation, type, for targetTableModel and relatedField to this TableModel with autoLoad and returns true on success; false otherwise
    bool setRelation(Relation type, TableModel *targetTableModel, const QString &relatedField, bool autoLoad = false);

    //! Reimplemented public function that clears all previously defined relationships before calling TableModel::setSource
    virtual void setSource(AbstractAdocDataSource *adocDataSource, const QString &tableName, const QStringList &fields);

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void partialLoadError(const QString &error, int tag);                   //!< Emitted when one or more related tables failed to load properly

public slots:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void clearRelations();                                                  //!< Removes all defined relationships

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    virtual void __selectReady(const QList<DataRow> &dataRows, int tag);    //!< Override the TableModel::__selectReady signal to provide mechanism for loading related records

    void __relationLoadError(const QString &error, int tag);                //!< Called when a related table has encountered an error while attempting to load the data associated with tag
    void __relationLoadDone(int tag);                                       //!< Called when a related table has successfully loaded the data specified by tag
    void __relationSourceChanged(TableModel *targetTableModel);             //!< Called when targetTableModel has changed its source parameters

private:
    // ------------------------------------------------------------------------------------------------
    // Private helper structs
    /**
      * RelationInfo simply aggregates a specific database field for relationships and whether this
      * relation should be auto loaded. Because we use this struct as part of a QHash, it must be
      * possible to construct an instance without any parameters. Thus, relatedField and autoLoad
      * are given default values (which are not strictly utilized).
      */
    struct RelationInfo
    {
        QString relatedField_;
        bool autoLoad_;

        RelationInfo(QString relatedField = QString(), bool autoLoad = false) : relatedField_(relatedField), autoLoad_(autoLoad)    {}
    };

    /**
      * RelatedLoadRequest simply aggregates a master tag identifier which a set of related child requests.
      */
    struct RelatedLoadRequest
    {
        int parentTag_;                             //!< Source load request tag grouping all related child requests
        int nOutstanding_;                          //!< Number of outstanding child requests
        QHash<int, TableModel *> requests_;         //! {child tag} -> {TableModel *}; currently belongsTo since that is the only one implemented so far
        QSet<int> successful_;                      //!< Set of child tag requests that have been successful
        QHash<int, QString> failed_;                //!< Those requests that have failed; {child tag} -> {error message}

        RelatedLoadRequest(int parentTag = 0) : parentTag_(parentTag) { nOutstanding_ = 0; }
    };

    // ------------------------------------------------------------------------------------------------
    // Private methods
    void processRelatedLoadRequest(RelatedLoadRequest *relatedLoadRequest); //!<
    //! Returns a list of unique identifiers from the value of fieldName from each dataRows
    QList<int> uniqueIntList(const QList<DataRow> &dataRows, const QString &fieldName) const;

    // ------------------------------------------------------------------------------------------------
    // Private members
    QHash<TableModel *, RelationInfo> belongsTo_;           //!< Contains all belongsTo relationships: (this->tableName(), QString) belongsTo (TableModel->tableName(), "id")
    QHash<TableModel *, RelationInfo> hasMany_;             //!< Contains all hasMany relationships: (this->tableName(), "id") hasMany (TableModel->tableName(), QString)

    QHash<int, RelatedLoadRequest *> relationRequests_;     //!< Maps a related child tag to its source request object

    //    QHash<TableModel *, QString> hasOne_;           //!< Contains all hasOne relationships: (this->tableName(), "id") hasOne (TableModel->tableName(), QString)
};

#endif // RELATEDTABLEMODEL_H
