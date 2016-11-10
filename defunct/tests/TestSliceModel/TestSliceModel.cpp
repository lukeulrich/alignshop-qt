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
#include "models/SliceModel.h"
#include "TreeNode.h"

#include "MockSliceModel.h"

#include <QtDebug>

Q_DECLARE_METATYPE(QModelIndex)

// ------------------------------------------------------------------------------------------------
// Core test class
class TestSliceModel : public QObject
{
    Q_OBJECT

public:
    TestSliceModel()
    {
        // For the signal spy to work with model indices
        qRegisterMetaType<QModelIndex>("QModelIndex");
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Test methods
    void constructor();
    void setTreeModel();

    void loadSlice();
    void releaseSlice();

    void defaultSortColumn();

    // This method tests that setting the tree model also clears any loaded slices
    void setTreeModelClearSlices();

    void index();
    void rowCount();
    void parent();
    void clear();
    void indexFromNode();
    void mapToSource();
    void flags();

    void removeRows();

    // Drag and drop
    void dragdrop();

    // -------------------------------------------------
    // Signal related tests
    void dataChanged();
    void modelReset();
    void insertRow();
    void removeRow();

    // -------------------------------------------------
    // Protected functions
    void indexFromSlice();

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
AdocTreeModel *TestSliceModel::loadTestTree()
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

void TestSliceModel::constructor()
{
    MockSliceModel model;

    QVERIFY(model.adocTreeModel_ == 0);
    QVERIFY(model.mappedNodes_.isEmpty());
    QVERIFY(model.sliceHash_.isEmpty());
    QVERIFY(model.mappedNodesToRemove_.isEmpty());
    QVERIFY(model.sliceRowsToRemove_.isEmpty());

    MockSliceModel *model2 = new MockSliceModel(this);
    delete model2;
    model2 = 0;
}

/**
  * Largely borrowed from TestVaryingColumnProxyModel
  */
void TestSliceModel::setTreeModel()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    // Test: default treemodel should be empty
    QVERIFY(mockModel->adocTreeModel() == 0);

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
    QVERIFY(mockModel->adocTreeModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: Case 2
    mockModel->setTreeModel(alpha);
    QVERIFY(mockModel->adocTreeModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    spyAboutToReset.clear();
    spyReset.clear();
    mockModel->virtualSignalCounts_.clear();

    // Test: Case 3
    mockModel->setTreeModel(alpha);
    QVERIFY(mockModel->adocTreeModel() == alpha);
    QCOMPARE(spyAboutToReset.count(), 0);
    QCOMPARE(spyReset.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: Case 4
    mockModel->setTreeModel(beta);
    QVERIFY(mockModel->adocTreeModel() == beta);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    spyAboutToReset.clear();
    spyReset.clear();
    mockModel->virtualSignalCounts_.clear();

    // Test: changing from valid to null should trigger the reset signal
    mockModel->setTreeModel(0);
    QVERIFY(mockModel->adocTreeModel() == 0);
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    spyAboutToReset.clear();
    spyReset.clear();
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------------------------
    // Part 2: check that the appropriate signals are hooked up and disconnect properly
    //
    // Because there is no direct way to query the signals that are hooked up to an object, the following approach
    // was used. Disconnect will only return true if the signal was disconnected successfully. For this to happen, it
    // must have existed in the first place. Thus by checking the return value of disconnect we can in essence check
    // if the signal was attached correctly.
    //
    // A side effect is that the object is no longer in an expected state.
    MockSliceModel *mockModel2 = new MockSliceModel(this);
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
    MockSliceModel *mockModel3 = new MockSliceModel(this);
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
    MockSliceModel *mockModel4 = new MockSliceModel(this);
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

void TestSliceModel::loadSlice()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup the signal spies
    QList<QVariant> spyArguments;
    QSignalSpy spyRowsAboutToBeInserted(mockModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy spyRowsInserted(mockModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeInserted.isValid());
    QVERIFY(spyRowsInserted.isValid());


    // Test: Load slice for the root
    mockModel->virtualSignalCounts_.clear();
    mockModel->loadSlice(QModelIndex());

    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), treeModel->root());
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(treeModel->root()));
    Slice *slice = mockModel->sliceHash_[treeModel->root()];

    // MockSliceModel only keeps those rows that begin with [Aa] or [Bb]. So in this case,
    // slice should have two rows - Alpha and Beta
    QCOMPARE(slice->rows_.count(), 2);
    QCOMPARE(slice->refCount_, 1);
    QCOMPARE(slice->sourceNodes_.count(), 2);
    QCOMPARE(slice->sourceNodes_.at(0), treeModel->nodeFromIndex(treeModel->index(1, 0)));
    QCOMPARE(slice->sourceNodes_.at(1), treeModel->nodeFromIndex(treeModel->index(3, 0)));
    QCOMPARE(slice->sourceRowIndices_.count(), 2);
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);

    // Verify that the signals were triggered
    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);

    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);

    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice);
    mockModel->virtualSignalCounts_.clear();

    // -----------------------------------
    // Test: loadSlice with same modelindex should increase the refCount, but not change the data
    //       nor emit any more signals
    mockModel->loadSlice(QModelIndex());

    // Re-check the internal data structures
    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), treeModel->root());
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(treeModel->root()));
    QCOMPARE(slice, mockModel->sliceHash_[treeModel->root()]);
    QCOMPARE(slice->rows_.count(), 2);
    QCOMPARE(slice->refCount_, 2);                  // <--- Only field that should be different
    QCOMPARE(slice->sourceNodes_.count(), 2);
    QCOMPARE(slice->sourceNodes_.at(0), treeModel->nodeFromIndex(treeModel->index(1, 0)));
    QCOMPARE(slice->sourceNodes_.at(1), treeModel->nodeFromIndex(treeModel->index(3, 0)));
    QCOMPARE(slice->sourceRowIndices_.count(), 2);
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);

    QCOMPARE(spyRowsAboutToBeInserted.count(), 0);
    QCOMPARE(spyRowsInserted.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // ----------------------------------
    // Test: load another slice from a node with no members that match the mock slice model's filtering
    QModelIndex deltaIndex = treeModel->index(2, 0);
    AdocTreeNode *deltaNode = treeModel->nodeFromIndex(deltaIndex);
    QCOMPARE(deltaNode->label_, QString("Delta"));      // Make sure it is a valid node
    mockModel->loadSlice(deltaIndex);
    QCOMPARE(mockModel->mappedNodes_.count(), 2);
    QCOMPARE(mockModel->mappedNodes_.at(1), deltaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 2);
    QVERIFY(mockModel->sliceHash_.contains(deltaNode));
    Slice *slice2 = mockModel->sliceHash_[deltaNode];
    QCOMPARE(slice2->rows_.count(), 0);
    QCOMPARE(slice2->refCount_, 1);
    QCOMPARE(slice2->sourceNodes_.count(), 0);
    QCOMPARE(slice2->sourceRowIndices_.count(), 0);

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice2);
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node with no children
    QModelIndex kappaIndex = treeModel->index(5, 0);
    AdocTreeNode *kappaNode = treeModel->nodeFromIndex(kappaIndex);
    QCOMPARE(kappaNode->label_, QString("Kappa"));
    mockModel->loadSlice(kappaIndex);
    QCOMPARE(mockModel->mappedNodes_.count(), 3);
    QCOMPARE(mockModel->mappedNodes_.at(2), kappaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 3);
    QVERIFY(mockModel->sliceHash_.contains(kappaNode));
    Slice *slice3 = mockModel->sliceHash_[kappaNode];
    QCOMPARE(slice3->rows_.count(), 0);
    QCOMPARE(slice3->refCount_, 1);
    QCOMPARE(slice3->sourceNodes_.count(), 0);
    QCOMPARE(slice3->sourceRowIndices_.count(), 0);

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice3);
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node with some children
    QModelIndex alphaIndex = treeModel->index(1, 0);
    AdocTreeNode *alphaNode = treeModel->nodeFromIndex(alphaIndex);
    QCOMPARE(alphaNode->label_, QString("Alpha"));
    mockModel->loadSlice(alphaIndex);

    QCOMPARE(mockModel->mappedNodes_.count(), 4);
    QCOMPARE(mockModel->mappedNodes_.at(3), alphaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 4);
    QVERIFY(mockModel->sliceHash_.contains(alphaNode));
    Slice *slice4 = mockModel->sliceHash_[alphaNode];
    QCOMPARE(slice4->rows_.count(), 2);
    QCOMPARE(slice4->refCount_, 1);
    QCOMPARE(slice4->sourceNodes_.count(), 2);
    QCOMPARE(slice4->sourceNodes_.at(0), alphaNode->childAt(1));
    QCOMPARE(slice4->sourceNodes_.at(1), alphaNode->childAt(3));
    QCOMPARE(slice4->sourceRowIndices_.count(), 2);
    QCOMPARE(slice4->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice4->sourceRowIndices_.at(1), 3);

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice4);
    mockModel->virtualSignalCounts_.clear();

    // ----------------------------------
    // Test: load slice from a node should only load immediate children, not grandchildren
    QModelIndex gammaIndex = treeModel->index(4, 0);
    AdocTreeNode *gammaNode = treeModel->nodeFromIndex(gammaIndex);
    QCOMPARE(gammaNode->label_, QString("Gamma"));
    mockModel->loadSlice(gammaIndex);

    QCOMPARE(mockModel->mappedNodes_.count(), 5);
    QCOMPARE(mockModel->mappedNodes_.at(4), gammaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 5);
    QVERIFY(mockModel->sliceHash_.contains(gammaNode));
    Slice *slice5 = mockModel->sliceHash_[gammaNode];
    QCOMPARE(slice5->rows_.count(), 0);
    QCOMPARE(slice5->refCount_, 1);
    QCOMPARE(slice5->sourceNodes_.count(), 0);
    QCOMPARE(slice5->sourceRowIndices_.count(), 0);

    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsInserted.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice5);
    mockModel->virtualSignalCounts_.clear();
}

