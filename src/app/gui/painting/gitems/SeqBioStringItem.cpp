/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SeqBioStringItem.h"

#include "AbstractSeqItem.h"
#include "BioStringItem.h"
#include "../../../core/macros.h"

static const double kDefaultVerticalSpacing = 5.;
static const double kDefaultBioStringVisiblePPUThreshold = 4.;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * Takes ownership of aminoSeqItem.
  *
  * @param abstractSeqItem [AbstractSeqItem *]
  * @param font [const QFont &]
  * @param parentItem [QGraphicsItem *]
  */
SeqBioStringItem::SeqBioStringItem(AbstractSeqItem *abstractSeqItem, const QFont &font, QGraphicsItem *parentItem)
    : QGraphicsItemGroup(parentItem),
      abstractSeqItem_(abstractSeqItem),
      bioStringItem_(nullptr),
      verticalSpacing_(kDefaultVerticalSpacing),
      bioStringEnabled_(true),
      bioStringVisiblePPUThreshold_(kDefaultBioStringVisiblePPUThreshold)
{
    ASSERT(abstractSeqItem != nullptr);

    abstractSeqItem_->setParentItem(this);
    connect(abstractSeqItem_, SIGNAL(pixelsPerUnitChanged(double)), SLOT(onSeqItemPixelsPerUnitChanged(double)));

    setHandlesChildEvents(false);
    bioStringItem_ = new BioStringItem(abstractSeqItem_->abstractSeq()->abstractAnonSeq()->seq_.toBioString(),
                                       font,
                                       abstractSeqItem_);
    bioStringItem_->setY(abstractSeqItem_->height() + verticalSpacing_);

    // Initialize the BioStringItem via the slot method
    onSeqItemPixelsPerUnitChanged(abstractSeqItem_->pixelsPerUnit());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AminoSeqItem *
  */
AbstractSeqItem *SeqBioStringItem::abstractSeqItem() const
{
    return abstractSeqItem_;
}

/**
  * @returns bool
  */
bool SeqBioStringItem::bioStringEnabled() const
{
    return bioStringEnabled_;
}

/**
  * @returns BioStringItem *
  */
BioStringItem *SeqBioStringItem::bioStringItem() const
{
    return bioStringItem_;
}

/**
  * @returns double
  */
double SeqBioStringItem::height() const
{
    if (bioStringItem_->isVisible())
        return abstractSeqItem_->height() + verticalSpacing_ + bioStringItem_->height();

    return abstractSeqItem_->height();
}

/**
  * @param enabled [bool]
  */
void SeqBioStringItem::setBioStringEnabled(bool enabled)
{
    if (enabled == bioStringEnabled_)
        return;

    bioStringEnabled_ = enabled;

    if (abstractSeqItem_->pixelsPerUnit() < bioStringVisiblePPUThreshold_)
        return;

    bioStringItem_->setVisible(bioStringEnabled_);
    emit heightChanged(height());
}

/**
  * @param newVerticalSpacing [const double]
  */
void SeqBioStringItem::setVerticalSpacing(const double newVerticalSpacing)
{
    if (qFuzzyCompare(verticalSpacing_, newVerticalSpacing))
        return;

    verticalSpacing_ = newVerticalSpacing;
    emit heightChanged(height());
}

/**
  * @returns double
  */
double SeqBioStringItem::verticalSpacing() const
{
    return verticalSpacing_;
}

/**
  * @returns double
  */
double SeqBioStringItem::width() const
{
    return abstractSeqItem_->width();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newPixelsPerUnit [const double]
  */
void SeqBioStringItem::setPixelsPerUnit(const double newPixelsPerUnit)
{
    abstractSeqItem_->setPixelsPerUnit(newPixelsPerUnit);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param newPixelsPerUnit [const double]
  */
void SeqBioStringItem::onSeqItemPixelsPerUnitChanged(const double newPixelsPerUnit)
{
    // Even though this item may or may not be visible, it is important to set its pixels per unit so that
    // the scene rect may be appropriately updated.
    bioStringItem_->setPixelsPerUnit(newPixelsPerUnit);

    if (newPixelsPerUnit >= bioStringVisiblePPUThreshold_)
    {
        if (bioStringEnabled_)
        {
            double oldHeight = abstractSeqItem_->height();
            bioStringItem_->show();

            if (!qFuzzyCompare(bioStringItem_->height(), oldHeight))
                emit heightChanged(height());
        }
    }
    else
    {
        if (bioStringEnabled_)
        {
            bioStringItem_->hide();
            emit heightChanged(height());
        }
    }
}
