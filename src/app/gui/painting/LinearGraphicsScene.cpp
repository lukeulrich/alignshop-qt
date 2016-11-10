/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LinearGraphicsScene.h"

#include "gitems/AbstractLinearItem.h"
#include "gitems/SeqBioStringItem.h"
#include "../../core/macros.h"

static const double kDefaultPixelsPerUnit = .65;
static const double kDefaultScaleFactor = .05;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
LinearGraphicsScene::LinearGraphicsScene(QObject *parent)
    : QGraphicsScene(parent),
      pixelsPerUnit_(kDefaultPixelsPerUnit),
      scaleFactor_(kDefaultScaleFactor),
      maxPixelsPerUnit_(0.)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param abstractLinearItem [AbstractLinearItem *]
  */
void LinearGraphicsScene::addLinearItem(AbstractLinearItem *abstractLinearItem)
{
    if (abstractLinearItem == nullptr)
        return;

    abstractLinearItem->setPixelsPerUnit(pixelsPerUnit_);

    connect(this, SIGNAL(pixelsPerUnitChanged(double)), abstractLinearItem, SLOT(setPixelsPerUnit(double)));
    QGraphicsScene::addItem(abstractLinearItem);
}

/**
  * @param seqBioStringItem [SeqBioStringItem *]
  */
void LinearGraphicsScene::addSeqBioStringItem(SeqBioStringItem *seqBioStringItem)
{
    if (seqBioStringItem == nullptr)
        return;

    seqBioStringItem->setPixelsPerUnit(pixelsPerUnit_);

    connect(this, SIGNAL(pixelsPerUnitChanged(double)), seqBioStringItem, SLOT(setPixelsPerUnit(double)));
    QGraphicsScene::addItem(seqBioStringItem);
}

/**
  * @returns double
  */
double LinearGraphicsScene::maxPixelsPerUnit() const
{
    return maxPixelsPerUnit_;
}

/**
  * @param padding [const double]
  * @returns QRectF
  */
QRectF LinearGraphicsScene::paddedItemsBoundingRect(const double padding) const
{
    return itemsBoundingRect().adjusted(-padding, -padding, padding, padding);
}

/**
  * @param verticalPadding [const double]
  * @param horizontalPadding [const double]
  * @returns QRectF
  */
QRectF LinearGraphicsScene::paddedItemsBoundingRect(const double verticalPadding, const double horizontalPadding) const
{
    return itemsBoundingRect().adjusted(-horizontalPadding, -verticalPadding, horizontalPadding, verticalPadding);
}

/**
  * @returns double
  */
double LinearGraphicsScene::pixelsPerUnit() const
{
    return pixelsPerUnit_;
}

/**
  * @returns double
  */
double LinearGraphicsScene::scaleFactor() const
{
    return scaleFactor_;
}

/**
  * If maxPixelsPerUnit is zero or negative it is ignored.
  *
  * @param maxPixelsPerUnit [const double]
  */
void LinearGraphicsScene::setMaxPixelsPerUnit(const double maxPixelsPerUnit)
{
    maxPixelsPerUnit_ = maxPixelsPerUnit;
    if (maxPixelsPerUnit_ >= 0. && pixelsPerUnit_ > maxPixelsPerUnit_)
        setPixelsPerUnit(maxPixelsPerUnit_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newPixelsPerUnit [const double]
  */
void LinearGraphicsScene::setPixelsPerUnit(const double newPixelsPerUnit)
{
    double clampedPixelsPerUnit = (maxPixelsPerUnit_ >= 0.) ? qMin(newPixelsPerUnit, maxPixelsPerUnit_) : newPixelsPerUnit;
    if (qFuzzyCompare(pixelsPerUnit_, clampedPixelsPerUnit))
        return;

    pixelsPerUnit_ = clampedPixelsPerUnit;
    emit pixelsPerUnitChanged(pixelsPerUnit_);
}

/**
  * @param newScaleFactor [const double]
  */
void LinearGraphicsScene::setScaleFactor(const double newScaleFactor)
{
    scaleFactor_ = newScaleFactor;
}

/**
  * @param direction [const int]
  */
void LinearGraphicsScene::scalePixelsPerUnit(const int direction)
{
    if (direction > 0)
        setPixelsPerUnit(pixelsPerUnit_ * (1. + scaleFactor_));
    else if (direction < 0)
        setPixelsPerUnit(pixelsPerUnit_ * (1. / (1. + scaleFactor_)));
}
