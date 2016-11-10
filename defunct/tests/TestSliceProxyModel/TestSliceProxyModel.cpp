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

#include "models/AdocTreeModel.h"
#include "models/SliceProxyModel.h"
#include "TreeNode.h"

#include "MockSliceProxyModel.h"

#include <QtDebug>

Q_DECLARE_METATYPE(QModelIndex)

// ------------------------------------------------------------------------------------------------
// Core test class
class TestSliceProxyModel : public QObject
{
    Q_OBJECT

public:
    TestSliceProxyModel()
    {
        // For the signal spy to work with model indices
        qRegisterMetaType<QModelIndex>("QModelIndex");
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Test methods
    void constructor();
    void setTreeModel();

    void setSourceParent();     // Also tests sourceParent()
    void defaultSortColumn();
    void primaryColumn();

    // This method tests that setting the tree model also clears any loaded slices
    void setTreeModelClearSlices();

    void headerData();
    void index();
    void rowCount();
    void parent();
    void clear();
    void mapToSource();
    void flags();

    void removeRows();

    // Drag and drop
    void dragdrop();

    void refreshSourceParent();
    void isGoodIndex();

    // -------------------------------------------------
    // Signal related tests
    void dataChanged();
    void modelReset();
    void insertRow();
    void removeRow();

private:
    AdocTreeModel *loadTestTree();
};

/**
  * Test tree one:
  *
  * Root
  * |___ Zeta (Group)
  *      |___ Alignment
  *      |___ Binding (Group)
  *      |___ Site
  * |___ Alpha (Group)
  *      |___ Toxic
  *      |___ Acid
  *      |___ Dummy
  *      |___ Base
  * |___ Delta (Group)
  *      |___ Force
  * |___ Beta (Group)
  * |___ Gamma (Group)
  *      |____ Level2 (Group)
  *            |____ Apple
  * |___ Kappa (Group)
  *
  * It is vital that when testing the removal of rows, none of these nodes have a fkId_ defined
  * unless an underlying database has been configured. Otherwise, the tests will fail because
  * AdocTreeModel will attempt to remove these from the non-existent database which will throw
  * an exception and in turn not update the Tree model. For details, see AdocTreeModel::removeRow()
  */
AdocTreeModel *TestSliceProxyModel::loadTestTree()
{
    AdocTreeModel *model = new AdocTreeModel(this);

    AdocTreeNode *root = model->root();
    AdocTreeNode *zeta = new AdocTreeNode(AdocTreeNode::GroupType, "Zeta");
    root->appendChild(zeta);
    zeta->appendChild(new AdocTreeNode(AdocTreeNode::MsaAminoType, "Alignment"));
    zeta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Binding"));
    zeta->appendChild(new AdocTreeNode(AdocTreeNode::PrimerType, "Site"));

    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::GroupType, "Alpha");
    root->appendChild(alpha);
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Toxic"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Acid"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Dummy"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Base"));

    AdocTreeNode *delta = new AdocTreeNode(AdocTreeNode::GroupType, "Delta");
    root->appendChild(delta);
    delta->appendChild(new AdocTreeNode(AdocTreeNode::SeqAminoType, "Force"));

    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Beta"));

    AdocTreeNode *gamma = new AdocTreeNode(AdocTreeNode::GroupType, "Gamma");
    root->appendChild(gamma);
    AdocTreeNode *level2 = new AdocTreeNode(AdocTreeNode::GroupType, "Level2");
    gamma->appendChild(level2);
    level2->appendChild(new AdocTreeNode(AdocTreeNode::SubseqRnaType, "Apple"));

    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Kappa"));

    return model;
}

void TestSliceProxyModel::constructor()
{
    MockSliceProxyModel model;

    QVERIFY(model.adocTreeModel_ == 0);
    QVERIFY(model.sourceParent_ == 0);

    MockSliceProxyModel *model2 = new MockSliceProxyModel(this);
    delete model2;
    model2 = 0;
}

/**
  * Largely borrowed from TestVaryingColumnProxyModel
  */
void TestSliceProxyModel::setTreeModel()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Test: default treemodel should be empty
    QVERIFY(mockModel->sourceTreeModel() == 0);

    // Create a couple dummy models for testing that the signals are attached and detached as expected
    AdocTreeModel *alpha = new AdocTreeModel(this);
    AdocTreeModel *beta = new AdocTreeModel(this);

    // ----------------------------------------------------
    // Part 1: Check the reset signal and the virtual signals
    // Setup the signal spies
    QSignalSpy spyAboutToReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyReset(mockModel, SIGNAL(modelReset()));

    // 5 possible scenarios:
    //    Current model     Provided model  Result
    // 1) Null              Null            No change
    // 2) Null              Good            Reset
    // 3) Model1            Model1          No change
    // 4) Model2            Model1          Reset
    // 5) Good              Null            Reset

    // Test: Case 1
    mockModel->setTreeModel(0);
    QVERIFY(mockModel->sourceTreeModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: Case 2
    mockModel->setTreeModel(alpha);
    QVERIFY(mockModel->sourceTreeModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();
    spyAboutToReset.clear();
    spyReset.clear();

    // Test: Case 3
    mockModel->setTreeModel(alpha);
    QVERIFY(mockModel->sourceTreeModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: Case 4
    mockModel->setTreeModel(beta);
    QVERIFY(mockModel->sourceTreeModel() == beta);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();
    spyAboutToReset.clear();
    spyReset.clear();

    // Test: changing from valid to null should trigger the reset signal
    mockModel->setTreeModel(0);
    QVERIFY(mockModel->sourceTreeModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();
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
    MockSliceProxyModel *mockModel2 = new MockSliceProxyModel(this);
    AdocTreeModel *gamma = new AdocTreeModel(this);

    // Verify that no signals have been setup on a default constructed slice model
    QCOMPARE(QObject::disconnect(gamma, 0, mockModel2, 0), false);

    // Test: verify that the appropriate signals have been setup
    mockModel2->setTreeModel(gamma);

    // Only way to verify that the signal handlers were setup is to disconnect them and check if it was true
    // Signals:
    // - dataChanged
    // - layoutAboutToBeChanged
    // - layoutChanged
    // - modelReset
    // - rowsInserted
    // - rowsAboutToBeRemoved
    // - rowsRemoved
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(dataChanged(QModelIndex,QModelIndex)), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(layoutAboutToBeChanged()), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(layoutChanged()), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(modelReset()), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsInserted(QModelIndex,int,int)), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), mockModel2, 0), true);
    QCOMPARE(QObject::disconnect(gamma, SIGNAL(rowsRemoved(QModelIndex,int,int)), mockModel2, 0), true);

    // We know that the signals are hooked up properly when setting a valid source model. When clearing that
    // they should all be disconnected.
    MockSliceProxyModel *mockModel3 = new MockSliceProxyModel(this);
    AdocTreeModel *delta = new AdocTreeModel(this);

    mockModel3->setTreeModel(delta);
    mockModel3->setTreeModel(0);

    QCOMPARE(QObject::disconnect(delta, SIGNAL(dataChanged(QModelIndex,QModelIndex)), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(layoutAboutToBeChanged()), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(layoutChanged()), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(modelReset()), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsInserted(QModelIndex,int,int)), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), mockModel3, 0), false);
    QCOMPARE(QObject::disconnect(delta, SIGNAL(rowsRemoved(QModelIndex,int,int)), mockModel3, 0), false);


    // Final test: changing from a valid model to another valid model
    MockSliceProxyModel *mockModel4 = new MockSliceProxyModel(this);
    AdocTreeModel *epsilon = new AdocTreeModel(this);
    AdocTreeModel *theta = new AdocTreeModel(this);

    mockModel4->setTreeModel(epsilon);
    mockModel4->setTreeModel(theta);

    // Verify all relevant signals are disconnected from epsilon
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(dataChanged(QModelIndex,QModelIndex)), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(layoutAboutToBeChanged()), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(layoutChanged()), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(modelReset()), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsInserted(QModelIndex,int,int)), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), mockModel4, 0), false);
    QCOMPARE(QObject::disconnect(epsilon, SIGNAL(rowsRemoved(QModelIndex,int,int)), mockModel4, 0), false);

    // Verify that all relevant signals are hooked up for theta
    QCOMPARE(QObject::disconnect(theta, SIGNAL(dataChanged(QModelIndex,QModelIndex)), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(layoutAboutToBeChanged()), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(layoutChanged()), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(modelReset()), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsInserted(QModelIndex,int,int)), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), mockModel4, 0), true);
    QCOMPARE(QObject::disconnect(theta, SIGNAL(rowsRemoved(QModelIndex,int,int)), mockModel4, 0), true);
}

