/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QStyleOptionGraphicsItem>

#include "BioStringItem.h"

#include "../../../graphics/BasicTextRenderer.h"
#include "../../../graphics/TextColorStyle.h"
#include "../../gui_misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param bioString [const BioString &]
  * @param font [const QFont &]
  * @param parentItem [QGraphicsItem *]
  */
BioStringItem::BioStringItem(const BioString &bioString, const QFont &font, QGraphicsItem *parentItem)
    : AbstractLinearItem(bioString.length(), parentItem),
      bioString_(bioString),
      textRenderer_(nullptr)
{
    textRenderer_ = new BasicTextRenderer(font);
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioString
  */
BioString BioStringItem::bioString() const
{
    return bioString_;
}

/**
  * @returns double
  */
double BioStringItem::height() const
{
    if (isVisible())
        return textRenderer_->height();

    return 0;
}

/**
  * @param painter [QPainter *]
  * @param option [const QStyleOptionGraphicsItem *]
  * @param widget [QWidget *]
  */
void BioStringItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * /* widget */)
{
    // Map the exposed rect to the relevant portion - setFlag(QGraphicsItem::ItemUsesExtendedStyleOption)
    int firstChar = unitFromPosition(option->exposedRect.left());
    int lastChar = qMin(length(), unitFromPosition(option->exposedRect.right()));

    double blockWidth = pixelsPerUnit();
    double x = leftPositionFromUnit(firstChar);
    double offset = (blockWidth - textRenderer_->width()) / 2.;      // To center the letter in the allotted space
    QPointF pointF(x + offset, 0);
    const char *ch = bioString_.constData() + firstChar - 1;
    for (int i=firstChar; i<=lastChar; ++i, ++ch)
    {
        textRenderer_->drawChar(pointF, *ch, TextColorStyle(Qt::black, Qt::transparent), painter);
        pointF.rx() += blockWidth;
    }
}

/**
  * @param newBioString [const BioString &]
  */
void BioStringItem::setBioString(const BioString &newBioString)
{
    if (newBioString == bioString_)
        return;

    if (newBioString.length() != bioString_.length())
        prepareGeometryChange();

    bioString_ = newBioString;
    update();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void BioStringItem::onPixelsPerUnitChanged()
{
    if (pixelsPerUnit() < 1)
        return;

    // Prepare geometry changed already called in super class
    QFont font = textRenderer_->font();
    font.setPointSize(::estimatePointSizeFromPixelWidth(font, pixelsPerUnit()));
    textRenderer_->setFont(font);
}
