/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QAbstractItemModel>

#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "models/VaryingColumnProxyModel.h"
#include "TreeNode.h"

#include "MockModel.h"

Q_DECLARE_METATYPE(QModelIndex)

// ------------------------------------------------------------------------------------------------
// Core test class
class TestVaryingColumnProxyModel : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Test methods
    void constructor();
    void setColumnCount();
    void setSourceModel();
    void mapFromSource();
    void mapFromSourceBeforeMappedParent();
    void mapToSource();
    void index();
    void rowCount();
    void parent();


    // -------------------------------------------------
    // Signal related tests
    void dataChanged();
    void modelReset();
    void insertRow();
    void insertRows();
    void removeRow();
};

void TestVaryingColumnProxyModel::constructor()
{
    // Test: default number of columns
    VaryingColumnProxyModel proxy;
    QCOMPARE(proxy.columnCount(), 1);

    // Test: positive number of columns
    VaryingColumnProxyModel proxy2(0, 2);
    QCOMPARE(proxy2.columnCount(), 2);

    VaryingColumnProxyModel proxy3(0, 3);
    QCOMPARE(proxy3.columnCount(), 3);

    // Test: zero columns
    VaryingColumnProxyModel proxy4(0, 0);
    QCOMPARE(proxy4.columnCount(), 0);

    // Test: negative number of columns
    VaryingColumnProxyModel proxy5(0, -1);
    QCOMPARE(proxy5.columnCount(), 0);

    // Test: invalid source model
    QVERIFY(proxy.sourceModel() == 0);
}

void TestVaryingColumnProxyModel::setColumnCount()
{
    VaryingColumnProxyModel proxy;

    // Check that the reset signal is sent
    QSignalSpy spyModelAboutToBeReset(&proxy, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(&proxy, SIGNAL(modelReset()));

    QVERIFY(spyModelAboutToBeReset.isValid());
    QVERIFY(spyModelReset.isValid());

    proxy.setColumnCount(0);
    QCOMPARE(proxy.columnCount(), 0);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    proxy.setColumnCount(1);
    QCOMPARE(proxy.columnCount(), 1);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    proxy.setColumnCount(2);
    QCOMPARE(proxy.columnCount(), 2);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    proxy.setColumnCount(2);
    QCOMPARE(proxy.columnCount(), 2);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
}

void TestVaryingColumnProxyModel::setSourceModel()
{
    // Test: default model should be null
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    QVERIFY(proxy->sourceModel() == 0);

    // Create a couple models to check for the proper signal response
    MockModel *alpha = new MockModel(this);
    MockModel *beta = new MockModel(this);

    // ----------------------------------------------------
    // Part 1: Check the reset signal
    // Setup the signal spies
    QSignalSpy spyAboutToReset(proxy, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyReset(proxy, SIGNAL(modelReset()));

    // 5 possible scenarios:
    //    Current model     Provided model  Result
    // 1) Null              Null            No change
    // 2) Null              Good            Reset
    // 3) Model1            Model1          No change
    // 4) Model2            Model1          Reset
    // 5) Good              Null            Reset

    // Test: Case 1
    proxy->setSourceModel(0);
    QVERIFY(proxy->sourceModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);

    // Test: Case 2
    proxy->setSourceModel(alpha);
    QVERIFY(proxy->sourceModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();

    // Test: Case 3
    proxy->setSourceModel(alpha);
    QVERIFY(proxy->sourceModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);

    // Test: Case 4
    proxy->setSourceModel(beta);
    QVERIFY(proxy->sourceModel() == beta);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();

    // Test: changing from valid to null should trigger the reset signal
    proxy->setSourceModel(0);
    QVERIFY(proxy->sourceModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();

    // ----------------------------------------------------
    // Part 2: check that the appropriate signals are hooked up and disconnect properly
    //
    // Because there is no direct way to query the signals that are hooked up to an object, the following approach
    // was used. Disconnect will only return true if the signal was disconnected successfully. For this to happen, it
    // must have existed in the first place. Thus by checking the return value of disconnect we can in essence check
    // if the signal was attached correctly.
    //
    // A side effect is that the object is no longer in an expected state.
    VaryingColumnProxyModel *proxy2 = new VaryingColumnProxyModel(this);
    MockModel *gamma = new MockModel(this);

    // Verify that no signals have been setup on a default constructed proxy
    QCOMPARE(QObject::disconnect(gamma, 0, proxy2, 0), false);

    // Test: verify that the appropriate signals have been setup
    proxy2->setSourceModel(gamma);

    // Only way to verify that the signal handlers were setup is to disconnect them and check if it was true
    // Signals:
    // - dataChanged
    // - headerDataChanged
    // - layoutAboutToBeChanged
    // - layoutChanged
    // - modelAboutToBeReset
    // - modelReset
    // - rowsAboutToBeInserted
    // - rowsInserted
    // - rowsAboutToBeRemoved
    // - rowsRemoved
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(dataChanged(QModelIndex,QModelIndex)), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(layoutAboutToBeChanged()), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(layoutChanged()), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(modelAboutToBeReset()), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(modelReset()), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsInserted(QModelIndex,int,int)), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), proxy2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsRemoved(QModelIndex,int,int)), proxy2, 0), true);

    // We know that the signals are hooked up properly when setting a valid source model. When clearing that
    // they should all be disconnected.
    VaryingColumnProxyModel *proxy3 = new VaryingColumnProxyModel(this);
    MockModel *delta = new MockModel(this);

    proxy3->setSourceModel(delta);
    proxy3->setSourceModel(0);

    QCOMPARE(QObject::disconnect(delta, SIGNAL(dataChanged(QModelIndex,QModelIndex)), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(layoutAboutToBeChanged()), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(layoutChanged()), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(modelAboutToBeReset()), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(modelReset()), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsInserted(QModelIndex,int,int)), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), proxy3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsRemoved(QModelIndex,int,int)), proxy3, 0), false);


    // Final test: changing from a valid model to another valid model
    VaryingColumnProxyModel *proxy4 = new VaryingColumnProxyModel(this);
    MockModel *epsilon = new MockModel(this);
    MockModel *theta = new MockModel(this);

    proxy4->setSourceModel(epsilon);
    proxy4->setSourceModel(theta);

    // Verify all relevant signals are disconnected from epsilon
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(dataChanged(QModelIndex,QModelIndex)), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(layoutAboutToBeChanged()), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(layoutChanged()), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(modelAboutToBeReset()), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(modelReset()), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsInserted(QModelIndex,int,int)), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), proxy4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsRemoved(QModelIndex,int,int)), proxy4, 0), false);

    // Verify that all relevant signals are hooked up for theta
    QCOMPARE(QObject::disconnect(theta, SIGNAL(dataChanged(QModelIndex,QModelIndex)), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(layoutAboutToBeChanged()), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(layoutChanged()), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(modelAboutToBeReset()), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(modelReset()), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsInserted(QModelIndex,int,int)), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), proxy4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsRemoved(QModelIndex,int,int)), proxy4, 0), true);
}

