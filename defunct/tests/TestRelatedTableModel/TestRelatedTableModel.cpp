/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "SynchronousAdocDataSource.h"
#include "AdocTreeNode.h"
#include "DataRow.h"

#include "TableModel.h"
#include "RelatedTableModel.h"

#include <QtDebug>

class TestRelatedTableModel : public QObject
{
    Q_OBJECT

public:
    TestRelatedTableModel()
    {
        qRegisterMetaType<QList<DataRow> >();
        qRegisterMetaType<TableModel *>();
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setRelation();     // Also tests hasRelation, relatedField
    void removeRelation();  // Also tests hasRelation
    void clearRelations();
    void setAutoLoadRelations();    // Also tests isAutoLoadedRelation
    void setSource();

    void loadSimpleBelongsTo();
    void loadMultipleBelongsTo();
    void loadComplexBelongsTo();

    void loadHasMany();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestRelatedTableModel::setRelation()
{
    // ------------------------------------------------------------------------
    // Suite: belongsTo
    RelatedTableModel x;
    TableModel y;

    // Test: x (uninitialized) to itself and y (uninitialized)
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &x, "") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &x, "amino_seq_id") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id") == false);

    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &x) == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);

    // Setup - valid y
    y.setSource(0, constants::kTableAminoSeqs, QStringList() << "id" << "protein");

    // Test: x (uninitialized) and y
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "protein") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);

    // Setup - valid x
    x.setSource(0, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "digest");

    // Test: x to x
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &x, "") == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &x, "amino_seq_id") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &x) == false);

    // Test: x to y
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "missing_field") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y));
    QVERIFY(QObject::disconnect(&y, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))));
    QCOMPARE(x.relatedField(RelatedTableModel::eRelationBelongsTo, &y), QString("amino_seq_id"));

    // Test: invalid relationship request should not remove prior valid relationship
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "missing_field") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y));
    QCOMPARE(x.relatedField(RelatedTableModel::eRelationBelongsTo, &y), QString("amino_seq_id"));

    // Test: x to invalid y2
    TableModel y2;
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id") == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Test: setRelation with same parameters multiple times, should not have the signal duplicated. In other
    //       words, the signal should only be hooked up once.
    y2.setSource(0, constants::kTableAminoSeqs, QStringList() << "id" << "protein");
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id");
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id");
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id");
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))));
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: default autoload should be false
    x.clearRelations();
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id");
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Test: explicity set autoload
    x.clearRelations();
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id", true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2));
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))));
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);

    x.clearRelations();
    x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "amino_seq_id", false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // ------------------------------------------------------------------------
    // Suite: hasMany
    RelatedTableModel l;
    TableModel m;

    // Test: l (uninitialized) to itself and m (uninitialized)
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &l, "") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &l, "amino_seq_id") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "amino_seq_id") == false);

    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &l) == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m) == false);

    // Setup - valid m
    m.setSource(0, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "start" << "stop");

    // Test: l (uninitialized) and m
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "amino_seq_id") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "protein") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m) == false);

    // Setup - valid l
    l.setSource(0, constants::kTableAminoSeqs, QStringList() << "id" << "protein");

    // Test: l to l
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &l, "") == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &l, "amino_seq_id") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &l) == false);

    // Test: l to m
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m) == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "missing_field") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m) == false);
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "amino_seq_id"));
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m));
    QVERIFY(QObject::disconnect(&m, SIGNAL(sourceChanged(TableModel *)), &l, SLOT(__relationSourceChanged(TableModel *))));
    QCOMPARE(l.relatedField(RelatedTableModel::eRelationHasMany, &m), QString("amino_seq_id"));

    // Test: invalid relationship request should not remove prior valid relationship
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m, "missing_field") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m));
    QCOMPARE(l.relatedField(RelatedTableModel::eRelationHasMany, &m), QString("amino_seq_id"));

    // Test: l to invalid m2
    TableModel m2;
    QVERIFY(l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id") == false);
    QVERIFY(l.hasRelation(RelatedTableModel::eRelationHasMany, &m2) == false);

    // Test: setRelation with same parameters multiple times, should not have the signal duplicated. In other
    //       words, the signal should only be hooked up once.
    m2.setSource(0, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "start" << "stop");
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id");
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id");
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id");
    QVERIFY(QObject::disconnect(&m2, SIGNAL(sourceChanged(TableModel *)), &l, SLOT(__relationSourceChanged(TableModel *))));
    QVERIFY(QObject::disconnect(&m2, SIGNAL(sourceChanged(TableModel *)), &l, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: default autoload should be false
    l.clearRelations();
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id");
    QVERIFY(l.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y2) == false);

    // Test: explicity set autoload
    l.clearRelations();
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id", true);
    QVERIFY(l.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &m2));
    QVERIFY(QObject::disconnect(&m2, SIGNAL(sourceChanged(TableModel *)), &l, SLOT(__relationSourceChanged(TableModel *))));
    QVERIFY(QObject::disconnect(&m2, SIGNAL(sourceChanged(TableModel *)), &l, SLOT(__relationSourceChanged(TableModel *))) == false);

    l.clearRelations();
    l.setRelation(RelatedTableModel::eRelationHasMany, &m2, "amino_seq_id", false);
    QVERIFY(l.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &m2) == false);
}

