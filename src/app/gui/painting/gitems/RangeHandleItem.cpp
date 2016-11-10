/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPen>
#include <QtGui/QStyleOptionGraphicsItem>

#include "RangeHandleItem.h"
#include "AbstractRangeItem.h"
#include "../../../core/global.h"
#include "../../../core/macros.h"

static const double kDefaultTopStemHeight = 5.;
static const double kDefaultBottomStemHeight = 5.;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param targetRangeItem [AbstractRangeItem *]
  * @param handleType [HandleType]
  * @param diameter [double]
  */
RangeHandleItem::RangeHandleItem(AbstractRangeItem *targetRangeItem, HandleType handleType, double diameter)
    : QGraphicsEllipseItem(0, 0, diameter, diameter),
      targetRangeItem_(nullptr),
      handleType_(handleType),
      diameter_(diameter),
      radius_(diameter / 2.),
      topStemHeight_(kDefaultTopStemHeight),
      coreStemHeight_(0.),
      bottomStemHeight_(kDefaultBottomStemHeight),
      handleIndex_(0),
      oldHandleIndex_(0),
      stemItem_(nullptr),
      fadeRectItem_(nullptr)
{
    ASSERT(diameter_ > 0);

    // ---------
    // Stem item
    stemItem_ = new QGraphicsLineItem(this);
    QPen pen;
    pen.setStyle(Qt::DashLine);
    stemItem_->setPen(pen);
    stemItem_->setPos(radius_, diameter_);

    // --------------
    // Fade Rect item
    fadeRectItem_ = new QGraphicsRectItem(this);
    fadeRectItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    fadeRectItem_->setBrush(Qt::white);
    fadeRectItem_->setPen(Qt::NoPen);
    fadeRectItem_->setOpacity(.75);
    fadeRectItem_->setPos(radius_, diameter_ + topStemHeight_);

    // -------------
    // Assign target
    setTargetRangeItem(targetRangeItem);

    // Other setup
    updateFadeRect();
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns double
  */
double RangeHandleItem::bottomStemHeight() const
{
    return bottomStemHeight_;
}

/**
  * @returns double
  */
double RangeHandleItem::coreStemHeight() const
{
    return coreStemHeight_;
}

/**
  * @returns double
  */
double RangeHandleItem::diameter() const
{
    return diameter_;
}

/**
  * @returns double
  */
double RangeHandleItem::fadeRectOpacity() const
{
    return fadeRectItem_->opacity();
}

/**
  * @returns double
  */
double RangeHandleItem::height() const
{
    return diameter_ + stemHeight();
}

/**
  * @param painter [QPainter *]
  * @param option [const QStyleOptionGraphicsItem *]
  * @param widget [QWidget *]
  */
void RangeHandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (targetRangeItem_ == nullptr)
        return;

    // Prevent the dashed selection line from being drawn if in a selected state
    QStyleOptionGraphicsItem itemOption = *option;
    itemOption.state &= !QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &itemOption, widget);
}

/**
  * @param targetRangeItem [AbstractRangeItem *]
  */
void RangeHandleItem::setTargetRangeItem(AbstractRangeItem *targetRangeItem)
{
    if (targetRangeItem_ != nullptr)
    {
        if (handleType_ == eStartHandle)
            disconnect(targetRangeItem_, SIGNAL(startChanged(int)), this, SLOT(setHandleIndex(int)));
        else // if (handleType_ == eStopHandle)
            disconnect(targetRangeItem_, SIGNAL(stopChanged(int)), this, SLOT(setHandleIndex(int)));

        disconnect(targetRangeItem_, SIGNAL(pixelsPerUnitChanged(double)), this, SLOT(onTargetPixelsPerUnitChanged()));
    }

    handleIndex_ = 0;
    targetRangeItem_ = targetRangeItem;
    if (targetRangeItem_ != nullptr)
    {
        setPos(handlePosition(), -diameter_ - topStemHeight_);
        coreStemHeight_ = targetRangeItem_->height();
        stemItem_->setLine(0, 0, 0, stemHeight());

        connect(targetRangeItem_, SIGNAL(pixelsPerUnitChanged(double)), SLOT(onTargetPixelsPerUnitChanged()));
        if (handleType_ == eStartHandle)
        {
            handleIndex_ = targetRangeItem_->start();
            connect(targetRangeItem_, SIGNAL(startChanged(int)), SLOT(setHandleIndex(int)));
        }
        else // if (handleType_ == eStopHandle)
        {
            handleIndex_ = targetRangeItem_->stop();
            connect(targetRangeItem_, SIGNAL(stopChanged(int)), SLOT(setHandleIndex(int)));
        }
        setZValue(targetRangeItem_->zValue() + 1);
    }

    setParentItem(targetRangeItem_);
}

/**
  * @returns double
  */
double RangeHandleItem::stemHeight() const
{
    return topStemHeight_ + coreStemHeight_ + bottomStemHeight_;
}

/**
  * @returns double
  */