void TestSliceModel::releaseSlice()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup the signal spies
    QList<QVariant> spyArguments;
    QSignalSpy spyRowsAboutToBeRemoved(mockModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyRowsRemoved(mockModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeRemoved.isValid());
    QVERIFY(spyRowsRemoved.isValid());

    // -----------------------------------------------------
    // Test: release unloaded slice - should do nothing
    mockModel->virtualSignalCounts_.clear();
    mockModel->releaseSlice(QModelIndex());
    QVERIFY(mockModel->mappedNodes_.isEmpty());
    QVERIFY(mockModel->sliceHash_.isEmpty());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // -----------------------------------------------------
    // Test: Load and release slice for the root
    mockModel->loadSlice(QModelIndex());
    // Verify that the slice is present
    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), treeModel->root());
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(treeModel->root()));
    Slice *mockRootSlice = mockModel->sliceHash_[treeModel->root()];

    mockModel->virtualSignalCounts_.clear();
    mockModel->releaseSlice(QModelIndex());
    QVERIFY(mockModel->mappedNodes_.isEmpty());
    QVERIFY(mockModel->sliceHash_.isEmpty());

    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
    spyArguments = spyRowsRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, mockRootSlice);
    mockModel->virtualSignalCounts_.clear();

    // -----------------------------------------------------
    // Test: calling releaseSlice multiple times should not be a problem, nor emit any signals
    mockModel->releaseSlice(QModelIndex());
    mockModel->releaseSlice(QModelIndex());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // ----------------------------------------------------------------
    // Test: Number of calls to loadSlice should require number of times to call releaseSlice
    QModelIndex alphaIndex = treeModel->index(1, 0);
    AdocTreeNode *alphaNode = treeModel->nodeFromIndex(alphaIndex);
    QCOMPARE(alphaNode->label_, QString("Alpha"));

    mockModel->loadSlice(alphaIndex);
    mockModel->loadSlice(alphaIndex);
    mockModel->loadSlice(alphaIndex);
    Slice *slice = mockModel->sliceHash_[mockModel->mappedNodes_.at(0)];
    QCOMPARE(slice->refCount_, 3);

    mockModel->virtualSignalCounts_.clear();
    mockModel->releaseSlice(alphaIndex);
    // Verify the contents
    QCOMPARE(slice->refCount_, 2);
    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), alphaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(alphaNode));
    QCOMPARE(slice->rows_.count(), 2);
    QCOMPARE(slice->sourceNodes_.count(), 2);
    QCOMPARE(slice->sourceNodes_.at(0), alphaNode->childAt(1));
    QCOMPARE(slice->sourceNodes_.at(1), alphaNode->childAt(3));
    QCOMPARE(slice->sourceRowIndices_.count(), 2);
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);

    QCOMPARE(spyRowsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());


    mockModel->releaseSlice(alphaIndex);
    // Verify the contents
    QCOMPARE(slice->refCount_, 1);
    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), alphaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(alphaNode));
    QCOMPARE(slice->rows_.count(), 2);
    QCOMPARE(slice->sourceNodes_.count(), 2);
    QCOMPARE(slice->sourceNodes_.at(0), alphaNode->childAt(1));
    QCOMPARE(slice->sourceNodes_.at(1), alphaNode->childAt(3));
    QCOMPARE(slice->sourceRowIndices_.count(), 2);
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);

    QCOMPARE(spyRowsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyRowsRemoved.count(), 0);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    mockModel->releaseSlice(alphaIndex);
    QVERIFY(mockModel->mappedNodes_.isEmpty());
    QVERIFY(mockModel->sliceHash_.isEmpty());
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
    spyArguments = spyRowsRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice);
    mockModel->virtualSignalCounts_.clear();

    // Pointer is no longer valid
    slice = 0;

    // -----------------------------------------------------
    // Final test, releasing a specific slice from among multiple loaded slices
    QModelIndex zetaIndex = treeModel->index(0, 0);
    mockModel->loadSlice(zetaIndex);
    mockModel->loadSlice(alphaIndex);

    mockModel->releaseSlice(zetaIndex);
    QCOMPARE(mockModel->mappedNodes_.count(), 1);
    QCOMPARE(mockModel->mappedNodes_.at(0), alphaNode);
    QCOMPARE(mockModel->sliceHash_.count(), 1);
    QVERIFY(mockModel->sliceHash_.contains(alphaNode));
    slice = mockModel->sliceHash_[alphaNode];
    QCOMPARE(slice->rows_.count(), 2);
    QCOMPARE(slice->sourceNodes_.count(), 2);
    QCOMPARE(slice->sourceNodes_.at(0), alphaNode->childAt(1));
    QCOMPARE(slice->sourceNodes_.at(1), alphaNode->childAt(3));
    QCOMPARE(slice->sourceRowIndices_.count(), 2);
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);

    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    QCOMPARE(spyRowsRemoved.count(), 1);
    spyArguments = spyRowsRemoved.takeFirst();
    QVERIFY(qvariant_cast<QModelIndex>(spyArguments.at(0)).isValid() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
}