void TestVaryingColumnProxyModel::mapFromSource()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    // Test: Before any source model has been defined, make sure it returns an invalid qmodelindex
    //       Will generate a warning that may be ignored since we are testing
    QVERIFY(proxy->mapFromSource(QModelIndex()) == QModelIndex());

    // Create a valid model for testing
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    // Test: root index returns root index
    QVERIFY(proxy->mapFromSource(QModelIndex()) == QModelIndex());

    // Test: valid source index from different model returns an invalid qmodelindex
    //       Will generate a warning that may be ignored since we are testing
    MockModel *model2 = new MockModel(this);
    model2->loadTreeSetOne();
    QVERIFY(proxy->mapFromSource(model2->index(0, 0, QModelIndex())) == QModelIndex());

    // Test: Go through all the nodes in model and verify that they are the same in the proxy
    // Tree Set #1
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    QModelIndex sourceGroup = model->index(0, 0, QModelIndex());
    QModelIndex sourceOne = model->index(0, 0, sourceGroup);
    QModelIndex sourceTwo = model->index(1, 0, sourceGroup);
    QModelIndex sourceAnotherGroup = model->index(2, 0, sourceGroup);
    QModelIndex sourceFive = model->index(0, 0, sourceAnotherGroup);
    QModelIndex sourceThree = model->index(1, 0, QModelIndex());
    QModelIndex sourceFour = model->index(2, 0, QModelIndex());
    QModelIndex sourceLabMembers = model->index(3, 0, QModelIndex());
    QModelIndex sourceLuke = model->index(0, 0, sourceLabMembers);
    QModelIndex sourceMegan = model->index(1, 0, sourceLabMembers);

    // Test: verify that they map 1:1 to the proxy but have the proxy model
    QModelIndex proxyGroup = proxy->mapFromSource(sourceGroup);
    QModelIndex proxyOne = proxy->mapFromSource(sourceOne);
    QModelIndex proxyTwo = proxy->mapFromSource(sourceTwo);
    QModelIndex proxyAnotherGroup = proxy->mapFromSource(sourceAnotherGroup);
    QModelIndex proxyFive = proxy->mapFromSource(sourceFive);
    QModelIndex proxyThree = proxy->mapFromSource(sourceThree);
    QModelIndex proxyFour = proxy->mapFromSource(sourceFour);
    QModelIndex proxyLabMembers = proxy->mapFromSource(sourceLabMembers);
    QModelIndex proxyLuke = proxy->mapFromSource(sourceLuke);
    QModelIndex proxyMegan = proxy->mapFromSource(sourceMegan);

    QCOMPARE(proxyGroup.row(), 0);
    QCOMPARE(proxyOne.row(), 0);
    QCOMPARE(proxyTwo.row(), 1);
    QCOMPARE(proxyAnotherGroup.row(), 2);
    QCOMPARE(proxyFive.row(), 0);
    QCOMPARE(proxyThree.row(), 1);
    QCOMPARE(proxyFour.row(), 2);
    QCOMPARE(proxyLabMembers.row(), 3);
    QCOMPARE(proxyLuke.row(), 0);
    QCOMPARE(proxyMegan.row(), 1);

    QCOMPARE(proxyGroup.column(), 0);
    QCOMPARE(proxyOne.column(), 0);
    QCOMPARE(proxyTwo.column(), 0);
    QCOMPARE(proxyAnotherGroup.column(), 0);
    QCOMPARE(proxyFive.column(), 0);
    QCOMPARE(proxyThree.column(), 0);
    QCOMPARE(proxyFour.column(), 0);
    QCOMPARE(proxyLabMembers.column(), 0);
    QCOMPARE(proxyLuke.column(), 0);
    QCOMPARE(proxyMegan.column(), 0);

    QCOMPARE(proxyGroup.model(), proxy);
    QCOMPARE(proxyOne.model(), proxy);
    QCOMPARE(proxyTwo.model(), proxy);
    QCOMPARE(proxyAnotherGroup.model(), proxy);
    QCOMPARE(proxyFive.model(), proxy);
    QCOMPARE(proxyThree.model(), proxy);
    QCOMPARE(proxyFour.model(), proxy);
    QCOMPARE(proxyLabMembers.model(), proxy);
    QCOMPARE(proxyLuke.model(), proxy);
    QCOMPARE(proxyMegan.model(), proxy);
}