void TestSliceProxyModel::setSourceParent()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    QVERIFY(mockModel->sourceParentNode() == 0);

    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup the signal spies
    QSignalSpy spyModelAboutToBeReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(mockModel, SIGNAL(modelReset()));

    // Test: Load slice for the root
    mockModel->virtualSignalCounts_.clear();
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->sourceParentNode(), treeModel->root());

    // MockSliceProxyModel only keeps those rows that begin with [Aa] or [Bb]. So in this case,
    // slice should have two rows - Alpha and Beta
    SliceProxyModel::Slice slice = mockModel->slice_;
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(slice.sourceNodes_.count(), 2);
    QCOMPARE(slice.sourceNodes_.at(0), treeModel->nodeFromIndex(treeModel->index(1, 0)));
    QCOMPARE(slice.sourceNodes_.at(1), treeModel->nodeFromIndex(treeModel->index(3, 0)));
    QCOMPARE(slice.sourceRowIndices_.count(), 2);
    QCOMPARE(slice.sourceRowIndices_.at(0), 1);
    QCOMPARE(slice.sourceRowIndices_.at(1), 3);

    // Verify that the signals were triggered
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 3);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("taggedSliceCreated"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).start_, 0);
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).end_, 1);
    mockModel->virtualSignalCounts_.clear();

    // -----------------------------------
    // Test: setSourceParent with same modelindex should do nothing and not emit any signals
    mockModel->setSourceParent(QModelIndex());
    slice = mockModel->slice_;
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(slice.sourceNodes_.count(), 2);
    QCOMPARE(slice.sourceNodes_.at(0), treeModel->nodeFromIndex(treeModel->index(1, 0)));
    QCOMPARE(slice.sourceNodes_.at(1), treeModel->nodeFromIndex(treeModel->index(3, 0)));
    QCOMPARE(slice.sourceRowIndices_.count(), 2);
    QCOMPARE(slice.sourceRowIndices_.at(0), 1);
    QCOMPARE(slice.sourceRowIndices_.at(1), 3);

    // Verify that the signals were triggered
    QVERIFY(spyModelAboutToBeReset.isEmpty());
    QVERIFY(spyModelReset.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // ----------------------------------
    // Test: setSourceParent with node with no members that match the mock slice model's filtering
    QModelIndex deltaIndex = treeModel->index(2, 0);
    AdocTreeNode *deltaNode = treeModel->nodeFromIndex(deltaIndex);
    QCOMPARE(deltaNode->label_, QString("Delta"));      // Make sure it is a valid node
    mockModel->setSourceParent(deltaIndex);
    QCOMPARE(mockModel->sourceParentNode(), deltaNode);
    slice = mockModel->slice_;
    QCOMPARE(slice.sourceNodes_.count(), 0);
    QCOMPARE(slice.sourceRowIndices_.count(), 0);
    // Verify that the signals were triggered
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node with no children
    QModelIndex kappaIndex = treeModel->index(5, 0);
    AdocTreeNode *kappaNode = treeModel->nodeFromIndex(kappaIndex);
    QCOMPARE(kappaNode->label_, QString("Kappa"));
    mockModel->setSourceParent(kappaIndex);
    QCOMPARE(mockModel->sourceParentNode(), kappaNode);
    slice = mockModel->slice_;
    QCOMPARE(slice.sourceNodes_.count(), 0);
    QCOMPARE(slice.sourceRowIndices_.count(), 0);
    // Verify that the signals were triggered
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node with some children
    QModelIndex alphaIndex = treeModel->index(1, 0);
    AdocTreeNode *alphaNode = treeModel->nodeFromIndex(alphaIndex);
    QCOMPARE(alphaNode->label_, QString("Alpha"));
    mockModel->setSourceParent(alphaIndex);
    QCOMPARE(mockModel->sourceParentNode(), alphaNode);
    slice = mockModel->slice_;
    QCOMPARE(slice.sourceNodes_.count(), 2);
    QCOMPARE(slice.sourceNodes_.at(0), alphaNode->childAt(1));
    QCOMPARE(slice.sourceNodes_.at(1), alphaNode->childAt(3));
    QCOMPARE(slice.sourceRowIndices_.count(), 2);
    QCOMPARE(slice.sourceRowIndices_.at(0), 1);
    QCOMPARE(slice.sourceRowIndices_.at(1), 3);

    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 3);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("taggedSliceCreated"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).start_, 0);
    QCOMPARE(mockModel->virtualSignalCounts_.at(2).end_, 1);
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node should only load immediate children, not grandchildren
    QModelIndex gammaIndex = treeModel->index(4, 0);
    AdocTreeNode *gammaNode = treeModel->nodeFromIndex(gammaIndex);
    QCOMPARE(gammaNode->label_, QString("Gamma"));
    mockModel->setSourceParent(gammaIndex);
    QCOMPARE(mockModel->sourceParentNode(), gammaNode);

    slice = mockModel->slice_;
    QCOMPARE(slice.sourceNodes_.count(), 0);
    QCOMPARE(slice.sourceRowIndices_.count(), 0);

    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();
}