void TestSliceModel::defaultSortColumn()
{
    MockSliceModel mockModel;

    QCOMPARE(mockModel.defaultSortColumn(), 0);
}

void TestSliceModel::setTreeModelClearSlices()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup: load some slices
    mockModel->loadSlice(QModelIndex());
    QModelIndex alphaIndex = treeModel->index(1, 0);
    mockModel->loadSlice(alphaIndex);

    QCOMPARE(mockModel->mappedNodes_.count(), 2);
    QCOMPARE(mockModel->sliceHash_.count(), 2);

    // Test: set a new source tree model
    mockModel->setTreeModel(new AdocTreeModel(this));
    QCOMPARE(mockModel->mappedNodes_.count(), 0);
    QCOMPARE(mockModel->sliceHash_.count(), 0);

    // Test: set an empty source treemodel
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(QModelIndex());
    mockModel->loadSlice(alphaIndex);
    QCOMPARE(mockModel->mappedNodes_.count(), 2);
    QCOMPARE(mockModel->sliceHash_.count(), 2);
    mockModel->setTreeModel(0);
    QCOMPARE(mockModel->mappedNodes_.count(), 0);
    QCOMPARE(mockModel->sliceHash_.count(), 0);
}

void TestSliceModel::index()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    // Test: index without any set treemodel
    QVERIFY(mockModel->index(0, 0).isValid() == false);

    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: before calling loadSlice there should no be no valid indicies whatsoever
    QVERIFY(mockModel->index(0, 0).isValid() == false);
    QVERIFY(mockModel->index(1, 0).isValid() == false);
    QVERIFY(mockModel->index(0, 1).isValid() == false);
    QVERIFY(mockModel->index(1, 1).isValid() == false);

    // Setup: load some slices
    QModelIndex alphaIndex = treeModel->index(1, 0);
    QCOMPARE(alphaIndex.data().toString(), QString("Alpha"));
    QModelIndex level2Index = treeModel->index(0, 0, treeModel->index(4, 0));
    QCOMPARE(level2Index.data().toString(), QString("Level2"));

    mockModel->loadSlice(QModelIndex());
    mockModel->loadSlice(alphaIndex);
    mockModel->loadSlice(level2Index);

    // Test: negative numbers for either row or column should fail
    QVERIFY(mockModel->index(-1, 0).isValid() == false);
    QVERIFY(mockModel->index(0, -1).isValid() == false);
    QVERIFY(mockModel->index(-1, -1).isValid() == false);

    // --------------------------------------
    // Test: Top level items, should be able to create indices for rows 0..2 with root QModelIndex
    QModelIndex mockRootIndex = mockModel->index(0, 0);
    QModelIndex mockAlphaIndex = mockModel->index(1, 0);
    QModelIndex mockLevel2Index = mockModel->index(2, 0);

    QVERIFY(mockRootIndex.isValid());
    QVERIFY(mockAlphaIndex.isValid());
    QVERIFY(mockLevel2Index.isValid());

    // 4th and higher should return invalid model index
    QVERIFY(mockModel->index(-1, 0).isValid() == false);
    QVERIFY(mockModel->index(4, 0).isValid() == false);

    // --------------------------------------
    // Test: slice rows - all columns should be valid even though technically those that begin with [Aa]
    // should have one column, and those with [Bb], two columns
    // Subtest: mockRootIndex
    QVERIFY(mockModel->index(0, 0, mockRootIndex).isValid());   // Alpha
    QVERIFY(mockModel->index(0, 1, mockRootIndex).isValid());
    QVERIFY(mockModel->index(1, 0, mockRootIndex).isValid());   // Beta
    QVERIFY(mockModel->index(1, 1, mockRootIndex).isValid());
    QVERIFY(mockModel->index(1, 2, mockRootIndex).isValid() == false);
    QVERIFY(mockModel->index(2, 0, mockRootIndex).isValid() == false);  // Out of rows :)

    // Subtest: mockAlphaIndex
    QVERIFY(mockModel->index(0, 0, mockAlphaIndex).isValid());   // Acid
    QVERIFY(mockModel->index(0, 1, mockAlphaIndex).isValid());
    QVERIFY(mockModel->index(1, 0, mockAlphaIndex).isValid());   // Base
    QVERIFY(mockModel->index(1, 1, mockAlphaIndex).isValid());
    QVERIFY(mockModel->index(1, 2, mockAlphaIndex).isValid() == false);
    QVERIFY(mockModel->index(2, 0, mockAlphaIndex).isValid() == false);  // Out of rows :)

    // Subtest: mockLevel2Index
    QVERIFY(mockModel->index(0, 0, mockLevel2Index).isValid());   // Apple
    QVERIFY(mockModel->index(0, 1, mockLevel2Index).isValid());
    QVERIFY(mockModel->index(1, 0, mockLevel2Index).isValid() == false); // Out of rows :)

    // --------------------------------------
    // Test: pass parent index from different model
    AdocTreeModel *model2 = loadTestTree();
    QVERIFY(mockModel->index(0, 0, model2->index(0, 0)).isValid() == false);
    QVERIFY(mockModel->index(1, 0, model2->index(1, 0)).isValid() == false);
}

void TestSliceModel::rowCount()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    // Test: rowCount should be zero
    QCOMPARE(mockModel->rowCount(), 0);

    // Setup: load some slices
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(treeModel->index(1, 0));
    mockModel->loadSlice(treeModel->index(4, 0));

    // Test: number of top level items
    QCOMPARE(mockModel->rowCount(), 2);

    // Slice row counts
    QCOMPARE(mockModel->rowCount(mockModel->index(0, 0)), 2);
    QCOMPARE(mockModel->rowCount(mockModel->index(1, 0)), 0);

    // Test: index from another model
    AdocTreeModel *treeModel2 = loadTestTree();
    QCOMPARE(mockModel->rowCount(treeModel2->index(0, 0)), 0);
}

void TestSliceModel::parent()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    // Test: uninitialized mock model
    QCOMPARE(mockModel->parent(QModelIndex()), QModelIndex());

    // Setup: load some slices
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(treeModel->index(1, 0));   // Alpha
    mockModel->loadSlice(treeModel->index(4, 0));   // Gamma

    // Test: top level index should return invalid qmodelindex parent
    QCOMPARE(mockModel->parent(mockModel->index(0, 0)), QModelIndex());

    // Test: slice item should return the proper top level parent
    QModelIndex mockAlphaIndex = mockModel->index(0, 0);
    QCOMPARE(mockModel->parent(mockModel->index(0, 0, mockAlphaIndex)), mockAlphaIndex);
    QCOMPARE(mockModel->parent(mockModel->index(1, 0, mockAlphaIndex)), mockAlphaIndex);

    // Test: child index from a different model should return invalid model index
    AdocTreeModel *treeModel2 = loadTestTree();
    QCOMPARE(mockModel->parent(treeModel2->index(0, 0, treeModel2->index(1, 0))), QModelIndex());
}