void TestVaryingColumnProxyModel::mapFromSourceBeforeMappedParent()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    // Create test model
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    // Tree Set #1
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    QModelIndex sourceGroup = model->index(0, 0, QModelIndex());
    QModelIndex sourceOne = model->index(0, 0, sourceGroup);
    QModelIndex sourceTwo = model->index(1, 0, sourceGroup);
    QModelIndex sourceAnotherGroup = model->index(2, 0, sourceGroup);
    QModelIndex sourceFive = model->index(0, 0, sourceAnotherGroup);
    QModelIndex sourceThree = model->index(1, 0, QModelIndex());
    QModelIndex sourceFour = model->index(2, 0, QModelIndex());
    QModelIndex sourceLabMembers = model->index(3, 0, QModelIndex());
    QModelIndex sourceLuke = model->index(0, 0, sourceLabMembers);
    QModelIndex sourceMegan = model->index(1, 0, sourceLabMembers);

    // Test: Map from source with a deep source index (parent source indexes have not been mapped)
    QModelIndex proxyIndex = proxy->mapFromSource(sourceFive);
    QCOMPARE(proxyIndex.data().toInt(), 5);
    QVERIFY(proxyIndex.column() == 0);
    QVERIFY(proxyIndex.row() == 0);
    QVERIFY(proxyIndex.parent().parent().isValid());
    QVERIFY(proxyIndex.parent().parent().parent().isValid() == false);
}