void TestSliceProxyModel::defaultSortColumn()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    QCOMPARE(mockModel->defaultSortColumn(), 0);

    mockModel->setDefaultSortColumn(1);
    QCOMPARE(mockModel->defaultSortColumn(), 1);

    mockModel->setDefaultSortColumn(0);
    QCOMPARE(mockModel->defaultSortColumn(), 0);
}

void TestSliceProxyModel::primaryColumn()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    QCOMPARE(mockModel->primaryColumn(), 0);

    mockModel->setPrimaryColumn(1);
    QCOMPARE(mockModel->primaryColumn(), 1);

    mockModel->setPrimaryColumn(0);
    QCOMPARE(mockModel->primaryColumn(), 0);
}

void TestSliceProxyModel::setTreeModelClearSlices()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup: load some slices
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 2);

    // Test: set a new source tree model
    mockModel->setTreeModel(new AdocTreeModel(this));           // Ignore small memory leak :)
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 0);

    // Test: set an empty source treemodel
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 2);
    mockModel->setTreeModel(0);
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 0);
}

void TestSliceProxyModel::headerData()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    QList<Qt::Orientation> orientations;
    orientations << Qt::Horizontal << Qt::Vertical;
    QList<int> roles;
    roles << Qt::DisplayRole << Qt::EditRole;

    // Test: when there is no tree model configured for the proxy model, headerData should always return an invalid QVariant
    for (int i=-5; i< 5; ++i)
        foreach (Qt::Orientation orientation, orientations)
            foreach (int role, roles)
                QVERIFY(mockModel->headerData(i, orientation, role).isValid() == false);

    // Test: when there is a tree model, it should return the exact value returned by the treeModel for the same parameters
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    for (int i=-5; i< 5; ++i)
        foreach (Qt::Orientation orientation, orientations)
            foreach (int role, roles)
                QCOMPARE(mockModel->headerData(i, orientation, role), treeModel->headerData(i, orientation, role));
}

void TestSliceProxyModel::index()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Test: index without any set treemodel
    QVERIFY(mockModel->index(0, 0).isValid() == false);

    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: before calling setSourceParent there should no be no valid indices whatsoever
    QVERIFY(mockModel->index(0, 0).isValid() == false);
    QVERIFY(mockModel->index(1, 0).isValid() == false);
    QVERIFY(mockModel->index(0, 1).isValid() == false);
    QVERIFY(mockModel->index(1, 1).isValid() == false);

    // Setup: load some slices
    QModelIndex alphaIndex = treeModel->index(1, 0);
    QCOMPARE(alphaIndex.data().toString(), QString("Alpha"));
    QModelIndex level2Index = treeModel->index(0, 0, treeModel->index(4, 0));
    QCOMPARE(level2Index.data().toString(), QString("Level2"));

    mockModel->setSourceParent(QModelIndex());

    // Test: negative numbers for either row or column should fail
    QVERIFY(mockModel->index(-1, 0).isValid() == false);
    QVERIFY(mockModel->index(0, -1).isValid() == false);
    QVERIFY(mockModel->index(-1, -1).isValid() == false);

    // --------------------------------------
    // Test: slice rows - all columns should be valid even though technically those that begin with [Aa]
    // should have one column, and those with [Bb], two columns
    // Subtest: mockRootIndex
    QVERIFY(mockModel->index(0, 0).isValid());   // Alpha
    QVERIFY(mockModel->index(0, 1).isValid());
    QVERIFY(mockModel->index(1, 0).isValid());   // Beta
    QVERIFY(mockModel->index(1, 1).isValid());
    QVERIFY(mockModel->index(1, 2).isValid() == false);
    QVERIFY(mockModel->index(2, 0).isValid() == false);  // Out of rows :)

    // Subtest: mockAlphaIndex
    mockModel->setSourceParent(alphaIndex);
    QVERIFY(mockModel->index(0, 0).isValid());   // Acid
    QVERIFY(mockModel->index(0, 1).isValid());
    QVERIFY(mockModel->index(1, 0).isValid());   // Base
    QVERIFY(mockModel->index(1, 1).isValid());
    QVERIFY(mockModel->index(1, 2).isValid() == false);
    QVERIFY(mockModel->index(2, 0).isValid() == false);  // Out of rows :)

    // Subtest: mockLevel2Index
    mockModel->setSourceParent(level2Index);
    QVERIFY(mockModel->index(0, 0).isValid());   // Apple
    QVERIFY(mockModel->index(0, 1).isValid());
    QVERIFY(mockModel->index(1, 0).isValid() == false); // Out of rows :)

    // --------------------------------------
    // Test: pass parent index from different model is not used and should therefore work
    AdocTreeModel *model2 = loadTestTree();
    QVERIFY(mockModel->index(0, 0, model2->index(0, 0)).isValid());
    QVERIFY(mockModel->index(0, 1, model2->index(1, 0)).isValid());
}

void TestSliceProxyModel::rowCount()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Test: rowCount should be zero
    QCOMPARE(mockModel->rowCount(), 0);

    // Setup: load some slices
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(treeModel->index(4, 0));
    QCOMPARE(mockModel->rowCount(), 0);
    mockModel->setSourceParent(treeModel->index(1, 0));
    QCOMPARE(mockModel->rowCount(), 2);

    // Test: index from another model; should ignore the parent parameter and still
    //       return number of rows in this model
    AdocTreeModel *treeModel2 = loadTestTree();
    QCOMPARE(mockModel->rowCount(treeModel2->index(0, 0)), 2);
}

