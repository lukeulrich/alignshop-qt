/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "RangeHandlePairItem.h"

#include "AbstractRangeItem.h"
#include "RangeHandleItem.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param targetRangeItem [AbstractRangeItem *]
  * @param diameter [double]
  * @param parentItem [QGraphicsItem *]
  */
RangeHandlePairItem::RangeHandlePairItem(AbstractRangeItem *targetRangeItem, double diameter, QGraphicsItem *parentItem)
    : QGraphicsItemGroup(parentItem),
      startHandle_(nullptr),
      stopHandle_(nullptr)
{
    ASSERT(targetRangeItem != nullptr);

    // Ownership gets passed to the targetRangeItem
    startHandle_ = new RangeHandleItem(targetRangeItem, RangeHandleItem::eStartHandle, diameter);
    stopHandle_ = new RangeHandleItem(targetRangeItem, RangeHandleItem::eStopHandle, diameter);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns double
  */
double RangeHandlePairItem::bottomStemHeight() const
{
    return startHandle_->bottomStemHeight();
}

/**
  * @returns double
  */
double RangeHandlePairItem::coreStemHeight() const
{
    return startHandle_->coreStemHeight();
}

/**
  * @returns double
  */
double RangeHandlePairItem::diameter() const
{
    return startHandle_->diameter();
}

/**
  * @returns double
  */
double RangeHandlePairItem::fadeRectOpacity() const
{
    return startHandle()->fadeRectOpacity();
}

/**
  * @returns double
  */
double RangeHandlePairItem::height() const
{
    return startHandle_->height();
}

/**
  * @param targetRangeItem [AbstractRangeItem *]
  */
void RangeHandlePairItem::setTargetRangeItem(AbstractRangeItem *targetRangeItem)
{
    startHandle_->setTargetRangeItem(targetRangeItem);
    stopHandle_->setTargetRangeItem(targetRangeItem);
}

/**
  * @returns RangeHandleItem *
  */
RangeHandleItem *RangeHandlePairItem::startHandle() const
{
    return startHandle_;
}

/**
  * @returns RangeHandleItem *
  */
RangeHandleItem *RangeHandlePairItem::stopHandle() const
{
    return stopHandle_;
}

/**
  * @returns double
  */
double RangeHandlePairItem::stemHeight() const
{
    return startHandle_->stemHeight();
}

/**
  * @returns double
  */
double RangeHandlePairItem::topStemHeight() const
{
    return startHandle_->topStemHeight();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newBottomStemHeight [double]
  */
void RangeHandlePairItem::setBottomStemHeight(double newBottomStemHeight)
{
    startHandle_->setBottomStemHeight(newBottomStemHeight);
    stopHandle_->setBottomStemHeight(newBottomStemHeight);
}

/**
  * @param newCoreStemHeight [double]
  */
void RangeHandlePairItem::setCoreStemHeight(double newCoreStemHeight)
{
    startHandle_->setCoreStemHeight(newCoreStemHeight);
    stopHandle_->setCoreStemHeight(newCoreStemHeight);
}

/**
  * @param newFadeRectOpacity [double]
  */
void RangeHandlePairItem::setFadeRectOpacity(double newFadeRectOpacity)
{
    startHandle_->setFadeRectOpacity(newFadeRectOpacity);
    stopHandle_->setFadeRectOpacity(newFadeRectOpacity);
}

/**
  * @param newHandleIndex [const int]
  */
void RangeHandlePairItem::setHandleIndex(const int newHandleIndex)
{
    startHandle_->setHandleIndex(newHandleIndex);
    stopHandle_->setHandleIndex(newHandleIndex);
}

/**
  * @param newTopStemHeight [double]
  */
void RangeHandlePairItem::setTopStemHeight(double newTopStemHeight)
{
    startHandle_->setTopStemHeight(newTopStemHeight);
    stopHandle_->setTopStemHeight(newTopStemHeight);
}