void TestRelatedTableModel::removeRelation()
{
    RelatedTableModel x;
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    TableModel y, y2;

    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "digest");
    y.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "id");
    y2.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "id");

    // ------------------------------------------------------------------------
    // Suite: belongsTo

    // Test: remove single relationship
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    x.removeRelation(RelatedTableModel::eRelationBelongsTo, &y);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(QObject::disconnect(&y, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: removing one relationship should not interfere with other relationships
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "digest"));
    x.removeRelation(RelatedTableModel::eRelationBelongsTo, &y2);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y));
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(QObject::disconnect(&y, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))));


    // ------------------------------------------------------------------------
    // Suite: hasMany
    RelatedTableModel dnaSeqs;
    TableModel dnaSubseqs, dnaSubseqs2;

    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "gene" << "amino_seq_id");
    dnaSubseqs.setSource(dataSource, constants::kTableDnaSubseqs, QStringList() << "dna_seq_id" << "start");
    dnaSubseqs2.setSource(dataSource, constants::kTableDnaSubseqs, QStringList() << "dna_seq_id" << "start");

    // Test: remove single relationship, from the setRelationHasMany test, we know that the dnaSubseqs.sourceChanged signal
    //       will have been attached to dnaSeqs.__relationSourceChanged; thus, check that it is not connected after the
    //       relation is removed
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));
    dnaSeqs.removeRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: removing one relationship should not interfere with other relationships
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs2, "dna_seq_id"));
    dnaSeqs.removeRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs2);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs2) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs2, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))));
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y));
}

