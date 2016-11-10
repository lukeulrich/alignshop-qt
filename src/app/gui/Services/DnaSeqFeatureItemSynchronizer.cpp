/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaSeqFeatureItemSynchronizer.h"
#include "../../primer/DnaSeqPrimerModel.h"
#include "../factories/PrimerItemFactory.h"
#include "../painting/features/AbstractPrimerItem.h"
#include "../painting/features/FeatureTypes.h"
#include "../painting/gitems/DnaSeqItem.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
DnaSeqFeatureItemSynchronizer::DnaSeqFeatureItemSynchronizer(QObject *parent)
    : QObject(parent),
      dnaSeqItem_(nullptr),
      dnaSeqPrimerModel_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void DnaSeqFeatureItemSynchronizer::setDnaSeqItem(DnaSeqItem *dnaSeqItem)
{
    if (dnaSeqItem == dnaSeqItem_)
        return;

    if (dnaSeqItem_)
        disconnect(dnaSeqItem_, SIGNAL(aboutToBeDestroyed()), this, SLOT(onDnaSeqItemAboutToBeDestroyed()));

    dnaSeqItem_ = dnaSeqItem;
    if (dnaSeqItem_)
    {
        connect(dnaSeqItem_, SIGNAL(aboutToBeDestroyed()), SLOT(onDnaSeqItemAboutToBeDestroyed()));
        initializePrimerItemsFromModel();
    }
}

void DnaSeqFeatureItemSynchronizer::setDnaSeqPrimerModel(DnaSeqPrimerModel *dnaSeqPrimerModel)
{
    if (dnaSeqPrimerModel == dnaSeqPrimerModel_)
        return;

    if (dnaSeqPrimerModel_ != nullptr)
    {
        disconnect(dnaSeqPrimerModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onPrimerDataChanged(QModelIndex,QModelIndex)));
        disconnect(dnaSeqPrimerModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(onPrimerRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(dnaSeqPrimerModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onPrimerRowsInserted(QModelIndex,int,int)));
        disconnect(dnaSeqPrimerModel_, SIGNAL(modelReset()), this, SLOT(initializePrimerItemsFromModel()));
    }

    dnaSeqPrimerModel_ = dnaSeqPrimerModel;

    if (dnaSeqPrimerModel_ != nullptr)
    {
        connect(dnaSeqPrimerModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onPrimerDataChanged(QModelIndex,QModelIndex)));
        connect(dnaSeqPrimerModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onPrimerRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(dnaSeqPrimerModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onPrimerRowsInserted(QModelIndex,int,int)));
        connect(dnaSeqPrimerModel_, SIGNAL(modelReset()), SLOT(initializePrimerItemsFromModel()));
    }

    // Regardless if a valid DnaSeqPrimerModel is defined, reset any primer features associated with this dna seq
    initializePrimerItemsFromModel();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void DnaSeqFeatureItemSynchronizer::onDnaSeqItemAboutToBeDestroyed()
{
    setDnaSeqItem(nullptr);
}

void DnaSeqFeatureItemSynchronizer::onPrimerDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (dnaSeqItem_ == nullptr)
        return;

    ASSERT(dnaSeqPrimerModel_ != nullptr);
    ASSERT(topLeft.row() <= bottomRight.row());
    ASSERT(topLeft.column() <= bottomRight.column());

    if (DnaSeqPrimerModel::eNameColumn < topLeft.column() || DnaSeqPrimerModel::eNameColumn > bottomRight.column())
        return;

    // Update the names of all the primer items
    using namespace constants::Features;
    for (int i=topLeft.row(); i<= bottomRight.row(); ++i)
    {
        int primerId = dnaSeqPrimerModel_->primerId(i);
        QModelIndex nameIndex = dnaSeqPrimerModel_->index(i, DnaSeqPrimerModel::eNameColumn);
        QString name = nameIndex.data().toString();
        QVector<AbstractFeatureItem *> featureItems = dnaSeqItem_->findFeatures(kPrimerFeatureType, primerId);
        foreach (AbstractFeatureItem *featureItem, featureItems)
        {
            AbstractPrimerItem *primerItem = static_cast<AbstractPrimerItem *>(featureItem);
            primerItem->setName(name);
        }
    }
}

void DnaSeqFeatureItemSynchronizer::onPrimerRowsAboutToBeRemoved(const QModelIndex & /* parent */, int startRow, int endRow)
{
    ASSERT(dnaSeqPrimerModel_ != nullptr);
    if (dnaSeqItem_ == nullptr || dnaSeqItem_->dnaSeq() != dnaSeqPrimerModel_->dnaSeq())
        return;

    using namespace constants::Features;
    for (int i=startRow; i<= endRow; ++i)
        dnaSeqItem_->removeFeature(kPrimerFeatureType, dnaSeqPrimerModel_->primerId(i));
}

void DnaSeqFeatureItemSynchronizer::onPrimerRowsInserted(const QModelIndex & /* parent */, int startRow, int endRow)
{
    ASSERT(dnaSeqPrimerModel_ != nullptr);
    if (dnaSeqItem_ == nullptr || dnaSeqItem_->dnaSeq() != dnaSeqPrimerModel_->dnaSeq())
        return;

    generatePrimerItems(startRow, endRow);
}

void DnaSeqFeatureItemSynchronizer::initializePrimerItemsFromModel()
{
    // Clear any existing existing primers
    if (dnaSeqItem_ == nullptr)
        return;
    using namespace constants::Features;
    dnaSeqItem_->clearFeatures(kPrimerFeatureType);

    // Re-initialize all the primer instances
    if (dnaSeqPrimerModel_ == nullptr || dnaSeqItem_->dnaSeq() != dnaSeqPrimerModel_->dnaSeq())
        return;
    int numberOfPrimers = dnaSeqPrimerModel_->rowCount();
    if (numberOfPrimers == 0)
        return;
    generatePrimerItems(0, numberOfPrimers - 1);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void DnaSeqFeatureItemSynchronizer::generatePrimerItems(int startRow, int endRow)
{
    ASSERT(dnaSeqPrimerModel_ != nullptr);
    ASSERT(startRow >= 0);
    ASSERT(startRow <= endRow);

    DnaSeqSPtr dnaSeq = dnaSeqItem_->dnaSeq();
    PrimerItemFactory factory;
    factory.setDnaSeq(dnaSeq);
    for (int i=startRow; i<= endRow; ++i)
    {
        QVector<AbstractPrimerItem *> primerItems = factory.makePrimerItemsFromPrimer(dnaSeqPrimerModel_->primerFromRow(i));
        foreach (AbstractPrimerItem *primerItem, primerItems)
            dnaSeqItem_->addFeature(primerItem);
    }
}