double RangeHandleItem::topStemHeight() const
{
    return topStemHeight_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newBottomStemHeight [double]
  */
void RangeHandleItem::setBottomStemHeight(double newBottomStemHeight)
{
    ASSERT(newBottomStemHeight >= 0.);

    // Normalize
    bottomStemHeight_ = qMax(0., newBottomStemHeight);
    updateStemLine();
}

/**
  * @param newCoreStemHeight [double]
  */
void RangeHandleItem::setCoreStemHeight(double newCoreStemHeight)
{
    ASSERT(newCoreStemHeight >= 0.);

    // Normalize
    coreStemHeight_ = qMax(0., newCoreStemHeight);

    // Update stem line and fade rect
    updateStemLine();
    QRectF rect = fadeRectItem_->rect();
    rect.setHeight(coreStemHeight_);
    fadeRectItem_->setRect(rect);
}

/**
  * @param newFadeRectOpacity [double]
  */
void RangeHandleItem::setFadeRectOpacity(double newFadeRectOpacity)
{
    fadeRectItem_->setOpacity(newFadeRectOpacity);
}

/**
  * In this case, index refers to an integral unit of this linear item.
  *
  * @param newHandleIndex [const int]
  */
void RangeHandleItem::setHandleIndex(const int newHandleIndex)
{
    if (targetRangeItem_ == nullptr)
        return;

    if (handleIndex_ == newHandleIndex)
        return;

    setX(targetRangeItem_->middlePositionFromUnit(newHandleIndex) - radius_);

    // This will trigger an item change which will update the handle index accordingly.
}

/**
  * @param newTopStemHeight [double]
  */
void RangeHandleItem::setTopStemHeight(double newTopStemHeight)
{
    ASSERT(newTopStemHeight >= 0.);

    // Normalize
    topStemHeight_ = qMax(0., newTopStemHeight);

    // Update stem line and fade rect positions
    updateStemLine();
    fadeRectItem_->setPos(radius_, diameter_ + topStemHeight_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param event [QGraphicsSceneHoverEvent *]
  */
void RangeHandleItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /* event */)
{
    setBrush(Qt::blue);
}

/**
  * @param event [QGraphicsSceneHoverEvent *]
  */
void RangeHandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /* event */)
{
    setBrush(Qt::white);
}

/**
  * @param change [GraphicsItemChange]
  * @param value [const QVariant &]
  * @returns QVariant
  */
QVariant RangeHandleItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (targetRangeItem_ != nullptr)
    {
        if (change == ItemPositionChange)
        {
            // Constrain movement to horizontal axis
            QPointF newPos = value.toPointF();
            newPos.setY(y());

            // Constrain horizontal position to discrete integral values relevant to their start and stop positions
            handleIndex_ = clampIndex(targetRangeItem_->unitFromPosition(newPos.x() + radius_));
            newPos.setX(handlePosition(handleIndex_));
            return newPos;
        }
        else if (change == ItemPositionHasChanged)
        {
            updateFadeRect();
        }
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

/**
  * @param event [QGraphicsSceneMouseEvent *]
  */
void RangeHandleItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        setBrush(Qt::green);
        setCursor(Qt::SizeHorCursor);

        oldHandleIndex_ = handleIndex_;
    }

    QGraphicsEllipseItem::mousePressEvent(event);
}

/**
  * @param event [QGraphicsSceneMouseEvent *]
  */
void RangeHandleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton &&
        !setTargetIndex(handleIndex_))
    {
        handleIndex_ = oldHandleIndex_;
    }

    setBrush(Qt::blue);
    setCursor(Qt::ArrowCursor);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void RangeHandleItem::onTargetPixelsPerUnitChanged()
{
    if (targetRangeItem_ == nullptr)
        return;

    updateStemLine();
    setX(targetRangeItem_->middlePositionFromUnit(handleIndex_) - radius_);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param index [const int]
  * @returns int
  */
int RangeHandleItem::clampIndex(const int index) const
{
    if (targetRangeItem_ == nullptr)
        return 0.;

    return (handleType_ == eStartHandle) ? qBound(1, index, targetRangeItem_->stop())
                                         : qBound(targetRangeItem_->start(), index, targetRangeItem_->length());
}

/**
  * @returns double
  */
double RangeHandleItem::handlePosition() const
{
    if (targetRangeItem_ == nullptr)
        return 0.;

    if (handleType_ == eStartHandle)
        return targetRangeItem_->leftPositionFromUnit(targetRangeItem_->start()) - radius_;
    else
        return targetRangeItem_->rightPositionFromUnit(targetRangeItem_->stop()) - radius_;
}

/**
  * @param index [const int]
  * @returns double
  */
double RangeHandleItem::handlePosition(const int index) const
{
    if (targetRangeItem_ == nullptr)
        return 0.;

    return (handleType_ == eStartHandle) ? targetRangeItem_->leftPositionFromUnit(index) - radius_
                                         : targetRangeItem_->rightPositionFromUnit(index) - radius_;
}

/**
  * @param index [const int]
  * @returns bool
  */
bool RangeHandleItem::setTargetIndex(const int index) const
{
    if (targetRangeItem_ == nullptr)
        return false;

    return (handleType_ == eStartHandle) ? targetRangeItem_->setStart(index)
                                         : targetRangeItem_->setStop(index);
}

/**
  */
void RangeHandleItem::updateFadeRect()
{
    if (targetRangeItem_ == nullptr)
        return;

    QRectF rect = fadeRectItem_->rect();
    rect.setHeight(coreStemHeight_);
    if (handleType_ == eStartHandle)
    {
        fadeRectItem_->setX(-x());
        rect.setWidth(x() + radius_);
    }
    else
    {
        rect.setWidth(targetRangeItem_->width() - x() - radius_);
    }
    fadeRectItem_->setRect(rect);
}

/**
  */
void RangeHandleItem::updateStemLine()
{
    stemItem_->setLine(0, 0, 0, stemHeight());
}