void TestSliceModel::clear()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    QSignalSpy spyAboutToReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyReset(mockModel, SIGNAL(modelReset()));

    // Test: clear on empty model should still emit the reset signals
    mockModel->clear();
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    spyAboutToReset.clear();
    spyReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    mockModel->virtualSignalCounts_.clear();

    // Test: clear on loaded model should indeed clear out the data structures
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(QModelIndex());
    mockModel->loadSlice(treeModel->index(0, 0));

    QCOMPARE(mockModel->rowCount(), 2);

    spyAboutToReset.clear();
    spyReset.clear();
    mockModel->virtualSignalCounts_.clear();
    mockModel->clear();
    QCOMPARE(spyAboutToReset.count(), 1);
    QCOMPARE(spyReset.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    mockModel->virtualSignalCounts_.clear();

    spyAboutToReset.clear();
    spyReset.clear();

    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(mockModel->mappedNodes_.isEmpty());
    QVERIFY(mockModel->sliceHash_.isEmpty());
}

void TestSliceModel::indexFromNode()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    AdocTreeModel *treeModel = loadTestTree();
    AdocTreeNode *root = treeModel->root();

    // Test: indexFromNode without any valid tree source model should return QModelIndex
    QCOMPARE(mockModel->indexFromNode(root), QModelIndex());
    QCOMPARE(mockModel->indexFromNode(static_cast<AdocTreeNode *>(root->childAt(0))), QModelIndex());

    // Test: indexFromNode for all items from top-level items
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(treeModel->index(1, 0));   // Alpha
    mockModel->loadSlice(treeModel->index(2, 0));   // Delta

    AdocTreeNode *alphaNode = static_cast<AdocTreeNode *>(root->childAt(1));
    AdocTreeNode *deltaNode = static_cast<AdocTreeNode *>(root->childAt(2));

    QCOMPARE(mockModel->indexFromNode(alphaNode), mockModel->index(0, 0));
    QCOMPARE(mockModel->indexFromNode(deltaNode), mockModel->index(1, 0));

    // Test: other non-loaded nodes
    QCOMPARE(mockModel->indexFromNode(static_cast<AdocTreeNode *>(root->childAt(4))), QModelIndex());

    // Test: random, unassociated AdocTreeNode
    AdocTreeNode *node = new AdocTreeNode(AdocTreeNode::GroupType, "Random");
    QCOMPARE(mockModel->indexFromNode(node), QModelIndex());
}

void TestSliceModel::mapToSource()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    AdocTreeModel *treeModel2 = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: root should return invalid
    QCOMPARE(mockModel->mapToSource(QModelIndex()), QModelIndex());

    // Test: index from another model should return invalid modelindex
    QCOMPARE(mockModel->mapToSource(treeModel2->index(0, 0)), QModelIndex());

    // Setup: load some slices
    QModelIndex alphaIndex = treeModel->index(1, 0);
    QModelIndex level2Index = treeModel->index(0, 0, treeModel->index(4, 0));
    mockModel->loadSlice(alphaIndex);
    mockModel->loadSlice(level2Index);

    // Test: alphaindex
    QCOMPARE(mockModel->mapToSource(mockModel->index(0, 0)), alphaIndex);

    // Test: alphaIndex children
    QCOMPARE(mockModel->mapToSource(mockModel->index(0, 0).child(0, 0)), alphaIndex.child(1, 0));
    QCOMPARE(mockModel->mapToSource(mockModel->index(0, 0).child(1, 0)), alphaIndex.child(3, 0));

    // Test: level2Index
    QCOMPARE(mockModel->mapToSource(mockModel->index(1, 0)), level2Index);

    // Test: level2Index children
    QCOMPARE(mockModel->mapToSource(mockModel->index(1, 0).child(0, 0)), level2Index.child(0, 0));
}