void TestSliceProxyModel::parent()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Test: uninitialized mock model
    QCOMPARE(mockModel->parent(QModelIndex()), QModelIndex());

    // Setup: load some slices
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(treeModel->index(1, 0)); // Alpha

    // Test: top level index should return invalid qmodelindex parent
    QCOMPARE(mockModel->parent(mockModel->index(0, 0)), QModelIndex());

    // Test: all slice items should return invalid QModelIndex
    QModelIndex mockAlphaIndex = mockModel->index(0, 0);
    QCOMPARE(mockModel->parent(mockModel->index(0, 0, mockAlphaIndex)), QModelIndex());
    QCOMPARE(mockModel->parent(mockModel->index(1, 0, mockAlphaIndex)), QModelIndex());

    // Test: child index from a different model should return invalid model index
    AdocTreeModel *treeModel2 = loadTestTree();
    QCOMPARE(mockModel->parent(treeModel2->index(0, 0, treeModel2->index(1, 0))), QModelIndex());
}

void TestSliceProxyModel::clear()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    QSignalSpy spyAboutToReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyReset(mockModel, SIGNAL(modelReset()));

    // Test: clear on empty model should still emit the reset signals
    mockModel->clear();
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();

    // Test: clear on loaded model should indeed clear out the data structures
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->sourceParentNode(), treeModel->root());

    QCOMPARE(mockModel->rowCount(), 2);

    spyAboutToReset.clear();
    spyReset.clear();
    mockModel->virtualSignalCounts_.clear();
    mockModel->clear();
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();

    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(mockModel->sourceParentNode() == 0);
}

void TestSliceProxyModel::mapToSource()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    AdocTreeModel *treeModel2 = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: root should return invalid index if source parent is empty
    QCOMPARE(mockModel->mapToSource(QModelIndex()), QModelIndex());

    // Test: index from another model should return invalid modelindex
    QCOMPARE(mockModel->mapToSource(treeModel2->index(0, 0)), QModelIndex());

    // Setup
    QModelIndex alphaIndex = treeModel->index(1, 0);
    mockModel->setSourceParent(alphaIndex);

    // Test: root should return its parent model index if setSourceParent has been defined
    QCOMPARE(mockModel->mapToSource(QModelIndex()), alphaIndex);

    // Test: children
    QCOMPARE(mockModel->mapToSource(mockModel->index(0, 0)), alphaIndex.child(1, 0));
    QCOMPARE(mockModel->mapToSource(mockModel->index(1, 0)), alphaIndex.child(3, 0));

    // Test: level2Index
    QModelIndex level2Index = treeModel->index(0, 0, treeModel->index(4, 0));
    mockModel->setSourceParent(level2Index);
    QCOMPARE(mockModel->mapToSource(mockModel->index(0, 0)), level2Index.child(0, 0));
}

void TestSliceProxyModel::flags()
{
    // AdocTreeModel only has one column, so the dataChanged would be for that one column
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: flags with empty slice model
    QCOMPARE(mockModel->index(0, 0).flags(), 0);

    // Test: root index without any parent node configured
    QCOMPARE(mockModel->flags(QModelIndex()), 0);

    // Test: Root index of root adoc tree node, should return zero
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->flags(QModelIndex()), 0);

    // Test: child items of root index should be non-empty
    for (int i=0, z=mockModel->rowCount(); i<z; ++i)
        QCOMPARE(mockModel->index(i, 0).flags(), Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    // Test: Group zeta
    mockModel->setSourceParent(treeModel->index(0));

    // Flags with valid parent, should return non-empty flags for an invalid QModelIndex
    QVERIFY(mockModel->flags(QModelIndex()) != 0);

    // --> first slice item: Alignment
    Qt::ItemFlags flags = mockModel->index(0, 0).flags();
    QVERIFY(flags & Qt::ItemIsEnabled);
    QVERIFY(flags & Qt::ItemIsEditable);
    QVERIFY(flags & Qt::ItemIsSelectable);
    QVERIFY(flags & Qt::ItemIsDragEnabled);
    flags = mockModel->index(0, 1).flags();  // Msa amino nodes do not have any flags for columns outside the primary column
    QVERIFY(flags == 0);

    // --> second slice item: Binding
    flags = mockModel->index(1, 0).flags();
    QVERIFY(flags & Qt::ItemIsEnabled);
    QVERIFY(flags & Qt::ItemIsEditable);
    QVERIFY(flags & Qt::ItemIsSelectable);
    QVERIFY(flags & Qt::ItemIsDragEnabled);
    QCOMPARE(mockModel->index(1, 1).flags(), 0);

    QVERIFY(mockModel->index(2, 0).isValid() == false);
}

void TestSliceProxyModel::removeRows()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Current tree looks like:
    // Root
    // |___ Zeta (Group)
    //      |___ Alignment
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group)
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group)
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)

    // Setup: load a slice
    mockModel->setSourceParent(treeModel->index(0, 0));

    // Test: parent argument is irrelevant, should remove whenever valid range is referenced
    QCOMPARE(mockModel->rowCount(), 2);
    QVERIFY(mockModel->removeRows(0, 2, QModelIndex()));
    QCOMPARE(mockModel->rowCount(), 0);
    QCOMPARE(treeModel->rowCount(treeModel->index(0, 0)), 1);

    // Setup: alpha
    QModelIndex alphaIndex = treeModel->index(1, 0);
    mockModel->setSourceParent(alphaIndex);

    // Test: actual valid row removal - Acid
    QVERIFY(mockModel->removeRows(0, 1));
    QCOMPARE(mockModel->rowCount(), 1);     // Only base is left
    QCOMPARE(treeModel->rowCount(alphaIndex), 3);
    QCOMPARE(alphaIndex.child(0, 0).data().toString(), QString("Toxic"));
    QCOMPARE(alphaIndex.child(1, 0).data().toString(), QString("Dummy"));
    QCOMPARE(alphaIndex.child(2, 0).data().toString(), QString("Base"));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Drag and drop
/**
  * This method tests the model methods called by the drap and drop framework. Ideally, a kind of
  * QAbstractItemView should be created and attached to this model. Then actually simulate drag and
  * drop events.
  *
  * Because we sidestep the above process, it does not 100% reflect what may occur in production code.
  * Specifically, after a successful call to dropMimeData() with Qt::MoveAction, QAbstractItemView
  * will attempt to clear or remove those rows that were selected. For things to behave properly,
  * we manually call the removeRows method.
  */