void TestVaryingColumnProxyModel::mapToSource()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    // Test: pass invalid qmodelindex to proxy without source model should return qmodelindex
    QVERIFY(proxy->mapToSource(QModelIndex()) == QModelIndex());

    // Create test model
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    // Test: root index returns invalid index
    QVERIFY(proxy->mapToSource(QModelIndex()) == QModelIndex());

    // Test: valid index from different proxy returns invalid QModelIndex
    //       Will generate a warning that may be ignored since we are testing
    VaryingColumnProxyModel *proxy2 = new VaryingColumnProxyModel(this);
    proxy2->setSourceModel(model);
    QModelIndex indexFromOtherModel = proxy2->mapFromSource(model->index(0, 0, QModelIndex()));
    QVERIFY(proxy->mapToSource(indexFromOtherModel) == QModelIndex());

    // Test: give it a source index
    indexFromOtherModel = model->index(0, 0, QModelIndex());
    QVERIFY(proxy->mapToSource(indexFromOtherModel) == QModelIndex());

    // Test: Go through all the nodes in model and verify that they are the same in the proxy
    // Tree Set #1
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    QModelIndex sourceGroup = model->index(0, 0, QModelIndex());
    QModelIndex sourceOne = model->index(0, 0, sourceGroup);
    QModelIndex sourceTwo = model->index(1, 0, sourceGroup);
    QModelIndex sourceAnotherGroup = model->index(2, 0, sourceGroup);
    QModelIndex sourceFive = model->index(0, 0, sourceAnotherGroup);
    QModelIndex sourceThree = model->index(1, 0, QModelIndex());
    QModelIndex sourceFour = model->index(2, 0, QModelIndex());
    QModelIndex sourceLabMembers = model->index(3, 0, QModelIndex());
    QModelIndex sourceLuke = model->index(0, 0, sourceLabMembers);
    QModelIndex sourceMegan = model->index(1, 0, sourceLabMembers);

    QModelIndex proxyGroup = proxy->mapFromSource(sourceGroup);
    QModelIndex proxyOne = proxy->mapFromSource(sourceOne);
    QModelIndex proxyTwo = proxy->mapFromSource(sourceTwo);
    QModelIndex proxyAnotherGroup = proxy->mapFromSource(sourceAnotherGroup);
    QModelIndex proxyFive = proxy->mapFromSource(sourceFive);
    QModelIndex proxyThree = proxy->mapFromSource(sourceThree);
    QModelIndex proxyFour = proxy->mapFromSource(sourceFour);
    QModelIndex proxyLabMembers = proxy->mapFromSource(sourceLabMembers);
    QModelIndex proxyLuke = proxy->mapFromSource(sourceLuke);
    QModelIndex proxyMegan = proxy->mapFromSource(sourceMegan);

    // Test: verify that they map 1:1 back to the source
    QCOMPARE(proxy->mapToSource(proxyGroup), sourceGroup);
    QCOMPARE(proxy->mapToSource(proxyOne), sourceOne);
    QCOMPARE(proxy->mapToSource(proxyTwo), sourceTwo);
    QCOMPARE(proxy->mapToSource(proxyAnotherGroup), sourceAnotherGroup);
    QCOMPARE(proxy->mapToSource(proxyFive), sourceFive);
    QCOMPARE(proxy->mapToSource(proxyThree), sourceThree);
    QCOMPARE(proxy->mapToSource(proxyFour), sourceFour);
    QCOMPARE(proxy->mapToSource(proxyLabMembers), sourceLabMembers);
    QCOMPARE(proxy->mapToSource(proxyLuke), sourceLuke);
    QCOMPARE(proxy->mapToSource(proxyMegan), sourceMegan);
}