void TestRelatedTableModel::clearRelations()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    RelatedTableModel x;
    TableModel y, y2;

    x.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "id" << "astring_id" << "protein");
    y.setSource(dataSource, constants::kTableAstrings, QStringList() << "id");
    y2.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "start" << "stop");

    // Test: clear a single belongsTo relationship
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "astring_id"));
    x.clearRelations();
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(QObject::disconnect(&y, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: clear a single hasMany relationship
    QVERIFY(x.setRelation(RelatedTableModel::eRelationHasMany, &y2, "amino_seq_id"));
    x.clearRelations();
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationHasMany, &y2) == false);
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: clear multiple relationships
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "astring_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationHasMany, &y2, "amino_seq_id"));
    x.clearRelations();
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationHasMany, &y2) == false);
    QVERIFY(QObject::disconnect(&y, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(QObject::disconnect(&y2, SIGNAL(sourceChanged(TableModel *)), &x, SLOT(__relationSourceChanged(TableModel *))) == false);
}

void TestRelatedTableModel::setAutoLoadRelations()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    RelatedTableModel x;
    TableModel y, y2;

    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "digest" << "sequence");
    y.setSource(dataSource, constants::kTableAstrings, QStringList() << "id");
    y2.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "start" << "stop");

    // ------------------------------------------------------------------------
    // Suite: belongsTo

    // Setup: add some relations
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "digest"));

    // Test: default, should be auto loading
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Test: set autoload should update this value
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y, true);
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y2, false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y) == true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Check that the loadDone and loadError signals from y were attached to the __relationLoadDone
    // and __relationLoadError signals of x. Again, do this via QObject::disconnect
    QVERIFY(QObject::disconnect(&y, SIGNAL(loadDone(int)), &x, SLOT(__relationLoadDone(int))));
    QVERIFY(QObject::disconnect(&y, SIGNAL(loadError(QString,int)), &x, SLOT(__relationLoadError(QString,int))));

    // Test: autoload setting true -> false
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y, false);
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y2, true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == true);

    // Test: autoload setting true -> true
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y, true);
    x.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &y2, true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y) == true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &y2) == true);

    // ------------------------------------------------------------------------
    // Suite: hasMany

    x.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "astring_id" << "protein");
    y.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "sequence");
    y2.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "sequence");

    // Setup: add some relations
    QVERIFY(x.setRelation(RelatedTableModel::eRelationHasMany, &y, "amino_seq_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationHasMany, &y2, "amino_seq_id"));

    // Test: default, should not be auto loading
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y) == false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y2) == false);

    // Test: set autoload should update this value
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y, true);
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y2, false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y) == true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y2) == false);

    // Check that the loadDone and loadError signals from y were attached to the __relationLoadDone
    // and __relationLoadError signals of x. Again, do this via QObject::disconnect
    QVERIFY(QObject::disconnect(&y, SIGNAL(loadDone(int)), &x, SLOT(__relationLoadDone(int))));
    QVERIFY(QObject::disconnect(&y, SIGNAL(loadError(QString,int)), &x, SLOT(__relationLoadError(QString,int))));

    // Test: autoload setting true -> false
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y, false);
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y2, true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y) == false);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y2) == true);

    // Test: autoload setting true -> true
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y, true);
    x.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &y2, true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y) == true);
    QVERIFY(x.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &y2) == true);
}