void TestSliceModel::flags()
{
    // AdocTreeModel only has one column, so the dataChanged would be for that one column
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Test: flags with empty slice model
    QCOMPARE(mockModel->index(0, 0).flags(), 0);

    // Test: Root index
    mockModel->loadSlice(QModelIndex());
    QModelIndex rootIndex = mockModel->index(0, 0);
    QCOMPARE(rootIndex.flags(), 0);
    for (int i=0, z=mockModel->rowCount(rootIndex); i<z; ++i)
        QCOMPARE(rootIndex.child(i, 0).flags(), Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    // Test: Group zeta
    // --> Top level index
    mockModel->loadSlice(treeModel->index(0));
    QModelIndex zetaIndex = mockModel->index(1, 0);
    QCOMPARE(zetaIndex.flags(), 0);
    // --> first slice item: Alignment
    Qt::ItemFlags flags = zetaIndex.child(0, 0).flags();
    QVERIFY(flags & Qt::ItemIsEnabled);
    QVERIFY(flags & Qt::ItemIsEditable);
    QVERIFY(flags & Qt::ItemIsSelectable);
    QVERIFY(flags & Qt::ItemIsDragEnabled);
    flags = zetaIndex.child(0, 1).flags();  // Msa amino nodes do not have any flags for columns outside the primary column
    QVERIFY(flags == 0);

    // --> second slice item: Binding
    flags = zetaIndex.child(1, 0).flags();
    QVERIFY(flags & Qt::ItemIsEnabled);
    QVERIFY(flags & Qt::ItemIsEditable);
    QVERIFY(flags & Qt::ItemIsSelectable);
    QVERIFY(flags & Qt::ItemIsDragEnabled);
    QCOMPARE(zetaIndex.child(1, 1).flags(), 0);

    QVERIFY(zetaIndex.child(2, 0).isValid() == false);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Drag and drop
/**
  *
  */
void TestSliceModel::removeRows()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
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

    QVERIFY(mockModel->removeRows(0, 1, QModelIndex()) == false);
    QVERIFY(mockModel->removeRows(1, 3, QModelIndex()) == false);
    QVERIFY(mockModel->removeRows(0, 1, treeModel->index(0, 0)) == false);

    // Setup: load some slices
    mockModel->loadSlice(treeModel->index(0, 0));
    mockModel->loadSlice(treeModel->index(1, 0));

    // Test: attempt to remove from root - should fail
    QVERIFY(mockModel->removeRows(0, 2, QModelIndex()) == false);
    QCOMPARE(mockModel->rowCount(), 2);
    QVERIFY(mockModel->removeRows(0, 1, QModelIndex()) == false);
    QCOMPARE(mockModel->rowCount(), 2);

    // Test: attempt to remove from slice items
    QModelIndex sliceZetaParent = mockModel->index(0, 0);
    QModelIndex sliceAlphaParent = mockModel->index(1, 0);
    QVERIFY(mockModel->removeRows(0, 1, sliceZetaParent.child(0, 0)) == false);
    QCOMPARE(mockModel->rowCount(sliceZetaParent), 2);
    QVERIFY(mockModel->removeRows(0, 2, sliceAlphaParent.child(1, 0)) == false);
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 2);

    // Test: attempt to remove from parent not belonging to this model
    QVERIFY(mockModel->removeRows(0, 1, treeModel->index(0, 0)) == false);

    // Test: actual valid row removal - Alignment
    QVERIFY(mockModel->removeRows(0, 1, sliceZetaParent));
    QCOMPARE(mockModel->rowCount(sliceZetaParent), 1);
    QCOMPARE(treeModel->rowCount(treeModel->index(0, 0)), 2);
    QCOMPARE(sliceZetaParent.child(0, 0).data().toString(), QString("Binding"));
    QCOMPARE(treeModel->index(0, 0).child(0, 0).data().toString(), QString("Binding"));
    QCOMPARE(treeModel->index(0, 0).child(1, 0).data().toString(), QString("Site"));

    // Test: multi-row removal
    QVERIFY(mockModel->removeRows(0, 2, sliceAlphaParent));
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 0);
    QCOMPARE(treeModel->rowCount(treeModel->index(1, 0)), 2);
    QCOMPARE(treeModel->index(1, 0).child(0, 0).data().toString(), QString("Toxic"));
    QCOMPARE(treeModel->index(1, 0).child(1, 0).data().toString(), QString("Dummy"));
}

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
void TestSliceModel::dragdrop()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
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
    mockModel->loadSlice(QModelIndex());
    QModelIndex sliceRootParent = mockModel->index(0, 0);
    QVERIFY(sliceRootParent.isValid());

    // Test: top-level item may not produce mimeData
    QVERIFY(mockModel->mimeData(QModelIndexList() << sliceRootParent) == 0);

    // Test: slice item (Alpha) should result in non-zero value of mimeData
    QMimeData *mimeData = mockModel->mimeData(QModelIndexList() << sliceRootParent.child(0, 0));
    QVERIFY(mimeData);
    // Check that the indices were converted
    ModelIndexMimeData *myMimeData = static_cast<ModelIndexMimeData *>(mimeData);
    QCOMPARE(myMimeData->indexes_.count(), 1);
    QCOMPARE(myMimeData->indexes_.at(0), treeModel->index(1, 0));
    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Context: Alpha slice loaded
    mockModel->loadSlice(treeModel->index(1, 0));
    QModelIndex sliceAlphaParent = mockModel->index(1, 0);

    // Test: two valid nodes on primaryColumn, but from different parents
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceAlphaParent.child(0, 0)      // Acid
                                   << sliceRootParent.child(1, 0));    // Beta
    QVERIFY(mimeData);
    // Check that the indices were mapped to the tree model equivalents
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);
    QCOMPARE(myMimeData->indexes_.count(), 2);
    QCOMPARE(myMimeData->indexes_.at(0), treeModel->index(1, 0).child(1, 0));
    QCOMPARE(myMimeData->indexes_.at(1), treeModel->index(3, 0));
    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Test: two valid nodes, with one index on a nonPrimaryColumn
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceAlphaParent.child(0, 0)      // Acid
                                   << sliceAlphaParent.child(1, 0)      // Base
                                   << sliceAlphaParent.child(1, nonPrimaryColumn));       // Base with nonPrimaryColumn
    QVERIFY(mimeData == 0);

    // Test: two valid nodes both on primaryColumn
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceAlphaParent.child(0, 0)      // Acid
                                   << sliceAlphaParent.child(1, 0));    // Base
    QVERIFY(mimeData);
    // Check that the indices were mapped to the tree model equivalents
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);
    QCOMPARE(myMimeData->indexes_.count(), 2);
    QCOMPARE(myMimeData->indexes_.at(0), treeModel->index(1, 0).child(1, 0));
    QCOMPARE(myMimeData->indexes_.at(1), treeModel->index(1, 0).child(3, 0));


    // --------------------------
    // Suite: dropMimeData method
    // Model data context:
    // [0] = Root
    //  |___ [0] Alpha
    //  |___ [1] Beta
    // [1] = Alpha
    //  |___ [0] Acid  <-- stored in myMimeData
    //  |___ [1] Base  <-- stored in myMimeData

    // Test: parent = invalid index, same parent, slice items
    QModelIndexList indices;
    indices << QModelIndex()
            << sliceRootParent
            << sliceAlphaParent;
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

    // Test: Empty mime data should return false (vary other parameters to explore test space)
    QCOMPARE(mockModel->dropMimeData(0,          Qt::CopyAction,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(0,          Qt::MoveAction,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(0,          Qt::LinkAction,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(0,          Qt::ActionMask,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(0,          Qt::IgnoreAction,      -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(0,          Qt::TargetMoveAction,  -1, -1, sliceRootParent.child(2, 0)), false);

    // Test: All actions except MoveAction should return false
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::CopyAction,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::LinkAction,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::ActionMask,        -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::IgnoreAction,      -1, -1, sliceRootParent.child(2, 0)), false);
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::TargetMoveAction,  -1, -1, sliceRootParent.child(2, 0)), false);

    // Test: dropping onto the same parent should fail. Acid and Base are children of the group alpha (on the tree node).
    //       Since the alpha group is a slice item underneath sliceRootParent, this provides a method of attempting
    //       to place these items under its current parent.
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceRootParent.child(0, 0)), false);

    // Test: dropping onto itself
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceAlphaParent.child(0, 0)), false);
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceAlphaParent.child(1, 0)), false);


    // ------------------------------
    // Suite: valid dropMimeData calls
    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    mimeData = mockModel->mimeData(QModelIndexList() << sliceAlphaParent.child(1, 0));
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    // Current tree looks like:
    // Root [0]
    // |___ Zeta (Group)
    //      |___ Alignment
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [1]
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
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceRootParent.child(1, 0)), true);

    // At this point, Base should still exist under Alpha, but have a fkId_ of zero
    QCOMPARE(treeModel->rowCount(treeModel->index(1, 0)), 4);
    QCOMPARE(treeModel->index(1, 0).child(3, 0).data().toString(), QString("Base"));
    QCOMPARE(treeModel->nodeFromIndex(treeModel->index(1, 0).child(3, 0))->fkId_, 0);

    // Remove the rows that were successfully moved (see log.txt, 15 September 2010 for details)
    treeModel->removeRow(3, treeModel->index(1, 0));

    // Check that the rows were moved and updated in the slice model
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 1);
    QCOMPARE(sliceAlphaParent.child(0, 0).data().toString(), QString("Acid"));
    QCOMPARE(mockModel->rowCount(sliceRootParent), 2);

    // Check that the rows were moved in the adoc tree model
    QCOMPARE(treeModel->rowCount(treeModel->index(3, 0)), 1);
    QCOMPARE(treeModel->index(3, 0).child(0, 0).data().toString(), QString("Base"));

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;

    // Test: dropping mime data with multiple parents onto any originating parent should fail
    // Current tree looks like:
    // Root [0]
    // |___ Zeta (Group) [2] ** after loadSlice
    //      |___ Alignment              <-- Moving this node
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [1]
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

    mockModel->loadSlice(treeModel->index(0, 0));
    QCOMPARE(mockModel->rowCount(), 3);
    QModelIndex sliceZetaParent = mockModel->index(2, 0);

    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceZetaParent.child(0, 0)       // "Alignment"
                                   << sliceAlphaParent.child(0, 0));    // "Acid"
    QVERIFY(mimeData);
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    // Artificially set the fkId_ to ensure that it is zeroed out during the move
    treeModel->nodeFromIndex(treeModel->index(1, 0).child(1, 0))->fkId_ = 100;

    // Test: check that we cannot drop on same parent; sliceRootParent.child(0, 0) == Alpha, which is the parent
    //       of Acid
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceRootParent.child(0, 0)), false);

    // Verify that the rows were not modifed
    QCOMPARE(mockModel->rowCount(sliceZetaParent), 2);
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 1);

    // Test: moving to another parent should work
    QCOMPARE(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceRootParent.child(1, 0)), true);

    // At this point, Alignment and Acid stub nodes should still exist under Alpha
    QCOMPARE(treeModel->rowCount(treeModel->index(0, 0)), 3);
    QCOMPARE(treeModel->index(0, 0).child(0, 0).data().toString(), QString("Alignment"));
    QCOMPARE(treeModel->rowCount(treeModel->index(1, 0)), 3);
    QCOMPARE(treeModel->index(1, 0).child(1, 0).data().toString(), QString("Acid"));
    QCOMPARE(treeModel->nodeFromIndex(treeModel->index(1, 0).child(1, 0))->fkId_, 0);

    // Remove the rows that were successfully moved (see log.txt, 15 September 2010 for details)
    treeModel->removeRow(0, treeModel->index(0, 0));
    treeModel->removeRow(1, treeModel->index(1, 0));

    QCOMPARE(mockModel->rowCount(sliceZetaParent), 1);
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 0);
    QCOMPARE(mockModel->rowCount(sliceRootParent), 2);

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
    // Root [0]
    // |___ Zeta (Group) [2]
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [1]
    //      |___ Toxic
    //      |___ Dummy
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group) [3]        <-- In mime data
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
    mockModel->loadSlice(treeModel->index(3, 0));
    QModelIndex sliceBetaParent = mockModel->index(3, 0);

    // Sanity check that the newly created node is present
    QCOMPARE(sliceBetaParent.child(3, 0).data().toString(), QString("Agroup"));
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceRootParent.child(1, 0));    // "Beta"
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    // Attempt to drop Beta onto Agroup - this should fail!
    QVERIFY(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceBetaParent.child(3, 0)) == false);
    QCOMPARE(mockModel->rowCount(sliceRootParent), 2);

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;



    // Test: drop items that have a parent-child relationship
    // Current tree looks like:
    // Root [0]
    // |___ Zeta (Group) [2]
    //      |___ Binding (Group)
    //      |___ Site
    // |___ Alpha (Group) [1]   <-- attempt to move here
    //      |___ Toxic
    //      |___ Dummy
    // |___ Delta (Group)
    //      |___ Force
    // |___ Beta (Group) [3]    <-- add to mime data
    //      |___ Base           <-- add to mime data
    //      |___ Alignment
    //      |___ Acid
    //      |___ Agroup
    // |___ Gamma (Group)
    //      |____ Level2 (Group)
    //            |____ Apple
    // |___ Kappa (Group)
    mimeData = mockModel->mimeData(QModelIndexList()
                                   << sliceRootParent.child(1, 0)    // "Beta"
                                   << sliceBetaParent.child(0, 0));  // "Base"
    myMimeData = static_cast<ModelIndexMimeData *>(mimeData);

    QVERIFY(mockModel->dropMimeData(myMimeData, Qt::MoveAction, -1, -1, sliceRootParent.child(0, 0)) == false);
    QCOMPARE(mockModel->rowCount(sliceAlphaParent), 0);
    QCOMPARE(mockModel->rowCount(sliceBetaParent), 4);

    delete mimeData;
    mimeData = 0;
    myMimeData = 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Signal related tests - changes to the source tree model should be appropriately handled and