void TestVaryingColumnProxyModel::index()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    // Test: index on proxy model without any source - will generate warning that may be ignored
    QVERIFY(proxy->index(0, 2, QModelIndex()) == QModelIndex());

    // Load actual model
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    // Test: valid index from different proxy returns invalid QModelIndex
    //       Will generate a warning that may be ignored since we are testing
    VaryingColumnProxyModel *proxy2 = new VaryingColumnProxyModel(this);
    proxy2->setSourceModel(model);
    QModelIndex indexFromOtherModel = proxy2->mapFromSource(model->index(0, 0, QModelIndex()));
    QVERIFY(proxy->index(0, 0, indexFromOtherModel) == QModelIndex());

    // Test: give it a source index
    indexFromOtherModel = model->index(0, 0, QModelIndex());
    QVERIFY(proxy->index(0, 0, indexFromOtherModel) == QModelIndex());

    // Test: index with negative indices should come back
    QVERIFY(proxy->index(-1, 1, QModelIndex()).isValid() == false);

    // Test: Go through all the nodes in model and verify that index works as expected
    // Tree Set #1
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    QModelIndex sourceGroup = model->index(0, 0, QModelIndex());
    QModelIndex sourceOne = model->index(0, 0, sourceGroup);
    QModelIndex sourceTwo = model->index(1, 0, sourceGroup);
    QModelIndex sourceAnotherGroup = model->index(2, 0, sourceGroup);
    QModelIndex sourceFive = model->index(0, 0, sourceAnotherGroup);
    QModelIndex sourceThree = model->index(1, 0, QModelIndex());
    QModelIndex sourceFour = model->index(2, 0, QModelIndex());
    QModelIndex sourceLabMembers = model->index(3, 0, QModelIndex());
    QModelIndex sourceLuke = model->index(0, 0, sourceLabMembers);
    QModelIndex sourceMegan = model->index(1, 0, sourceLabMembers);

    QModelIndex proxyGroup = proxy->mapFromSource(sourceGroup);
    QModelIndex proxyOne = proxy->mapFromSource(sourceOne);
    QModelIndex proxyTwo = proxy->mapFromSource(sourceTwo);
    QModelIndex proxyAnotherGroup = proxy->mapFromSource(sourceAnotherGroup);
    QModelIndex proxyFive = proxy->mapFromSource(sourceFive);
    QModelIndex proxyThree = proxy->mapFromSource(sourceThree);
    QModelIndex proxyFour = proxy->mapFromSource(sourceFour);
    QModelIndex proxyLabMembers = proxy->mapFromSource(sourceLabMembers);
    QModelIndex proxyLuke = proxy->mapFromSource(sourceLuke);
    QModelIndex proxyMegan = proxy->mapFromSource(sourceMegan);

    QVERIFY(proxy->index(0, 0) == proxyGroup);
    QVERIFY(proxy->index(0, 0, proxyGroup) == proxyOne);
    QVERIFY(proxy->index(1, 0, proxyGroup) == proxyTwo);
    QVERIFY(proxy->index(2, 0, proxyGroup) == proxyAnotherGroup);
    QVERIFY(proxy->index(0, 0, proxyAnotherGroup) == proxyFive);
    QVERIFY(proxy->index(1, 0) == proxyThree);
    QVERIFY(proxy->index(2, 0) == proxyFour);
    QVERIFY(proxy->index(3, 0) == proxyLabMembers);
    QVERIFY(proxy->index(0, 0, proxyLabMembers) == proxyLuke);
    QVERIFY(proxy->index(1, 0, proxyLabMembers) == proxyMegan);
}

void TestVaryingColumnProxyModel::rowCount()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    QCOMPARE(proxy->rowCount(), 0);

    // Load actual model
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);
    QCOMPARE(proxy->rowCount(), 4);

    // Test: valid index from different proxy returns invalid QModelIndex
    //       Will generate a warning that may be ignored since we are testing
    VaryingColumnProxyModel *proxy2 = new VaryingColumnProxyModel(this);
    proxy2->setSourceModel(model);
    QModelIndex indexFromOtherModel = proxy2->index(0, 0);
    QVERIFY(proxy->rowCount(indexFromOtherModel) == 0);

    // Test: give it a source index
    indexFromOtherModel = model->index(0, 0, QModelIndex());
    QVERIFY(proxy->rowCount(indexFromOtherModel) == 0);

    // Test: rowCount on all tree members
    QModelIndex proxyGroup = proxy->index(0, 0);
    QModelIndex proxyOne = proxy->index(0, 0, proxyGroup);
    QModelIndex proxyTwo = proxy->index(1, 0, proxyGroup);
    QModelIndex proxyAnotherGroup = proxy->index(2, 0, proxyGroup);
    QModelIndex proxyFive = proxy->index(0, 0, proxyAnotherGroup);
    QModelIndex proxyThree = proxy->index(1, 0);
    QModelIndex proxyFour = proxy->index(2, 0);
    QModelIndex proxyLabMembers = proxy->index(3, 0);
    QModelIndex proxyLuke = proxy->index(0, 0, proxyLabMembers);
    QModelIndex proxyMegan = proxy->index(1, 0, proxyLabMembers);

    QCOMPARE(proxy->rowCount(proxyGroup), 3);
    QCOMPARE(proxy->rowCount(proxyOne), 0);
    QCOMPARE(proxy->rowCount(proxyTwo), 0);
    QCOMPARE(proxy->rowCount(proxyAnotherGroup), 1);
    QCOMPARE(proxy->rowCount(proxyFive), 0);
    QCOMPARE(proxy->rowCount(proxyThree), 0);
    QCOMPARE(proxy->rowCount(proxyFour), 0);
    QCOMPARE(proxy->rowCount(proxyLabMembers), 2);
    QCOMPARE(proxy->rowCount(proxyLuke), 0);
    QCOMPARE(proxy->rowCount(proxyMegan), 0);
}