// Test how setSource impacts any defined relations
void TestRelatedTableModel::setSource()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    RelatedTableModel dnaSeqs;
    TableModel dstrings, dnaSubseqs;

    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    dstrings.setSource(dataSource, constants::kTableDstrings, QStringList() << "id" << "digest" << "sequence");
    dnaSubseqs.setSource(dataSource, constants::kTableDnaSubseqs, QStringList() << "dna_seq_id" << "start");

    // ------------------------------------------------------------------------
    // Suite: setSource changes to dnaSeqs

    // Setup: add some relations
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationBelongsTo, &dstrings, "dstring_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));

    // Test: setSource with same parameters should do nothing
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings));
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    // Test: setSource on dnaSeqs with any changed parameters should remove all relationships
    dnaSeqs.setSource(0, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings) == false);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dstrings, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: null data source, empty table
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationBelongsTo, &dstrings, "dstring_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));
    dnaSeqs.setSource(0, QString(), QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings) == false);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dstrings, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Test: different fields
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationBelongsTo, &dstrings, "dstring_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));
    dnaSeqs.setSource(0, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings) == false);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dstrings, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);


    // ------------------------------------------------------------------------
    // Suite: setSource changes to dnaSubseqs and dstrings
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationBelongsTo, &dstrings, "dstring_id"));
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));

    // Test: setSource with same parameters should do nothing
    dstrings.setSource(dataSource, constants::kTableDstrings, QStringList() << "id" << "digest" << "sequence");
    dnaSubseqs.setSource(dataSource, constants::kTableDnaSubseqs, QStringList() << "dna_seq_id" << "start");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings));
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    // Test: change setSource of dstring
    dstrings.setSource(0, constants::kTableDstrings, QStringList() << "id" << "digest" << "sequence");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationBelongsTo, &dstrings) == false);
    QVERIFY(QObject::disconnect(&dstrings, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    // Test: change setSource of dnaSubseqs
    dnaSubseqs.setSource(0, constants::kTableDnaSubseqs, QStringList() << "dna_seq_id" << "start");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    /*
      [25 October 2010] - Decided to simplify relation management relative to setSource method calls.
      Specifically, all predefined relationships are removed when setSource is called. Previously, we
      kept all possible relationships depending on the new source parameters, yet this was too much work
      and not a necessary design element. So we KISSed it goodbye.

      The following test code is left should I decide to revive this approach.


    // Test: setSource without changing any fields used in belongsTo relations should not affect them
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "digest");
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y));
    QCOMPARE(x.relatedField(RelatedTableModel::eRelationBelongsTo, &y), QString("amino_seq_id"));
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2));
    QCOMPARE(x.relatedField(RelatedTableModel::eRelationBelongsTo, &y2), QString("digest"));

    // Test: setSource without any of the belongsTo fields, should remove them both
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "sequence");
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Setup - add the relations back in
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "digest" << "sequence");
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "digest"));

    // Test: setSource with just one of the fields
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "digest");
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2));
    QCOMPARE(x.relatedField(RelatedTableModel::eRelationBelongsTo, &y2), QString("digest"));

    // Test: setSource to no fields - should remove all belongsTo relationships
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList());
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);

    // Setup - add the relations back in
    x.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "digest" << "sequence");
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y, "amino_seq_id"));
    QVERIFY(x.setRelation(RelatedTableModel::eRelationBelongsTo, &y2, "digest"));

    // Test: empty source table should remove all belongs to relationships
    x.setSource(dataSource, QString(), QStringList() << "id");
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y) == false);
    QVERIFY(x.hasRelation(RelatedTableModel::eRelationBelongsTo, &y2) == false);


    // ------------------------------------------------------------------------
    // Suite: hasMany
    //        - Must also check that the sourceChanged signal is removed
    RelatedTableModel dnaSeqs;
    TableModel dnaSubseqs;

    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    dnaSubseqs.setSource(dataSource, constants::kTableDnaSubseqs, QStringList() << "id" << "dna_seq_id" << "start" << "stop");

    // Setup: add some relations
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));

    // --------------------------------
    // Test: set the source of dnaSeqs to an empty list should remove any hasMany relationships
    dnaSeqs.setSource(0, constants::kTableDnaSeqs, QStringList());
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Restore the relationships
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));

    // --------------------------------
    // Test: setting the source to an empty table should also remove the hasMany relationship
    dnaSeqs.setSource(dataSource, QString(), QStringList() << "id");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs) == false);
    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))) == false);

    // Restore the relationships
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id" << "dstring_id" << "gene");
    QVERIFY(dnaSeqs.setRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs, "dna_seq_id"));

    // --------------------------------
    // Test: all other changes to the source for dnaSeqs should not touch the hasMany relationship
    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "id");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    dnaSeqs.setSource(0, constants::kTableDnaSeqs, QStringList() << "dstring_id");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    dnaSeqs.setSource(dataSource, constants::kTableDnaSeqs, QStringList() << "dstring_id");
    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));

    QVERIFY(QObject::disconnect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *))));
    QObject::connect(&dnaSubseqs, SIGNAL(sourceChanged(TableModel *)), &dnaSeqs, SLOT(__relationSourceChanged(TableModel *)));

    // ------------------------------------------------------------------------
    // Suite: Source changes to dnaSubseq should remove relationship from dnaSeq as necessary

    QVERIFY(dnaSeqs.hasRelation(RelatedTableModel::eRelationHasMany, &dnaSubseqs));
    */
}

void TestRelatedTableModel::loadSimpleBelongsTo()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    TableModel aminoSeqs;
    RelatedTableModel aminoSubseqs;

    // Setup
    aminoSeqs.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "id" << "astring_id" << "label");
    aminoSubseqs.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "start" << "stop");
    QVERIFY(aminoSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoSeqs, "amino_seq_id"));

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ?? Also test simple loadError? No, this is not necessary because it is not even being overrided. If an error occurs
    // when pulling the data for the first table, it calls __selectError which is not present in RelatedTableModel.

    // ------------------------------------------------------------------------
    // Suite: no autoload configured
    QVERIFY(aminoSubseqs.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &aminoSeqs) == false);

    QSignalSpy spySeqLoadDone(&aminoSeqs, SIGNAL(loadDone(int)));
    QSignalSpy spySeqLoadError(&aminoSeqs, SIGNAL(loadError(QString,int)));
    QSignalSpy spySubseqLoadDone(&aminoSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spySubseqLoadError(&aminoSubseqs, SIGNAL(loadError(QString,int)));

    // Test: Pulling straight from the original table without hitting the linked table
    aminoSubseqs.load(QList<int>() << 1, 1);
    QVERIFY(spySeqLoadDone.isEmpty());
    QVERIFY(spySeqLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());
    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(spySubseqLoadDone.takeFirst().at(0).toInt(), 1);   // Verify the tag
    QCOMPARE(aminoSubseqs.data(1, "amino_seq_id").toInt(), 1);  // Verify that it is present in the table
    QCOMPARE(aminoSubseqs.data(1, "stop").toInt(), 112);        // Verify that it is present in the table
    QVERIFY(aminoSeqs.data(1, "id").isValid() == false);
    spySubseqLoadDone.clear();

    // ------------------------------------------------------------------------
    // Suite: autoload configured

    aminoSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoSeqs, true);

    // Test: Basic list
    aminoSubseqs.load(QList<int>() << 2, 2);
    QVERIFY(spySeqLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());

    QCOMPARE(spySeqLoadDone.count(), 1);
    QCOMPARE(aminoSeqs.data(2, "astring_id").toInt(), 2);                            // Verify that it is present in the table
    QCOMPARE(aminoSeqs.data(2, "label").toString(), QString("bll2760_152-270"));        // Verify that it is present in the table
    spySeqLoadDone.clear();

    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(spySubseqLoadDone.takeFirst().at(0).toInt(), 2);   // Verify the tag
    QCOMPARE(aminoSubseqs.data(2, "amino_seq_id").toInt(), 2);  // Verify that it is present in the table
    QCOMPARE(aminoSubseqs.data(2, "stop").toInt(), 119);        // Verify that it is present in the table
    spySubseqLoadDone.clear();

    // Test: multiple records
    aminoSubseqs.load(QList<int>() << 3 << 4, 10);
    QVERIFY(spySeqLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());

    QCOMPARE(spySeqLoadDone.count(), 1);
    QCOMPARE(aminoSeqs.data(3, "astring_id").toInt(), 3);                            // Verify that it is present in the table
    QCOMPARE(aminoSeqs.data(4, "astring_id").toInt(), 4);                            // Verify that it is present in the table
    QCOMPARE(aminoSeqs.data(3, "label").toString(), QString("SMa1229_128-245"));        // Verify that it is present in the table
    QCOMPARE(aminoSeqs.data(4, "label").toString(), QString("E4_20-133"));              // Verify that it is present in the table
    spySeqLoadDone.clear();

    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(spySubseqLoadDone.takeFirst().at(0).toInt(), 10);   // Verify the tag
    QCOMPARE(aminoSubseqs.data(3, "amino_seq_id").toInt(), 3);  // Verify that it is present in the table
    QCOMPARE(aminoSubseqs.data(4, "amino_seq_id").toInt(), 4);  // Verify that it is present in the table
    QCOMPARE(aminoSubseqs.data(3, "stop").toInt(), 118);        // Verify that it is present in the table
    QCOMPARE(aminoSubseqs.data(4, "stop").toInt(), 114);        // Verify that it is present in the table
    spySubseqLoadDone.clear();
}

