/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QPen>

#include "AbstractSeqItem.h"
#include "../features/AbstractFeaureItem.h"
#include "../../models/ColumnAdapters/IColumnAdapter.h"

static const double kDefaultHeight_ = 30.;
static const double kDefaultBaseLineHeight = 6.;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param abstractSeq [const AbstractSeqSPtr &]
  * @param parentItem [QGraphicsItem *]
  */
AbstractSeqItem::AbstractSeqItem(const AbstractSeqSPtr &abstractSeq, QGraphicsItem *parentItem)
    : AbstractRangeItem(abstractSeq->range(), abstractSeq->abstractAnonSeq()->seq_.length(), parentItem),
      abstractSeq_(abstractSeq),
      columnAdapter_(nullptr),
      startColumn_(0),
      stopColumn_(0),
      baseLineHeight_(kDefaultBaseLineHeight),
      baseLine_(nullptr)
{
    constructor_();
}

/**
  * @param abstractSeq [const AbstractSeqSPtr &]
  * @param columnAdapter [IColumnAdapter *]
  * @param parentItem [QGraphicsItem *]
  */
AbstractSeqItem::AbstractSeqItem(const AbstractSeqSPtr &abstractSeq, IColumnAdapter *columnAdapter, int startColumn, int stopColumn, QGraphicsItem *parentItem)
    : AbstractRangeItem(abstractSeq->range(), abstractSeq->abstractAnonSeq()->seq_.length(), parentItem),
      abstractSeq_(abstractSeq),
      columnAdapter_(columnAdapter),
      startColumn_(startColumn),
      stopColumn_(stopColumn),
      baseLineHeight_(kDefaultBaseLineHeight),
      baseLine_(nullptr)
{
    constructor_();

    if (columnAdapter_ != nullptr)
        connect(columnAdapter_, SIGNAL(dataChanged(IEntitySPtr,int)), SLOT(onColumnAdapterDataChanged(IEntitySPtr,int)));
}

// Private constructor method
/**
  */
void AbstractSeqItem::constructor_()
{
    baseLine_ = new QGraphicsRectItem(0, 0, width(), baseLineHeight_, this);
    qreal halfBaselineHeight = baseLineHeight_ / 2.;
    qreal halfHeight = height() / 2.;
    baseLine_->setY(halfHeight - halfBaselineHeight);
    baseLine_->setPen(Qt::NoPen);
    baseLine_->setBrush(Qt::gray);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractSeqSPtr
  */
AbstractSeqSPtr AbstractSeqItem::abstractSeq() const
{
    return abstractSeq_;
}

/**
  * @returns QGraphicsRectItem *
  */
QGraphicsRectItem *AbstractSeqItem::baseLine() const
{
    return baseLine_;
}

/**
  * @returns double
  */
double AbstractSeqItem::height() const
{
    return kDefaultHeight_;
}

void AbstractSeqItem::addFeature(AbstractFeatureItem *featureItem)
{
    if (featureItem == nullptr)
        return;

    featureItem->setParentItem(this);
    featureItem->setPixelsPerUnit(pixelsPerUnit());
    features_[featureItem->type()].insertMulti(featureItem->id(), featureItem);
}

void AbstractSeqItem::removeFeature(const int featureType, const int featureId)
{
    if (!features_.contains(featureType))
        return;

    QMultiHash<int, AbstractFeatureItem *> &idFeatureHash = features_[featureType];
    if (!idFeatureHash.contains(featureId))
        return;

    // Even though this is a singularly named method, it is possible to have multiple features associated
    // with a single feature id (e.g. primers). Thus, we treat the features_ hash value as a multiple
    // valued container.
    QList<AbstractFeatureItem *> featureItems = idFeatureHash.values(featureId);
    QGraphicsScene *scene = this->scene();
    if (scene != nullptr)
        foreach (AbstractFeatureItem *item, featureItems)
            scene->removeItem(item);

    // Calling scene->removeItem releases ownership to this method.
    qDeleteAll(featureItems);
    idFeatureHash.remove(featureId);
}

void AbstractSeqItem::clearFeatures(const int featureType)
{
    if (!features_.contains(featureType))
        return;

    bool removeFromScene = scene() != nullptr;
    // According to the docs, it is more efficient to remove the item from the scene before destroying it
    // Walk through each id of featureType, and for each id, any abstractFeatureItem
    const QMultiHash<int, AbstractFeatureItem *> &idFeatureHash = features_.value(featureType);
    foreach (const int featureId, idFeatureHash.keys())
    {
        foreach (AbstractFeatureItem *item, idFeatureHash.values(featureId))
        {
            if (removeFromScene)
                scene()->removeItem(item);
            delete item;
        }
    }
    features_[featureType].clear();
}

QVector<AbstractFeatureItem *> AbstractSeqItem::findFeatures(const int featureType, const int featureId)
{
    if (!features_.contains(featureType))
        return QVector<AbstractFeatureItem *>();

    const QMultiHash<int, AbstractFeatureItem *> &idFeatureHash = features_.value(featureType);
    if (!idFeatureHash.contains(featureId))
        return QVector<AbstractFeatureItem *>();

    return idFeatureHash.values(featureId).toVector();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void AbstractSeqItem::onPixelsPerUnitChanged()
{
    QRectF rect = baseLine_->rect();
    rect.setWidth(width());
    baseLine_->setRect(rect);

    updateFeaturesPPU();
}

/**
  * @param newStart [int]
  * @returns bool
  */
bool AbstractSeqItem::setStartImpl(int newStart)
{
    ASSERT(abstractSeq_);
    if (columnAdapter_ != nullptr)
        return columnAdapter_->setData(abstractSeq_, startColumn_, newStart);

    return abstractSeq_->setStart(newStart);
}

/**
  * @param newStop [int]
  * @returns bool
  */
bool AbstractSeqItem::setStopImpl(int newStop)
{
    ASSERT(abstractSeq_);
    if (columnAdapter_ != nullptr)
        return columnAdapter_->setData(abstractSeq_, stopColumn_, newStop);

    return abstractSeq_->setStop(newStop);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param entity [const IEntitySPtr &]
  * @param column [int]
  */
void AbstractSeqItem::onColumnAdapterDataChanged(const IEntitySPtr &entity, int column)
{
    ASSERT(columnAdapter_ != nullptr);
    if (!abstractSeq_ || entity != abstractSeq_)
        return;

    if (column == startColumn_)
        setStart(columnAdapter_->data(abstractSeq_, column).toInt());
    else if (column == stopColumn_)
        setStop(columnAdapter_->data(abstractSeq_, column).toInt());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void AbstractSeqItem::updateFeaturesPPU()
{
    foreach (const int featureType, features_.keys())
    {
        QMultiHash<int, AbstractFeatureItem *> &idFeatureHash = features_[featureType];
        foreach (const int featureId, idFeatureHash.keys())
            foreach (AbstractFeatureItem *item, idFeatureHash.values(featureId))
                item->setPixelsPerUnit(pixelsPerUnit());
    }
}