void TestVaryingColumnProxyModel::parent()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);

    // Test: uninitialized source model
    QCOMPARE(proxy->parent(QModelIndex()), QModelIndex());

    // Load actual model
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    QCOMPARE(proxy->parent(QModelIndex()), QModelIndex());

    // Test: valid index from different proxy returns invalid QModelIndex
    //       Will generate a warning that may be ignored since we are testing
    VaryingColumnProxyModel *proxy2 = new VaryingColumnProxyModel(this);
    proxy2->setSourceModel(model);
    QModelIndex indexFromOtherModel = proxy2->index(0, 0);
    QVERIFY(proxy->parent(indexFromOtherModel) == QModelIndex());

    // Test: give it a source index
    indexFromOtherModel = model->index(0, 0, QModelIndex());
    QVERIFY(proxy->parent(indexFromOtherModel) == QModelIndex());

    // Test: Walk through all the indexes and test that their parent works as expected
    QModelIndex proxyGroup = proxy->index(0, 0);
    QModelIndex proxyOne = proxy->index(0, 0, proxyGroup);
    QModelIndex proxyTwo = proxy->index(1, 0, proxyGroup);
    QModelIndex proxyAnotherGroup = proxy->index(2, 0, proxyGroup);
    QModelIndex proxyFive = proxy->index(0, 0, proxyAnotherGroup);
    QModelIndex proxyThree = proxy->index(1, 0);
    QModelIndex proxyFour = proxy->index(2, 0);
    QModelIndex proxyLabMembers = proxy->index(3, 0);
    QModelIndex proxyLuke = proxy->index(0, 0, proxyLabMembers);
    QModelIndex proxyMegan = proxy->index(1, 0, proxyLabMembers);

    QVERIFY(proxy->parent(proxyGroup) == QModelIndex());
    QVERIFY(proxy->parent(proxyOne) == proxyGroup);
    QVERIFY(proxy->parent(proxyTwo) == proxyGroup);
    QVERIFY(proxy->parent(proxyAnotherGroup) == proxyGroup);
    QVERIFY(proxy->parent(proxyFive) == proxyAnotherGroup);
    QVERIFY(proxy->parent(proxyThree) == QModelIndex());
    QVERIFY(proxy->parent(proxyFour) == QModelIndex());
    QVERIFY(proxy->parent(proxyLabMembers) == QModelIndex());
    QVERIFY(proxy->parent(proxyLuke) == proxyLabMembers);
    QVERIFY(proxy->parent(proxyMegan) == proxyLabMembers);
}

void TestVaryingColumnProxyModel::dataChanged()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    qRegisterMetaType<QModelIndex>("QModelIndex");

    QSignalSpy spyDataChanged(proxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVERIFY(spyDataChanged.isValid());

    // Test: changes to underlying model are relayed by the proxy
    QVERIFY(model->setData(model->index(1, 1), "Swine"));       // Change "Three" to "Swine"
    QVERIFY(spyDataChanged.count() == 1);
    QList<QVariant> arguments = spyDataChanged.takeFirst();
    QModelIndex topLeft = qvariant_cast<QModelIndex>(arguments.at(0));
    QVERIFY(topLeft.model() == proxy);
    QCOMPARE(topLeft.row(), 1);
    QCOMPARE(topLeft.column(), 1);

    QModelIndex bottomRight = qvariant_cast<QModelIndex>(arguments.at(1));
    QVERIFY(bottomRight.model() == proxy);
    QCOMPARE(bottomRight.row(), 1);
    QCOMPARE(bottomRight.column(), 1);

    QCOMPARE(proxy->data(proxy->index(1, 1)).toString(), QString("Swine"));

    // Try changing data of a nested item
    spyDataChanged.clear();
    QVERIFY(model->setData(model->index(0, 0, model->index(0, 0)), 56));
    QVERIFY(spyDataChanged.count() == 1);
    arguments = spyDataChanged.takeFirst();
    topLeft = qvariant_cast<QModelIndex>(arguments.at(0));
    QVERIFY(topLeft.model() == proxy);
    QCOMPARE(topLeft.row(), 0);
    QCOMPARE(topLeft.column(), 0);

    bottomRight = qvariant_cast<QModelIndex>(arguments.at(1));
    QVERIFY(bottomRight.model() == proxy);
    QCOMPARE(bottomRight.row(), 0);
    QCOMPARE(bottomRight.column(), 0);

    QVERIFY(topLeft.parent() == proxy->index(0, 0));

    QCOMPARE(proxy->data(proxy->index(0, 0, proxy->index(0, 0))).toInt(), 56);
}

void TestVaryingColumnProxyModel::modelReset()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    QSignalSpy spyReset(proxy, SIGNAL(modelReset()));
    QVERIFY(spyReset.isValid());

    model->doReset();

    QVERIFY(spyReset.count() == 1);
}