void TestRelatedTableModel::loadMultipleBelongsTo()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    TableModel aminoSubseqs, aminoMsas;
    RelatedTableModel aminoMsaSubseqs;

    // Setup
    aminoSubseqs.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "start" << "stop");
    aminoMsas.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "id" << "name");

    aminoMsaSubseqs.setSource(dataSource, constants::kTableAminoMsaSubseqs, QStringList() << "id" << "amino_msa_id" << "amino_subseq_id");

    QVERIFY(aminoMsaSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas, "amino_msa_id"));
    QVERIFY(aminoMsaSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoSubseqs, "amino_subseq_id"));

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // Suite: no autoload configured
    QVERIFY(aminoMsaSubseqs.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas) == false);
    QVERIFY(aminoMsaSubseqs.isAutoLoadedRelation(RelatedTableModel::eRelationBelongsTo, &aminoSubseqs) == false);

    QSignalSpy spyMsaLoadDone(&aminoMsas, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaLoadError(&aminoMsas, SIGNAL(loadError(QString,int)));
    QSignalSpy spySubseqLoadDone(&aminoSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spySubseqLoadError(&aminoSubseqs, SIGNAL(loadError(QString,int)));
    QSignalSpy spyMsaSubseqLoadDone(&aminoMsaSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaSubseqLoadError(&aminoMsaSubseqs, SIGNAL(loadError(QString,int)));

    // Test: Pulling straight from the original table without hitting the linked table
    aminoMsaSubseqs.load(QList<int>() << 1, 1);
    QVERIFY(spyMsaLoadDone.isEmpty());
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spySubseqLoadDone.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());

    QVERIFY(spyMsaSubseqLoadError.isEmpty());
    QCOMPARE(spyMsaSubseqLoadDone.count(), 1);
    QCOMPARE(spyMsaSubseqLoadDone.takeFirst().at(0).toInt(), 1);   // Verify the tag
    QCOMPARE(aminoMsaSubseqs.data(1, "amino_subseq_id").toInt(), 2);  // Verify that it is present in the table
    QCOMPARE(aminoMsaSubseqs.data(1, "amino_msa_id").toInt(), 1);        // Verify that it is present in the table

    QVERIFY(aminoMsas.data(1, "id").isValid() == false);
    QVERIFY(aminoSubseqs.data(1, "id").isValid() == false);
    spyMsaSubseqLoadDone.clear();

    // ------------------------------------------------------------------------
    // Suite: one autoload configured
    aminoMsaSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoSubseqs, true);

    aminoMsaSubseqs.load(QList<int>() << 2, 100);
    QVERIFY(spyMsaLoadDone.isEmpty());
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());

    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(aminoSubseqs.data(3, "id").toInt(), 3);
    QCOMPARE(aminoSubseqs.data(3, "amino_seq_id").toInt(), 3);
    QCOMPARE(aminoSubseqs.data(3, "start").toInt(), 1);
    QCOMPARE(aminoSubseqs.data(3, "stop").toInt(), 118);

    QVERIFY(spyMsaSubseqLoadError.isEmpty());
    QCOMPARE(spyMsaSubseqLoadDone.count(), 1);
    QCOMPARE(spyMsaSubseqLoadDone.takeFirst().at(0).toInt(), 100);
    QCOMPARE(aminoMsaSubseqs.data(2, "amino_subseq_id").toInt(), 3);
    QCOMPARE(aminoMsaSubseqs.data(2, "amino_msa_id").toInt(), 1);

    QVERIFY(aminoMsas.data(1, "id").isValid() == false);
    QVERIFY(aminoSubseqs.data(1, "id").isValid() == false);
    spyMsaSubseqLoadDone.clear();
    spySubseqLoadDone.clear();

    // ------------------------------------------------------------------------
    // Suite: two autoloads configured
    aminoMsaSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas, true);

    aminoMsaSubseqs.load(QList<int>() << 3, 200);
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());
    QVERIFY(spyMsaSubseqLoadError.isEmpty());

    QCOMPARE(spyMsaLoadDone.count(), 1);
    QCOMPARE(aminoMsas.data(1, "id").toInt(), 1);
    QCOMPARE(aminoMsas.data(1, "name").toString(), QString("pas"));

    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(aminoSubseqs.data(4, "id").toInt(), 4);
    QCOMPARE(aminoSubseqs.data(4, "amino_seq_id").toInt(), 4);
    QCOMPARE(aminoSubseqs.data(4, "start").toInt(), 1);
    QCOMPARE(aminoSubseqs.data(4, "stop").toInt(), 114);

    QCOMPARE(spyMsaSubseqLoadDone.count(), 1);
    QCOMPARE(spyMsaSubseqLoadDone.takeFirst().at(0).toInt(), 200);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_subseq_id").toInt(), 4);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_msa_id").toInt(), 1);

    spyMsaSubseqLoadDone.clear();
    spySubseqLoadDone.clear();
    spyMsaLoadDone.clear();
}