void TestSliceProxyModel::dragdrop()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    int nonPrimaryColumn = 1;
    QVERIFY(mockModel->primaryColumn() != nonPrimaryColumn);

    // Test those functions that merely reflect/proxy to the treemodel
    QVERIFY(mockModel->supportedDragActions() == treeModel->supportedDragActions());
    QVERIFY(mockModel->supportedDropActions() == treeModel->supportedDropActions());
    QVERIFY(mockModel->mimeTypes() == treeModel->mimeTypes());



    // ------------------------
    // Suite: mimeData function
    // Context: empty slice model
    QVERIFY(mockModel->mimeData(QModelIndexList()) == 0);
    QVERIFY(mockModel->mimeData(QModelIndexList() << QModelIndex()) == 0);

    // Context: root slice loaded
    mockModel->setSourceParent(QModelIndex());

    // Test: mimeData with index from different model
    QVERIFY(mockModel->mimeData(QModelIndexList() << treeModel->index(0, 0)) == 0);

    // Test: slice item (Alpha) should result in non-zero value of mimeData
    QMimeData *mimeData = mockModel->mimeData(QModelIndexList() << mockModel->index(0, 0)); // Alpha
    QVERIFY(mimeData);
    // Check that the indices were converted
    ModelIndexMimeData *myMimeData = static_cast<ModelIndexMimeData *>(mimeData);
    QCOMPARE(myMimeData->indexes_.count(), 1);
    QCOMPARE(myMimeData->indexes_.at(0), treeModel->index(1, 0));
    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Context: Alpha slice loaded
    mockModel->setSourceParent(treeModel->index(1, 0));     // Subroot is now alpha

    // Test: two valid nodes, with one index on a nonPrimaryColumn
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << mockModel->index(0, 0)      // Acid
                                   << mockModel->index(1, 0)      // Base
                                   << mockModel->index(1, nonPrimaryColumn));       // Base with nonPrimaryColumn
    QVERIFY(mimeData == 0);

    // Test: sandwich an invalid QModelIndex between two valid nodes
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << mockModel->index(0, 0)      // Acid
                                   << QModelIndex()
                                   << mockModel->index(1, 0));    // Base
    QVERIFY(!mimeData);

    // Test: two valid nodes both on primaryColumn
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << mockModel->index(0, 0)      // Acid
                                   << mockModel->index(1, 0));    // Base
    QVERIFY(mimeData);
    // Check that the indices were mapped to the tree model equivalents
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);
    QCOMPARE(myMimeData->indexes_.count(), 2);
    QCOMPARE(myMimeData->indexes_.at(0), treeModel->index(1, 0).child(1, 0));
    QCOMPARE(myMimeData->indexes_.at(1), treeModel->index(1, 0).child(3, 0));


    // --------------------------
    // Suite: dropMimeData method
    // Model data context:
    //  << Subroot >> - treeModel->index(1, 0)
    //  |___ [0] Acid  <-- stored in myMimeData
    //  |___ [1] Base  <-- stored in myMimeData

    // Test: parent = invalid index, same parent, slice items
    QModelIndexList indices;
    indices << QModelIndex()
            << mockModel->index(0, 0)
            << mockModel->index(1, 0);
    foreach (const QModelIndex &index, indices)
    {
        // Loop over all columns and even invalid columns
        for (int i=-1; i< 3; ++i)
        {
            for (int j=-1; j< 3; ++j)
            {
                QCOMPARE(mockModel->dropMimeData(0,          Qt::CopyAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(0,          Qt::MoveAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(0,          Qt::LinkAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(0,          Qt::ActionMask,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(0,          Qt::IgnoreAction,      i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(0,          Qt::TargetMoveAction,  i, j, index), false);

                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::CopyAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::LinkAction,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::ActionMask,        i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::IgnoreAction,      i, j, index), false);
                QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::TargetMoveAction,  i, j, index), false);
            }
        }
    }

    // Setup another slice model that maps another node of the tree model (root)
    MockSliceProxyModel *mockModel2 = new MockSliceProxyModel(this);
    mockModel2->setTreeModel(treeModel);
    mockModel2->setSourceParent(QModelIndex());

    // Test: Empty mime data should return false (vary other parameters to explore test space)
    //       mockModel2 only has two rows, so requesting an index for row 2, will return a QModelIndex() which will
    //       attempt to add it to the sourceParent_ node of mockModel2...
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::CopyAction,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::MoveAction,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::LinkAction,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::ActionMask,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::IgnoreAction,      -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(0,          Qt::TargetMoveAction,  -1, -1, mockModel2->index(2, 0)), false);

    // Test: All actions except MoveAction should return false
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::CopyAction,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::LinkAction,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::ActionMask,        -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::IgnoreAction,      -1, -1, mockModel2->index(2, 0)), false);
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::TargetMoveAction,  -1, -1, mockModel2->index(2, 0)), false);

    // Test: dropping onto the same parent should fail. Acid and Base are children of the group alpha (on the tree node).
    //       Since the alpha group is a slice item underneath sliceRootParent, this provides a method of attempting
    //       to place these items under its current parent.
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel2->index(0, 0)), false);


    // ------------------------------
    // Suite: valid dropMimeData calls
    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    mimeData = mockModel->mimeData(QModelIndexList() << mockModel->index(1, 0));    // Base
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    // Current tree looks like:
    // Root [mockModel2]
    // |___ Zeta (Group)
    //      |___ Alignment
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [mockModel]
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base   <-- In mime data; moving this node
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group) <-- to here
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)

    // Artificially set the fkId_ to ensure that it is zeroed out during the move
    treeModel->nodeFromIndex(treeModel->index(1, 0).child(3, 0))->fkId_ = 50;
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel2->index(1, 0)), true);

    // At this point, Base should still exist under Alpha, but have a fkId_ of zero
    QCOMPARE(treeModel->rowCount(treeModel->index(1, 0)), 4);
    QCOMPARE(treeModel->index(1, 0).child(3, 0).data().toString(), QString("Base"));
    QCOMPARE(treeModel->nodeFromIndex(treeModel->index(1, 0).child(3, 0))->fkId_, 0);

    // Remove the rows that were successfully moved (see log.txt, 15 September 2010 for details)
    treeModel->removeRow(3, treeModel->index(1, 0));

    // Check that the rows were moved and updated in the slice model
    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->index(0, 0).data().toString(), QString("Acid"));
    QCOMPARE(mockModel2->rowCount(), 2);

    // Check that the rows were moved in the adoc tree model
    QCOMPARE(treeModel->rowCount(treeModel->index(3, 0)), 1);
    QCOMPARE(treeModel->index(3, 0).child(0, 0).data().toString(), QString("Base"));

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Test: dropping mime data with multiple parents onto any originating parent should fail
    // Current tree looks like:
    // Root [mockModel2]
    // |___ Zeta (Group) [mockModel3] ** after loadSlice
    //      |___ Alignment              <-- Moving this node
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [mockModel]
    //      |___ Toxic
    //      |___ Acid                   <-- and this one
    //      |___ Dummy
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group)                <-- to here
    //      |___ Base
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)
    MockSliceProxyModel *mockModel3 = new MockSliceProxyModel(this);
    mockModel3->setTreeModel(treeModel);
    mockModel3->setSourceParent(treeModel->index(0, 0));

    QCOMPARE(mockModel3->rowCount(), 2);

    // Build custom mimeData from multiple sources - but must contain treeModel indices
    // This is not currently possible with the public API, but perhaps may be at some point
    myMimeData = new ModelIndexMimeData();
    myMimeData->setData("application/x-alignshop", QByteArray());
    myMimeData->indexes_ << treeModel->index(0, 0).child(0, 0)       // "Alignment"
                         << treeModel->index(1, 0).child(1, 0);      // "Acid"
    QVERIFY(myMimeData);

    // Artificially set the fkId_ to ensure that it is zeroed out during the move
    treeModel->nodeFromIndex(treeModel->index(1, 0).child(1, 0))->fkId_ = 100;

    // Test: check that we cannot drop on same parent; sliceRootParent.child(0, 0) == Alpha, which is the parent
    //       of Acid
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel2->index(0, 0)), false);

    // Verify that the rows were not modifed
    QCOMPARE(mockModel3->rowCount(), 2);
    QCOMPARE(mockModel->rowCount(), 1);

    // Test: moving to another parent should work
    QCOMPARE(mockModel2->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel2->index(1, 0)), true);

    // At this point, Alignment and Acid stub nodes should still exist under Alpha
    QCOMPARE(treeModel->rowCount(treeModel->index(0, 0)), 3);
    QCOMPARE(treeModel->index(0, 0).child(0, 0).data().toString(), QString("Alignment"));
    QCOMPARE(treeModel->rowCount(treeModel->index(1, 0)), 3);
    QCOMPARE(treeModel->index(1, 0).child(1, 0).data().toString(), QString("Acid"));
    QCOMPARE(treeModel->nodeFromIndex(treeModel->index(1, 0).child(1, 0))->fkId_, 0);

    // Remove the rows that were successfully moved (see log.txt, 15 September 2010 for details)
    treeModel->removeRow(0, treeModel->index(0, 0));
    treeModel->removeRow(1, treeModel->index(1, 0));

    QCOMPARE(mockModel3->rowCount(), 1);
    QCOMPARE(mockModel->rowCount(), 0);
    QCOMPARE(mockModel2->rowCount(), 2);

    // Check that the tree model was updated properly
    QCOMPARE(treeModel->rowCount(treeModel->index(3, 0)), 3);
    QCOMPARE(treeModel->index(3, 0).child(0, 0).data().toString(), QString("Base"));
    QCOMPARE(treeModel->index(3, 0).child(1, 0).data().toString(), QString("Alignment"));
    QCOMPARE(treeModel->index(3, 0).child(2, 0).data().toString(), QString("Acid"));

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Test: dropping parent group onto a child group should fail
    // Current tree looks like:
    // Root [mockModel2]
    // |___ Zeta (Group) [mockModel3]
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [mockModel]
    //      |___ Toxic
    //      |___ Dummy
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group) [mockModel4]        <-- In mime data
    //      |___ Base
    //      |___ Alignment
    //      |___ Acid
    //      |___ [!BING!]           <-- Insert "Agroup" here
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)

    // Setup add another group
    AdocTreeNode *agroup = new AdocTreeNode(AdocTreeNode::GroupType, "Agroup");
    treeModel->appendRow(agroup, treeModel->index(3, 0));

    MockSliceProxyModel *mockModel4 = new MockSliceProxyModel(this);
    mockModel4->setTreeModel(treeModel);
    mockModel4->setSourceParent(treeModel->index(3, 0));

    // Sanity check that the newly created node is present
    QCOMPARE(treeModel->index(3, 0).child(3, 0).data().toString(), QString("Agroup"));
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << mockModel2->index(1, 0));    // "Beta"
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    // Attempt to drop Beta onto Agroup - this should fail!
    QVERIFY(mockModel4->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel4->index(3, 0)) == false);
    QCOMPARE(mockModel2->rowCount(), 2);

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;



    // Test: drop items that have a parent-child relationship
    // Current tree looks like:
    // Root [mockModel2]
    // |___ Zeta (Group) [mockModel3]
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [mockModel]   <-- attempt to move here
    //      |___ Toxic
    //      |___ Dummy
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group) [mockModel4]    <-- add to mime data
    //      |___ Base                    <-- add to mime data
    //      |___ Alignment
    //      |___ Acid
    //      |___ Agroup
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << mockModel2->index(1, 0)    // "Beta"
                                   << mockModel4->index(0, 0));  // "Base"
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    QVERIFY(mockModel2->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, mockModel2->index(0, 0)) == false);
    QCOMPARE(mockModel->rowCount(), 0);
    QCOMPARE(mockModel4->rowCount(), 4);

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;
}