// dealt with in the SliceModel

/**
  * For now, only test with group type labels, because AdocTreeModel is currently setup to submit
  * database requests when changing the data.
  *
  * Cannot test whether changing non-zero columns
  */
void TestSliceModel::dataChanged()
{
    // AdocTreeModel only has one column, so the dataChanged would be for that one column
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup: signal spies
    QSignalSpy spyDataChanged(mockModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVERIFY(spyDataChanged.isValid());

    // Test: dataChanged should not be propagated period for slice model with no slices loaded
    QVERIFY(treeModel->setData(treeModel->index(0, 0), "ZetaZeta"));
    QVERIFY(spyDataChanged.isEmpty());

    // Test: dataChanged with loadedSlice (root); changes to mapped slice items should be evoked :)
    mockModel->loadSlice(QModelIndex());

    // Subtest: change to non-mapped nodes should not transmit dataChanged signal
    QVERIFY(treeModel->setData(treeModel->index(0, 0), "Zeta"));
    QVERIFY(treeModel->setData(treeModel->index(2, 0), "DeltaDelta"));
    QVERIFY(treeModel->setData(treeModel->index(4, 0), "GammaGamma"));
    QVERIFY(treeModel->setData(treeModel->index(5, 0), "KappaKappa"));
    QVERIFY(spyDataChanged.isEmpty());

    // Subtest: change to mapped nodes should transmit the dataChanged signal appropriately
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
    // During testing (#define TESTING), TestSliceModel is a friend class of AdocTreeModel, which
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

    // Test: change item that is both mapped as a slice item and top level item
    //       should have dataChangedCount of 2; don't bother teasing apart which
    //       occurs first as this doesn't matter.
    //
    // Context: Two slices have been loaded at this point:
    // 1) Root (mock model index 0, 0) which has two rows: Alpha and Beta
    // 2) Alpha (mock model index 1, 0) which also has two rows: Acid and Base
    //
    // In the first case, Alpha is a slice item, and in the second case, alpha is a top-level
    // item. Thus, the dataChanged event for this node in the source tree model should
    // multiply into two dataChanged events in the slice model.
    mockModel->loadSlice(treeModel->index(1, 0));
    emit treeModel->dataChanged(treeModel->index(1, 0), treeModel->index(1, 0));
    QCOMPARE(spyDataChanged.count(), 2);
    spyDataChanged.clear();

    // Test: only top-level item being changed
    mockModel->releaseSlice(QModelIndex());
    emit treeModel->dataChanged(treeModel->index(1, 0), treeModel->index(1, 0));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
    bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QCOMPARE(topLeft, mockModel->index(0, 0));
    QCOMPARE(bottomRight, mockModel->index(0, 0));
    spyDataChanged.clear();
}

void TestSliceModel::modelReset()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

    // Setup
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);
    mockModel->loadSlice(treeModel->index(0, 0));
    mockModel->loadSlice(treeModel->index(1, 0));
    mockModel->loadSlice(QModelIndex());
    QCOMPARE(mockModel->rowCount(), 3);

    // Signal spies
    QSignalSpy spyModelAboutToBeReset(mockModel, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(mockModel, SIGNAL(modelReset()));
    QVERIFY(spyModelAboutToBeReset.isValid());
    QVERIFY(spyModelReset.isValid());

    // Test: change the treeModel parent root, which will trigger a reset that can be tested here
    mockModel->virtualSignalCounts_.clear();
    QVERIFY(treeModel->setRoot(new AdocTreeNode(AdocTreeNode::RootType, "Root")));

    // Check the signals
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("slicesReset"));
    mockModel->virtualSignalCounts_.clear();

    // Verify that the mockModel does not contain any rows
    QCOMPARE(mockModel->rowCount(), 0);
    QVERIFY(mockModel->mappedNodes_.isEmpty());
    QVERIFY(mockModel->sliceHash_.isEmpty());
}