void TestRelatedTableModel::loadComplexBelongsTo()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    TableModel aminoSeqs;
    RelatedTableModel aminoSubseqs;
    TableModel aminoMsas;
    RelatedTableModel aminoMsaSubseqs;

    // Setup
    aminoSeqs.setSource(dataSource, constants::kTableAminoSeqs, QStringList() << "id" << "astring_id" << "label");
    aminoSubseqs.setSource(dataSource, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "start" << "stop");
    aminoMsas.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "id" << "name");
    aminoMsaSubseqs.setSource(dataSource, constants::kTableAminoMsaSubseqs, QStringList() << "id" << "amino_msa_id" << "amino_subseq_id");

    QVERIFY(aminoSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoSeqs, "amino_seq_id"));
    QVERIFY(aminoMsaSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas, "amino_msa_id"));
    QVERIFY(aminoMsaSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoSubseqs, "amino_subseq_id"));

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // All autoloading
    aminoSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoSeqs, true);
    aminoMsaSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoSubseqs, true);
    aminoMsaSubseqs.setAutoLoadRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas, true);

    QSignalSpy spySeqLoadDone(&aminoSeqs, SIGNAL(loadDone(int)));
    QSignalSpy spySeqLoadError(&aminoSeqs, SIGNAL(loadError(QString,int)));
    QSignalSpy spySubseqLoadDone(&aminoSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spySubseqLoadError(&aminoSubseqs, SIGNAL(loadError(QString,int)));
    QSignalSpy spyMsaLoadDone(&aminoMsas, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaLoadError(&aminoMsas, SIGNAL(loadError(QString,int)));
    QSignalSpy spyMsaSubseqLoadDone(&aminoMsaSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaSubseqLoadError(&aminoMsaSubseqs, SIGNAL(loadError(QString,int)));

    aminoMsaSubseqs.load(QList<int>() << 2 << 3, 999);
    QVERIFY(spySeqLoadError.isEmpty());
    QVERIFY(spySubseqLoadError.isEmpty());
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spyMsaSubseqLoadError.isEmpty());

    QCOMPARE(spySeqLoadDone.count(), 1);
    QCOMPARE(spySubseqLoadDone.count(), 1);
    QCOMPARE(spyMsaLoadDone.count(), 1);
    QCOMPARE(spyMsaSubseqLoadDone.count(), 1);

    QCOMPARE(aminoSeqs.data(3, "astring_id").toInt(), 3);
    QCOMPARE(aminoSeqs.data(3, "label").toString(), QString("SMa1229_128-245"));
    QCOMPARE(aminoSeqs.data(4, "astring_id").toInt(), 4);
    QCOMPARE(aminoSeqs.data(4, "label").toString(), QString("E4_20-133"));

    QCOMPARE(aminoSubseqs.data(3, "amino_seq_id").toInt(), 3);
    QCOMPARE(aminoSubseqs.data(3, "start").toInt(), 1);
    QCOMPARE(aminoSubseqs.data(3, "stop").toInt(), 118);
    QCOMPARE(aminoSubseqs.data(4, "amino_seq_id").toInt(), 4);
    QCOMPARE(aminoSubseqs.data(4, "start").toInt(), 1);
    QCOMPARE(aminoSubseqs.data(4, "stop").toInt(), 114);

    QCOMPARE(aminoMsas.data(1, "name").toString(), QString("pas"));

    QCOMPARE(aminoMsaSubseqs.data(2, "amino_msa_id").toInt(), 1);
    QCOMPARE(aminoMsaSubseqs.data(2, "amino_subseq_id").toInt(), 3);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_msa_id").toInt(), 1);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_subseq_id").toInt(), 4);
}