void TestSliceProxyModel::refreshSourceParent()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(QModelIndex());
    QCOMPARE(mockModel->sourceParentNode(), treeModel->root());
    QVERIFY(mockModel->rowCount() > 0);

    QSignalSpy spyModelReset(mockModel, SIGNAL(modelReset()));
    mockModel->refreshSourceParent();
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();
    QVERIFY(mockModel->rowCount() > 0);
}

void TestSliceProxyModel::isGoodIndex()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(QModelIndex());

    // ------------------------------------------------------------------------
    // Test: invalid QModelIndex should return false
    QCOMPARE(mockModel->isGoodIndex(QModelIndex()), false);

    // Test: index belonging to another model should return false
    QCOMPARE(mockModel->isGoodIndex(treeModel->index(0, 0)), false);

    // Setup: add some rows
    mockModel->setSourceParent(treeModel->index(0, 0));
    int nRows = mockModel->rowCount();
    int nColumns = mockModel->columnCount();
    QVERIFY(nRows > 0);
    QVERIFY(nColumns > 0);

    for (int i=-3; i< nRows+2; ++i)
        for (int j=-3; j< nColumns+2; ++j)
            QCOMPARE(mockModel->isGoodIndex(mockModel->index(i, j)), (i >= 0 && i< nRows && j >= 0 && j< nColumns) ? true : false);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Signal related tests - changes to the source tree model should be appropriately handled and
// dealt with in the SliceProxyModel

/**
  * For now, only test with group type labels, because AdocTreeModel is currently setup to submit
  * database requests when changing the data.
  *
  * Cannot test whether changing non-zero columns
  */