void TestSliceModel::insertRow()
{
    MockSliceModel *mockModel = new MockSliceModel(this);

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
    mockModel->loadSlice(QModelIndex());    // This will actually trigger insert row signals
    // Clear them so we don't get a false positive on the tree insertion
    mockModel->virtualSignalCounts_.clear();
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->rowCount(), 1);
    treeModel->insertRow(8, new AdocTreeNode(AdocTreeNode::GroupType, "Iota"));
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: insert into tree model at position 0, non-mapped node - should update the parent row indices
    // Verify intial state
    AdocTreeNode *node = mockModel->mappedNodes_.at(0);
    Slice *slice = mockModel->sliceHash_[node];
    QCOMPARE(slice->sourceRowIndices_.at(0), 1);
    QCOMPARE(slice->sourceRowIndices_.at(1), 3);
    treeModel->insertRow(0, new AdocTreeNode(AdocTreeNode::GroupType, "Pi"));
    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->rowCount(mockModel->index(0, 0)), 2);
    QVERIFY(spyRowsAboutToBeInserted.isEmpty());
    QVERIFY(spyRowsInserted.isEmpty());
    // Verify state changed appropriately
    QCOMPARE(slice->sourceRowIndices_.at(0), 2);
    QCOMPARE(slice->sourceRowIndices_.at(1), 4);
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Test: insert mapped node into tree model at position 0
    AdocTreeNode *alignerNode = new AdocTreeNode(AdocTreeNode::GroupType, "Aligner");
    treeModel->insertRow(0, alignerNode);
    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->rowCount(mockModel->index(0, 0)), 3);
    // Verify that signals were emitted showing that row was appended
    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel->index(0, 0));
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyArguments = spyRowsInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel->index(0, 0));
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).start_, 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).end_, 2);
    mockModel->virtualSignalCounts_.clear();

    // Verify that alignerNode was appended to mappedNodes at third position
    QCOMPARE(slice->rows_.count(), 3);
    QVERIFY(slice->sourceNodes_.at(2) == alignerNode);
    QCOMPARE(slice->sourceRowIndices_.at(0), 3);
    QCOMPARE(slice->sourceRowIndices_.at(1), 5);
    QCOMPARE(slice->sourceRowIndices_.at(2), 0);

    // Test: insert mapped node at end of tree model
    AdocTreeNode *blastNode = new AdocTreeNode(AdocTreeNode::GroupType, "BLAST");
    treeModel->insertRow(treeModel->rowCount(), blastNode);
    QCOMPARE(mockModel->rowCount(), 1);
    QCOMPARE(mockModel->rowCount(mockModel->index(0, 0)), 4);
    // Verify that signals were emitted showing that row was appended
    QCOMPARE(spyRowsAboutToBeInserted.count(), 1);
    QCOMPARE(spyRowsInserted.count(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel->index(0, 0));
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyArguments = spyRowsInserted.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel->index(0, 0));
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();
    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceRowsInserted"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, slice);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).start_, 3);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).end_, 3);
    mockModel->virtualSignalCounts_.clear();

    // Verify that alignerNode was appended to mappedNodes at third position
    QCOMPARE(slice->rows_.count(), 4);
    QVERIFY(slice->sourceNodes_.at(3) == blastNode);
    QCOMPARE(slice->sourceRowIndices_.at(0), 3);
    QCOMPARE(slice->sourceRowIndices_.at(1), 5);
    QCOMPARE(slice->sourceRowIndices_.at(2), 0);
    QCOMPARE(slice->sourceRowIndices_.at(3), treeModel->rowCount() - 1);
}

