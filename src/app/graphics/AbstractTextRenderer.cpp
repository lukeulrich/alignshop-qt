/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QPointF>
#include <QtGui/QPainter>

#include "AbstractCharPixelMetrics.h"
#include "AbstractTextRenderer.h"
#include "TextColorStyle.h"
#include "../core/constants.h"
#include "../core/macros.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Scale must be greater than or equal to zero.
  *
  * @param font [const QFont &]
  * @param parent [QObject *]
  */
AbstractTextRenderer::AbstractTextRenderer(const QFont &font, QObject *parent) : QObject(parent)
{
    // ---------------------------
    // Initialize the text layouts
    QTextOption textOption;
    textOption.setUseDesignMetrics(true);
    for (const char *x = constants::k7BitCharacters; *x; ++x)
    {
        QTextLayout &textLayout = textLayouts_[*x-33];

        textLayout.setTextOption(textOption);
        textLayout.setCacheEnabled(true);
        textLayout.setText(QChar(*x));
        textLayout.setFont(font);
        textLayout.beginLayout();
        textLayout.createLine();
        textLayout.endLayout();
    }
}

/**
  */
AbstractTextRenderer::~AbstractTextRenderer()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns qreal
  */
qreal AbstractTextRenderer::baseline() const
{
    ASSERT(charPixelMetrics());
    return charPixelMetrics()->baseline();
}

/**
  * Utilizes a layout for rendering.
  *
  * @param pointF [const QPointF &]
  * @param character [const char character]
  * @param textColorStyle [const textColorStyle &]
  * @param painter [QPainter *]
  */
void AbstractTextRenderer::drawChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter)
{
    ASSERT(charPixelMetrics());
    painter->fillRect(QRectF(pointF, size()), textColorStyle.background_);
    painter->save();

    // Layout Origin is already in scaled space, thus translate before the painter scale method call
    painter->setFont(charPixelMetrics()->font());
    painter->translate(pointF + charPixelMetrics()->layoutOrigin(ch));
    painter->scale(scale(), scale());
    painter->setPen(textColorStyle.foreground_);
    textLayouts_[ch-33].draw(painter, QPointF());
    painter->restore();
}

/**
  * @returns QFont
  */
QFont AbstractTextRenderer::font() const
{
    ASSERT(charPixelMetrics());
    return charPixelMetrics()->font();
}

/**
  * @returns qreal
  */
qreal AbstractTextRenderer::height() const
{
    ASSERT(charPixelMetrics());
    return charPixelMetrics()->height();
}

/**
  * @returns qreal
  */
qreal AbstractTextRenderer::scale() const
{
    ASSERT(charPixelMetrics());
    return charPixelMetrics()->scale();
}

/**
  * @returns QSizeF
  */
QSizeF AbstractTextRenderer::size() const
{
    return QSizeF(width(), height());
}

/**
  * @returns qreal
  */
qreal AbstractTextRenderer::width() const
{
    ASSERT(charPixelMetrics());
    return charPixelMetrics()->width();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param newFont [const QFont &]
  */
void AbstractTextRenderer::setFont(const QFont &newFont)
{
    ASSERT(charPixelMetrics());

    if (newFont == this->font())
        return;

    // Rebuild the layouts, since the font has been updated
    for (const char *x = constants::k7BitCharacters; *x; ++x)
    {
        QTextLayout &textLayout = textLayouts_[*x-33];
        textLayout.setFont(newFont);
        textLayout.beginLayout();
        textLayout.createLine();
        textLayout.endLayout();
    }

    charPixelMetrics()->setFont(newFont);

    emit fontChanged();
}

/**
  * Scale must be greater than zero.
  *
  * @param scale [qreal]
  */
void AbstractTextRenderer::setScale(qreal scale)
{
    ASSERT(scale > 0);
    ASSERT(charPixelMetrics());

    if (qFuzzyCompare(scale, charPixelMetrics()->scale()))
        return;

    charPixelMetrics()->setScale(scale);

    emit scaleChanged();
}