void TestSliceProxyModel::dataChanged()
{
    // AdocTreeModel only has one column, so the dataChanged would be for that one column
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup: signal spies
    QSignalSpy spyDataChanged(mockModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVERIFY(spyDataChanged.isValid());

    // Test: dataChanged should not be propagated period for slice model with no slices loaded
    QVERIFY(treeModel->setData(treeModel->index(0, 0), "ZetaZeta"));
    QVERIFY(spyDataChanged.isEmpty());

    // Test: dataChanged with mapped root; changes to mapped slice items should be evoked :)
    mockModel->setSourceParent(QModelIndex());

    // Subtest: change to non-mapped nodes should not transmit dataChanged signal
    QVERIFY(treeModel->setData(treeModel->index(0, 0), "Zeta"));
    QVERIFY(treeModel->setData(treeModel->index(2, 0), "DeltaDelta"));
    QVERIFY(treeModel->setData(treeModel->index(4, 0), "GammaGamma"));
    QVERIFY(treeModel->setData(treeModel->index(5, 0), "KappaKappa"));
    QVERIFY(spyDataChanged.isEmpty());

    // Subtest: change to mapped nodes should do nothing transmit the dataChanged signal appropriately
    QVERIFY(treeModel->setData(treeModel->index(1, 0), "AlphaAlpha"));
    QCOMPARE(spyDataChanged.count(), 1);
    QList<QVariant> spyArguments = spyDataChanged.takeFirst();
    QModelIndex topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QModelIndex bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QModelIndex mockFirstTopIndex = mockModel->index(0, 0);
    QCOMPARE(topLeft, mockModel->index(0, 0, mockFirstTopIndex));
    QCOMPARE(bottomRight, mockModel->index(0, 0, mockFirstTopIndex));
    spyDataChanged.clear();

    QVERIFY(treeModel->setData(treeModel->index(3, 0), "BetaBeta"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
    bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QCOMPARE(topLeft, mockModel->index(1, 0, mockFirstTopIndex));
    QCOMPARE(bottomRight, mockModel->index(1, 0, mockFirstTopIndex));
    spyDataChanged.clear();

    // ----------------------------------
    // During testing (#define TESTING), TestSliceProxyModel is a friend class of AdocTreeModel, which
    // provides easier improved testing accessibility. Specifically, we can emit the dataChanged
    // signal without having to explicitly call setData and worry with the specific implementation
    // semantics.
    //
    // Nonetheless, to ensure that using the AdocTreeModel interface also works with this class
    // we still call the setData methods above. Having demonstrated that it works sufficiently, we
    // now utilize this shortcut for the remaining tests.

    // Test: changing non-zero column should not be transmitted
    emit treeModel->dataChanged(treeModel->index(1, 1), treeModel->index(1, 1));
    QCOMPARE(spyDataChanged.count(), 0);
}

void TestSliceProxyModel::modelReset()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Setup
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->setSourceParent(treeModel->index(0, 0));
    QCOMPARE(mockModel->rowCount(), 2);

    // Signal spies
    QSignalSpy spyModelAboutToBeReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(mockModel, SIGNAL(modelReset()));
    QVERIFY(spyModelAboutToBeReset.isValid());
    QVERIFY(spyModelReset.isValid());

    // Test: change the treeModel parent root, which will trigger a reset that can be tested here
    QVERIFY(treeModel->setRoot(new AdocTreeNode(AdocTreeNode::RootType, "Root")));

    // Check the signals
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    // Verify that the mockModel does not contain any rows
    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(mockModel->slice_.sourceNodes_.isEmpty());
}

void TestSliceProxyModel::insertRow()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);

    // Setup
    AdocTreeModel *treeModel = loadTestTree();

    // Signal spies
    QList<QVariant> spyArguments;
    QModelIndex spyIndex;
    QSignalSpy spyRowsAboutToBeInserted(mockModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy spyRowsInserted(mockModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeInserted.isValid());
    QVERIFY(spyRowsInserted.isValid());

    // Test: insertion into tree model that is NOT attached to the slice model should result in
    //       no mock rows inserted signals
    mockModel->virtualSignalCounts_.clear();
    treeModel->insertRow(6, new AdocTreeNode(AdocTreeNode::GroupType, "Theta"));
    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: insertion into attached tree model without any mapped slices should not emit any signals
    mockModel->setTreeModel(treeModel);
    mockModel->virtualSignalCounts_.clear();
    treeModel->insertRow(7, new AdocTreeNode(AdocTreeNode::GroupType, "Eta"));
    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: attached tree model, loaded root slice, insert non-mapped beyond any mapped slices no emit any signals
    mockModel->setSourceParent(QModelIndex());    // This will actually trigger insert row signals
    // Clear them so we don't get a false positive on the tree insertion
    mockModel->virtualSignalCounts_.clear();
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->rowCount(), 2);
    treeModel->insertRow(8, new AdocTreeNode(AdocTreeNode::GroupType, "Iota"));
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: insert into tree model at position 0, non-mapped node - should update the parent row indices
    // Verify intial state
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 1);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 3);
    treeModel->insertRow(0, new AdocTreeNode(AdocTreeNode::GroupType, "Pi"));
    QCOMPARE(mockModel->rowCount(), 2);
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    // Verify state changed appropriately
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 2);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 4);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: insert mapped node into tree model at position 0
    AdocTreeNode *alignerNode = new AdocTreeNode(AdocTreeNode::GroupType, "Aligner");
    treeModel->insertRow(0, alignerNode);
    QCOMPARE(mockModel->rowCount(), 3);
    // Verify that signals were emitted showing that row was appended
    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyArguments = spyRowsInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("taggedSliceCreated"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).start_, 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).end_, 2);
    mockModel->virtualSignalCounts_.clear();

    // Verify that alignerNode was appended to mappedNodes at third position
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 3);
    QVERIFY(mockModel->slice_.sourceNodes_.at(2) == alignerNode);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 3);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 5);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(2), 0);

    // Test: insert mapped node at end of tree model
    AdocTreeNode *blastNode = new AdocTreeNode(AdocTreeNode::GroupType, "BLAST");
    treeModel->insertRow(treeModel->rowCount(), blastNode);
    QCOMPARE(mockModel->rowCount(), 4);
    // Verify that signals were emitted showing that row was appended
    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyArguments = spyRowsInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("taggedSliceCreated"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).start_, 3);
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).end_, 3);
    mockModel->virtualSignalCounts_.clear();

    // Verify that alignerNode was appended to mappedNodes at third position
    QCOMPARE(mockModel->slice_.sourceNodes_.count(), 4);
    QVERIFY(mockModel->slice_.sourceNodes_.at(3) == blastNode);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 3);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 5);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(2), 0);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(3), treeModel->rowCount() - 1);
}