void TestVaryingColumnProxyModel::insertRow()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    QSignalSpy spyRowsAboutToBeInserted(proxy, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeInserted.isValid());

    QSignalSpy spyRowsInserted(proxy, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsInserted.isValid());

    // ----------------------------------------------------------
    // Test: insert rows at end of list under root
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // ---> NEW NODE HERE!! <---

    int row = model->rowCount(QModelIndex());
    QVERIFY(model->insertRow(row, new MockNode(1000, "MegaNode"), QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QModelIndex proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 1000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("MegaNode"));

    // ----------------------------------------------------------
    // Test: insert in middle of list under root
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // ---> NEW NODE HERE!  <---
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 1;
    QVERIFY(model->insertRow(row, new MockNode(2000, "Hairy"), QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 2000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("Hairy"));

    // ----------------------------------------------------------
    // Test: insert at beginning of list under root
    // Root
    // ---> NEW NODE HERE!  <---
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Hairy
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 0;
    QVERIFY(model->insertRow(row, new MockNode(3000, "Slimy"), QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 3000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("Slimy"));

    // ----------------------------------------------------------
    // Test: insert at subnode
    // Root
    // |___ Slimy
    // |___ Group
    // |    |___ One
    // ---> NEW NODE HERE!  <---
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Hairy
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 1;
    QVERIFY(model->insertRow(row, new MockNode(4000, "Printer"), model->index(1, 0)));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(1, 0));
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(1, 0));
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(proxy->index(1, 0)), model->rowCount(model->index(1, 0)));
    QCOMPARE(proxy->data(proxy->index(row, 0, proxy->index(1, 0))).toInt(), 4000);
    QCOMPARE(proxy->data(proxy->index(row, 1, proxy->index(1, 0))).toString(), QString("Printer"));
}

void TestVaryingColumnProxyModel::insertRows()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    QSignalSpy spyRowsAboutToBeInserted(proxy, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeInserted.isValid());

    QSignalSpy spyRowsInserted(proxy, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsInserted.isValid());

    // ----------------------------------------------------------
    // Test: insert rows at end of list under root
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // ---> NEW NODES HERE!! <---

    QList<MockNode *> mockNodes;
    mockNodes.append(new MockNode(1000, "MegaNode"));
    mockNodes.append(new MockNode(1001, "MegaNode2"));

    int row = model->rowCount(QModelIndex());
    QVERIFY(model->insertRows(row, mockNodes, QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QModelIndex proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row+1);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row+1);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 1000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("MegaNode"));
    QCOMPARE(proxy->data(proxy->index(row+1, 0)).toInt(), 1001);
    QCOMPARE(proxy->data(proxy->index(row+1, 1)).toString(), QString("MegaNode2"));

    // ----------------------------------------------------------
    // Test: insert in middle of list under root
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // ---> NEW NODE HERE!  <---
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    // |___ MegaNode2
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 1;
    mockNodes.clear();
    mockNodes.append(new MockNode(2000, "Hairy"));
    mockNodes.append(new MockNode(2001, "Hairy2"));
    QVERIFY(model->insertRows(row, mockNodes, QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row+1);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row+1);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 2000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("Hairy"));
    QCOMPARE(proxy->data(proxy->index(row+1, 0)).toInt(), 2001);
    QCOMPARE(proxy->data(proxy->index(row+1, 1)).toString(), QString("Hairy2"));

    // ----------------------------------------------------------
    // Test: insert at beginning of list under root
    // Root
    // ---> NEW NODE HERE!  <---
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Hairy
    // |___ Hairy2
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 0;
    mockNodes.clear();
    mockNodes.append(new MockNode(3000, "Slimy"));
    mockNodes.append(new MockNode(3001, "Slimy2"));
    QVERIFY(model->insertRows(row, mockNodes, QModelIndex()));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row+1);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row+1);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->data(proxy->index(row, 0)).toInt(), 3000);
    QCOMPARE(proxy->data(proxy->index(row, 1)).toString(), QString("Slimy"));
    QCOMPARE(proxy->data(proxy->index(row+1, 0)).toInt(), 3001);
    QCOMPARE(proxy->data(proxy->index(row+1, 1)).toString(), QString("Slimy2"));

    // ----------------------------------------------------------
    // Test: insert at subnode
    // Root
    // |___ Slimy
    // |___ Slimy2
    // |___ Group
    // |    |___ One
    // ---> NEW NODE HERE!  <---
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Hairy
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    // |___ MegaNode
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    row = 1;
    mockNodes.clear();
    mockNodes.append(new MockNode(4000, "Printer"));
    mockNodes.append(new MockNode(4001, "Printer2"));
    QVERIFY(model->insertRows(row, mockNodes, model->index(2, 0)));

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeInserted.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(2, 0));
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeInserted.at(0).at(2).toInt(), row+1);

    QCOMPARE(spyRowsInserted.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsInserted.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(2, 0));
    QCOMPARE(spyRowsInserted.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsInserted.at(0).at(2).toInt(), row+1);

    // Verify that the row was added and visible in the proxy
    QCOMPARE(proxy->rowCount(proxy->index(2, 0)), model->rowCount(model->index(2, 0)));
    QCOMPARE(proxy->data(proxy->index(row, 0, proxy->index(2, 0))).toInt(), 4000);
    QCOMPARE(proxy->data(proxy->index(row, 1, proxy->index(2, 0))).toString(), QString("Printer"));
    QCOMPARE(proxy->data(proxy->index(row+1, 0, proxy->index(2, 0))).toInt(), 4001);
    QCOMPARE(proxy->data(proxy->index(row+1, 1, proxy->index(2, 0))).toString(), QString("Printer2"));
}