void TestSliceModel::removeRow()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
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
    QVERIFY(mockModel->mappedNodes_.isEmpty());
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
    QVERIFY(mockModel->mappedNodes_.isEmpty());
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
    mockModel->loadSlice(treeModel->index(treeModel->rowCount() - 1, 0, QModelIndex()));
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(2, QModelIndex());
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->mappedNodes_.count() == 1);
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
    mockModel->loadSlice(treeModel->index(0, 0));   // Zeta
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(2, treeModel->index(0, 0));    // "Site"
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // At this point, mockModel has two mapped nodes (Kappa and Zeta). Confirm they are in a valid state
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(mockModel->rowCount(mockModel->index(0, 0)), 0);
    QCOMPARE(mockModel->rowCount(mockModel->index(1, 0)), 2);  // Contains Alignment and Binding
    QCOMPARE(mockModel->mappedNodes_.count(), 2);
    Slice *kappaSlice = mockModel->sliceHash_[mockModel->mappedNodes_.at(0)];
    QCOMPARE(kappaSlice->rows_.count(), 0);
    Slice *zetaSlice = mockModel->sliceHash_[mockModel->mappedNodes_.at(1)];
    QCOMPARE(zetaSlice->rows_.count(), 2);

    // -------------------------------------
    // Test: attached tree, loaded slice, mapped parent with mapped children, unmapped leaf row before mapped nodes
    //       "Toxic"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *3
    //      |___ Toxic      <--- TBR
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa *1
    mockModel->loadSlice(treeModel->index(1, 0));       // Mapping "Alpha" goes into 3rd row of mockModel
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(0, treeModel->index(1, 0));    // Removing "Toxic"
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());

    // Verify that the sourceRowIndices in the slice model were updated accordingly
    Slice *alphaSlice = mockModel->sliceHash_[mockModel->mappedNodes_.at(2)];
    QCOMPARE(alphaSlice->rows_.count(), 2);
    QCOMPARE(alphaSlice->sourceRowIndices_.at(0), 0);
    QCOMPARE(alphaSlice->sourceRowIndices_.at(1), 2);

    // -------------------------------------
    // Test: attached tree, loaded slice, mapped parent with mapped children, unmapped leaf row in middle mapped nodes
    //       "Dummy" under "Alpha"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *3
    //      |___ Acid
    //      |___ Dummy      <--- TBR
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa *1
    treeModel->removeRow(1, treeModel->index(1, 0));
    QVERIFY(spyRowsAboutToBeRemoved.isEmpty());
    QVERIFY(spyRowsRemoved.isEmpty());
    QVERIFY(mockModel->virtualSignalCounts_.isEmpty());
    QCOMPARE(alphaSlice->rows_.count(), 2);
    QCOMPARE(alphaSlice->sourceRowIndices_.at(0), 0);
    QCOMPARE(alphaSlice->sourceRowIndices_.at(1), 1);

    // -------------------------------------
    // Test: attached tree, mapped slice node occurs prior to other mapped slice nodes
    //       "Acid"
    //
    // Root
    // |___ Zeta *2
    //      |___ Alignment
    //      |___ Binding
    // |___ Alpha *3
    //      |___ Acid       <--- TBR
    //      |___ Base
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa *1
    treeModel->removeRow(0, treeModel->index(1, 0));

    // Check slice model internal state
    QCOMPARE(alphaSlice->rows_.count(), 1);
    QCOMPARE(alphaSlice->sourceRowIndices_.at(0), 0);
    QCOMPARE(alphaSlice->sourceNodes_.at(0)->label_, QString("Base"));

    // Check that the signals were emitted properly
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel->index(2, 0));
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
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, alphaSlice);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).start_, 0);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).end_, 0);

    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("sliceRowsRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).slice_, alphaSlice);
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
    //      |____ Level2 *4
    //            |____ Apple
    // |___ Kappa *1
    QModelIndex gammaIndex = treeModel->index(3, 0);
    QCOMPARE(gammaIndex.data().toString(), QString("Gamma"));
    mockModel->loadSlice(treeModel->index(0, 0, gammaIndex));       // 4th element loaded

    QCOMPARE(mockModel->mappedNodes_.at(3)->label_, QString("Level2"));
    QCOMPARE(mockModel->rowCount(), 4);

    Slice *level2Slice = mockModel->sliceHash_[mockModel->mappedNodes_.at(3)];

    // Removing "Gamma"
    mockModel->virtualSignalCounts_.clear();
    treeModel->removeRow(gammaIndex.row(), QModelIndex());

    // Check internal state of mock Model - it should have removed the slice
    QCOMPARE(mockModel->rowCount(), 3);
    QCOMPARE(mockModel->mappedNodes_.count(), 3);

    // Check signals
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 1);
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spyRowsRemoved.count(), 1);
    spyArguments = spyRowsRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), spyIndex);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 1);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, level2Slice);
    mockModel->virtualSignalCounts_.clear();

    // -------------------------------------
    // Test: remove of mapped top level node, an item of which is also mapped
    //
    // Root
    // |___ Zeta *2             <--- TBR
    //      |___ Alignment
    //      |___ Binding **4
    // |___ Alpha *3
    //      |___ Base
    // |___ Beta
    // |___ Kappa *1
    mockModel->loadSlice(treeModel->index(1, 0, treeModel->index(0, 0)));   // Loading "Binding" which is a child of zeta in the tree model (also mapped at this point)
    mockModel->virtualSignalCounts_.clear();
    // Remember the slice pointers for validating the virtual calls
    Slice *bindingSlice = mockModel->sliceHash_[mockModel->mappedNodes_.at(3)];
    treeModel->removeRow(0, QModelIndex());

    // Verify internal state of slicemodel
    QCOMPARE(mockModel->rowCount(), 2);
    QCOMPARE(mockModel->mappedNodes_.count(), 2);
    QCOMPARE(mockModel->mappedNodes_.at(0)->label_, QString("Kappa"));
    QCOMPARE(mockModel->mappedNodes_.at(1)->label_, QString("Alpha"));

    // Verify the signals
    QCOMPARE(spyRowsAboutToBeRemoved.count(), 2);

    // The first is for removing "Binding" (because removal is done in reverse order that they are on the mappedNodes_ list)
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());        // Parent of "Binding"
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    // The second is for removing "Zeta"
    spyArguments = spyRowsAboutToBeRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Kappa is before zeta making zeta row #1
    QCOMPARE(spyArguments.at(2).toInt(), 1);

    QCOMPARE(spyRowsRemoved.count(), 2);
    spyArguments = spyRowsRemoved.takeFirst();
    // Again, the first is for removing "Binding"
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    // The second is for removing "Zeta"
    spyArguments = spyRowsRemoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Kappa is before zeta making zeta row #1
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spyRowsAboutToBeRemoved.clear();
    spyRowsRemoved.clear();

    QCOMPARE(mockModel->virtualSignalCounts_.count(), 2);
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(0).slice_, bindingSlice);
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel->virtualSignalCounts_.at(1).slice_, zetaSlice);
    mockModel->virtualSignalCounts_.clear();

    // -------------------------------------
    // Test: remove top-level item that is both mapped and a slice item
    //
    // Root *1
    // |___ Zeta
    //      |___ Alignment
    //      |___ Binding
    //      |___ Site
    // |___ Alpha *2        <--- TBR
    //      |___ Toxic
    //      |___ Acid
    //      |___ Dummy
    //      |___ Base
    // |___ Delta
    //      |___ Force
    // |___ Beta
    // |___ Gamma
    //      |____ Level2
    //            |____ Apple
    // |___ Kappa
    MockSliceModel *mockModel2 = new MockSliceModel(this);
    AdocTreeModel *treeModel2 = loadTestTree();
    mockModel2->setTreeModel(treeModel2);

    mockModel2->loadSlice(QModelIndex());           // Load the root
    mockModel2->loadSlice(treeModel2->index(1, 0)); // Load "Alpha"

    // Remember the slice pointers for validating the virtual calls
    Slice *mock2RootSlice = mockModel2->sliceHash_[mockModel2->mappedNodes_.at(0)];
    Slice *mock2AlphaSlice = mockModel2->sliceHash_[mockModel2->mappedNodes_.at(1)];

    QSignalSpy spyRowsAboutToBeRemoved2(mockModel2, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyRowsRemoved2(mockModel2, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QVERIFY(spyRowsAboutToBeRemoved2.isValid());
    QVERIFY(spyRowsRemoved2.isValid());

    mockModel2->virtualSignalCounts_.clear();
    treeModel2->removeRow(1, QModelIndex());    // Removing "Alpha"

    // Verify internal state
    QCOMPARE(mockModel2->rowCount(), 1);
    QCOMPARE(mockModel2->rowCount(mockModel2->index(0, 0)), 1);
    QCOMPARE(mockModel2->mappedNodes_.at(0), treeModel2->root());

    // Signals
    // There should be two paired signals for removing this row
    // The first corresponds to the slice item associated with the mapped Root node
    // The second is the top-level node alpha itself
    //
    // Check the slice item first since these are processed first
    QCOMPARE(spyRowsAboutToBeRemoved2.count(), 2);

    // Root slice row "Alpha"
    spyArguments = spyRowsAboutToBeRemoved2.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel2->index(0, 0));    // Parent of alpha
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);

    // Top-level item "Alpha"
    spyArguments = spyRowsAboutToBeRemoved2.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 1);

    // Same thing for the rowsRemoved signal
    QCOMPARE(spyRowsRemoved2.count(), 2);

    // Root slice row "Alpha"
    spyArguments = spyRowsRemoved2.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, mockModel2->index(0, 0));    // Parent of alpha
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);

    // Top-level item "Alpha"
    spyArguments = spyRowsRemoved2.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex, QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 1);

    QCOMPARE(mockModel2->virtualSignalCounts_.count(), 3);
    QCOMPARE(mockModel2->virtualSignalCounts_.at(0).type_, QString("sliceRowsAboutToBeRemoved"));
    QCOMPARE(mockModel2->virtualSignalCounts_.at(0).slice_, mock2RootSlice);
    QCOMPARE(mockModel2->virtualSignalCounts_.at(0).start_, 0);
    QCOMPARE(mockModel2->virtualSignalCounts_.at(0).end_, 0);

    QCOMPARE(mockModel2->virtualSignalCounts_.at(1).type_, QString("sliceRowsRemoved"));
    QCOMPARE(mockModel2->virtualSignalCounts_.at(1).slice_, mock2RootSlice);
    QCOMPARE(mockModel2->virtualSignalCounts_.at(1).start_, 0);
    QCOMPARE(mockModel2->virtualSignalCounts_.at(1).end_, 0);

    QCOMPARE(mockModel2->virtualSignalCounts_.at(2).type_, QString("sliceAboutToBeRemoved"));
    QCOMPARE(mockModel2->virtualSignalCounts_.at(2).slice_, mock2AlphaSlice);
}

// Note: this is a protected function and not normally accessible externally; however, for testing
// purposes, this class is a friend class and thus able to check that this method works as expected.
void TestSliceModel::indexFromSlice()
{
    MockSliceModel *mockModel = new MockSliceModel(this);
    AdocTreeModel *treeModel = loadTestTree();
    mockModel->setTreeModel(treeModel);

    // Setup: load some slices
    mockModel->loadSlice(treeModel->index(0, 0));
    mockModel->loadSlice(QModelIndex());
    mockModel->loadSlice(treeModel->index(1, 0));

    // Test: Null slice should not be present
    QVERIFY(mockModel->indexFromSlice(0).isValid() == false);

    // Test: slice created outside of model
    Slice *slice = new Slice();
    QVERIFY(mockModel->indexFromSlice(slice).isValid() == false);

    // Test: Slices should map to their corresponding indices; to access the slices for testing, must
    Slice *first = mockModel->sliceHash_[mockModel->mappedNodes_.at(0)];
    Slice *second = mockModel->sliceHash_[mockModel->mappedNodes_.at(1)];
    Slice *third = mockModel->sliceHash_[mockModel->mappedNodes_.at(2)];

    QCOMPARE(mockModel->indexFromSlice(first), mockModel->index(0, 0));
    QCOMPARE(mockModel->indexFromSlice(second), mockModel->index(1, 0));
    QCOMPARE(mockModel->indexFromSlice(third), mockModel->index(2, 0));
}


QTEST_MAIN(TestSliceModel)
#include "TestSliceModel.moc"