void TestRelatedTableModel::loadHasMany()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    RelatedTableModel aminoMsas;
    RelatedTableModel aminoMsaSubseqs;

    // Setup
    aminoMsas.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "id" << "name");
    aminoMsaSubseqs.setSource(dataSource, constants::kTableAminoMsaSubseqs, QStringList() << "amino_msa_id" << "amino_subseq_id");
    QVERIFY(aminoMsas.setRelation(RelatedTableModel::eRelationHasMany, &aminoMsaSubseqs, "amino_msa_id"));

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    QSignalSpy spyMsaLoadDone(&aminoMsas, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaLoadError(&aminoMsas, SIGNAL(loadError(QString,int)));
    QSignalSpy spyMsaSubseqLoadDone(&aminoMsaSubseqs, SIGNAL(loadDone(int)));
    QSignalSpy spyMsaSubseqLoadError(&aminoMsaSubseqs, SIGNAL(loadError(QString,int)));

    // ------------------------------------------------------------------------
    // Suite: no autoload configured
    QVERIFY(aminoMsas.isAutoLoadedRelation(RelatedTableModel::eRelationHasMany, &aminoMsaSubseqs) == false);

    // Test: Pulling straight from the original table without hitting the linked table
    aminoMsas.load(QList<int>() << 1, 1);
    QCOMPARE(spyMsaLoadDone.count(), 1);
    QCOMPARE(spyMsaLoadDone.takeFirst().at(0).toInt(), 1);   // Verify the tag
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spyMsaSubseqLoadDone.isEmpty());
    QVERIFY(spyMsaSubseqLoadError.isEmpty());
    spyMsaLoadDone.clear();

    QCOMPARE(aminoMsas.data(1, "name").toString(), QString("pas"));
    QVERIFY(aminoMsaSubseqs.data(1, "amino_msa_id").isValid() == false);
    QVERIFY(aminoMsaSubseqs.data(2, "amino_msa_id").isValid() == false);
    QVERIFY(aminoMsaSubseqs.data(3, "amino_msa_id").isValid() == false);

    aminoMsas.clear();
    QVERIFY(aminoMsas.data(1, "name").isValid() == false);

    // ------------------------------------------------------------------------
    // Suite: autoload configured
    aminoMsas.setAutoLoadRelation(RelatedTableModel::eRelationHasMany, &aminoMsaSubseqs, true);

    aminoMsas.load(QList<int>() << -5 << 1, 20);
    QCOMPARE(spyMsaLoadDone.count(), 1);
    QCOMPARE(spyMsaLoadDone.takeFirst().at(0).toInt(), 20);   // Verify the tag
    QVERIFY(spyMsaLoadError.isEmpty());
    QVERIFY(spyMsaSubseqLoadError.isEmpty());
    QCOMPARE(spyMsaSubseqLoadDone.count(), 1);
    spyMsaLoadDone.clear();

    QCOMPARE(aminoMsas.data(1, "name").toString(), QString("pas"));
    QCOMPARE(aminoMsaSubseqs.data(1, "amino_msa_id").toInt(), 1);
    QCOMPARE(aminoMsaSubseqs.data(2, "amino_msa_id").toInt(), 1);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_msa_id").toInt(), 1);
    QCOMPARE(aminoMsaSubseqs.data(1, "amino_subseq_id").toInt(), 2);
    QCOMPARE(aminoMsaSubseqs.data(2, "amino_subseq_id").toInt(), 3);
    QCOMPARE(aminoMsaSubseqs.data(3, "amino_subseq_id").toInt(), 4);
    spyMsaSubseqLoadDone.clear();

    // ------------------------------------------------------------------------
    // Test: circular connection should work just fine!
    QVERIFY(aminoMsaSubseqs.setRelation(RelatedTableModel::eRelationBelongsTo, &aminoMsas, "amino_msa_id"));
    aminoMsas.clear();
    aminoMsaSubseqs.clear();

    aminoMsas.load(QList<int>() << 1);
    aminoMsaSubseqs.load(QList<int>() << 2);
}

QTEST_MAIN(TestRelatedTableModel)
#include "TestRelatedTableModel.moc"