void TestSliceProxyModel::removeRow()
{
    MockSliceProxyModel *mockModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = loadTestTree();

    // Signal spies
    QList<QVariant> spyArguments;
    QModelIndex spyIndex;
    QSignalSpy spyRowsAboutToBeRemoved(mockModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyRowsRemoved(mockModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeRemoved.isValid());
    QVERIFY(spyRowsRemoved.isValid());

    // -------------------------------------
    // Test: unattached tree
    treeModel->removeRow(5, QModelIndex()); // Removing kappa
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->slice_.sourceNodes_.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());
    // add "Kappa" back in
    treeModel->appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Kappa"), QModelIndex());

    // -------------------------------------
    // Test: attached tree with no slices loaded, removing "Force"
    //
    // Root
    // |___ Zeta
    //      |___ Alignment
    //      |___ Binding
    //      |___ Site
    // |___ Alpha
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Delta
    //      |___ Force    <--- TBR
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa
    mockModel->setTreeModel(treeModel);
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(0, treeModel->index(2, 0));
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->slice_.sourceNodes_.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // -------------------------------------
    // Test: attached tree, loaded slice, unmapped leaf row from unmapped parent chain (Delta)
    //
    // Root
    // |___ Zeta
    //      |___ Alignment
    //      |___ Binding
    //      |___ Site
    // |___ Alpha
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Delta       <--- TBR
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa *1
    mockModel->setSourceParent(treeModel->index(treeModel->rowCount() - 1, 0, QModelIndex()));
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(2, QModelIndex());
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->slice_.sourceNodes_.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // -------------------------------------
    // Test: attached tree, loaded slice, mapped parent with mapped children, unmapped leaf row after at least one mapped nodes
    //       >> "Sites"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    //      |___ Site        <--- TBR
    // |___ Alpha
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa *1
    MockSliceProxyModel *mockModel2 = new MockSliceProxyModel(this);
    mockModel2->setTreeModel(treeModel);
    mockModel2->setSourceParent(treeModel->index(0, 0));    // Zeta
    mockModel2->virtualSignalCounts_.clear();
    QSignalSpy spyRowsAboutToBeRemoved2(mockModel2, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyRowsRemoved2(mockModel2, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(2, treeModel->index(0, 0));    // "Site"
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    QVERIFY(spyRowsAboutToBeRemoved2.isEmpty());
    QVERIFY(spyRowsRemoved2.isEmpty());
    QVERIFY(mockModel2->virtualSignalCounts_.isEmpty());

    // At this point, mockModel is mapped to Kappa and mockModel2 is mapped to Zeta
    QCOMPARE(mockModel->rowCount(), 0);
    QCOMPARE(mockModel2->rowCount(), 2);    // Contains Alignment and Binding

    // -------------------------------------
    // Test: attached tree, loaded slice, mapped parent with mapped children, unmapped leaf row before mapped nodes
    //       "Toxic"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *1
    //      |___ Toxic      <--- TBR
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa
    mockModel->setSourceParent(treeModel->index(1, 0));       // "Alpha"
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(0, treeModel->index(1, 0));    // Removing "Toxic"
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Verify that the sourceRowIndices in the slice model were updated accordingly
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 0);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 2);

    // -------------------------------------
    // Test: attached tree, loaded slice, mapped parent with mapped children, unmapped leaf row in middle mapped nodes
    //       "Dummy" under "Alpha"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *1
    //      |___ Acid
    //      |___ Dummy      <--- TBR
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa
    treeModel->removeRow(1, treeModel->index(1, 0));
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 0);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(1), 1);

    // -------------------------------------
    // Test: attached tree, mapped slice node occurs prior to other mapped slice nodes
    //       "Acid"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *1
    //      |___ Acid       <--- TBR
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa
    treeModel->removeRow(0, treeModel->index(1, 0));

    // Check slice model internal state
    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->slice_.sourceRowIndices_.at(0), 0);
    QCOMPARE(mockModel->slice_.sourceNodes_.at(0)->label_, QString("Base"));

    // Check that the signals were emitted properly
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
    spyArguments = spyRowsRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), spyIndex);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceRowsAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).start_, 0);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).end_, 0);

    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("sliceRowsRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).start_, 0);
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).end_, 0);
    mockModel->virtualSignalCounts_.clear();


    // -------------------------------------
    // Test: attached tree, mapped level2, remove unmapped grandparent of mapped slice (e.g. Gamma)
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *3
    //      |___ Base
    // |___ Beta
    // |___ Gamma               <--- TBR
    //      |____ Level2 *1
    //            |____ Apple
    // |___ Kappa
    QModelIndex gammaIndex = treeModel->index(3, 0);
    QCOMPARE(gammaIndex.data().toString(), QString("Gamma"));
    mockModel->setSourceParent(treeModel->index(0, 0, gammaIndex));

    QSignalSpy spyModelReset(mockModel, SIGNAL(modelReset()));

    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->slice_.sourceNodes_.at(0)->label_, QString("Apple"));

    // Removing "Gamma"
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(gammaIndex.row(), QModelIndex());

    // Check internal state of mock Model - it should have removed the slice
    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(mockModel->sourceParent_ == 0);

    // Check signals
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceCleared"));
    mockModel->virtualSignalCounts_.clear();

    // -------------------------------------
    // Test: remove of mapped top level node, an item of which is also mapped
    //
    // Root
    // |___ Zeta *2             <--- TBR
    //      |___ Alignment
    //      |___ Binding *1
    // |___ Alpha
    //      |___ Base
    // |___ Beta
    // |___ Kappa
    QSignalSpy spyModelReset2(mockModel2, SIGNAL(modelReset()));

    mockModel->setSourceParent(treeModel->index(1, 0, treeModel->index(0, 0)));   // Loading "Binding" which is a child of zeta in the tree model (also mapped at this point)
    mockModel->virtualSignalCounts_.clear();
    spyModelReset.clear();
    // Remember the slice pointers for validating the virtual calls
    treeModel->removeRow(0, QModelIndex());

    // Verify internal state of slicemodel
    QCOMPARE(mockModel->rowCount(), 0);
    QCOMPARE(mockModel2->rowCount(), 0);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();
    QCOMPARE(spyModelReset2.count(), 1);
    spyModelReset.clear();
}

QTEST_MAIN(TestSliceProxyModel)
#include "TestSliceProxyModel.moc"