void TestVaryingColumnProxyModel::removeRow()
{
    VaryingColumnProxyModel *proxy = new VaryingColumnProxyModel(this);
    MockModel *model = new MockModel(this);
    model->loadTreeSetOne();
    proxy->setSourceModel(model);

    model->appendChild(new MockNode(1000, "TBR"));

    QSignalSpy spyRowsAboutToBeRemoved(proxy, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeRemoved.isValid());

    QSignalSpy spyRowsRemoved(proxy, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QVERIFY(spyRowsRemoved.isValid());

    // ----------------------------------------------------------
    // Test: remove rows at end of list under root
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    // |    |___ Luke
    // |    |___ Megan
    // |___ TBR    <---
    int row = 4;
    model->removeRows(row, 1, QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    QModelIndex proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsRemoved.at(0).at(2).toInt(), row);

    // Verify that the row was removed and no longer visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->rowCount(), 4);

    // ----------------------------------------------------------
    // Test: remove from middle of list
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four         <---
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    // Test: call index on Luke to map Lab members
    proxy->index(0, 0, proxy->index(3, 0));

    row = 2;
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();
    model->removeRows(row, 1, QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsRemoved.at(0).at(2).toInt(), row);

    // Verify that the row was removed and no longer visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->rowCount(), 3);
    QCOMPARE(proxy->data(proxy->index(2, 1)).toString(), QString("Lab members"));

    // ----------------------------------------------------------
    // Test: remove from beginning of list under root
    // Root
    // |___ Group               <---
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan

    // Test: call index on Group to map ot
    proxy->index(0, 0);

    row = 0;
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();
    model->removeRows(row, 1, QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsRemoved.at(0).at(0));
    QVERIFY(proxyParent == QModelIndex());
    QCOMPARE(spyRowsRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsRemoved.at(0).at(2).toInt(), row);

    // Verify that the row was removed and no longer visible in the proxy
    QCOMPARE(proxy->rowCount(), model->rowCount());
    QCOMPARE(proxy->rowCount(), 2);
    QCOMPARE(proxy->data(proxy->index(0, 1)).toString(), QString("Three"));

    // ----------------------------------------------------------
    // Test: remove from beginning of list under sub node
    // Root
    // |___ Three
    // |___ Lab members
    //      |___ Luke    <---
    //      |___ Megan

    // Test: call index on Group to map ot
    row = 0;
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();
    model->removeRows(row, 1, model->index(1, 0));
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsAboutToBeRemoved.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(1, 0));
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsAboutToBeRemoved.at(0).at(2).toInt(), row);

    QCOMPARE(spyRowsRemoved.count(), 1);
    proxyParent = qvariant_cast<QModelIndex>(spyRowsRemoved.at(0).at(0));
    QVERIFY(proxyParent == proxy->index(1, 0));
    QCOMPARE(spyRowsRemoved.at(0).at(1).toInt(), row);
    QCOMPARE(spyRowsRemoved.at(0).at(2).toInt(), row);

    // Verify that the row was removed and no longer visible in the proxy
    QCOMPARE(proxy->rowCount(proxy->index(1, 0)), model->rowCount(model->index(1, 0)));
    QCOMPARE(proxy->rowCount(proxy->index(1, 0)), 1);
    QCOMPARE(proxy->data(proxy->index(0, 1, proxy->index(1, 0))).toString(), QString("Megan"));
}

QTEST_MAIN(TestVaryingColumnProxyModel)
#include "TestVaryingColumnProxyModel.moc"
