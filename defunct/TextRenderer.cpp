/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "TextRenderer.h"

#include <QtGui/QPainter>
#include <QtGui/QTextLayout>

#include <cmath>

#include "global.h"

#include <QFontMetrics>

#include <QtSvg/QSvgGenerator>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Scale must be greater than or equal to zero.
  *
  * @param font [const QFont &]
  * @param scale [qreal]
  * @param parent [QObject *]
  */
TextRenderer::TextRenderer(const QFont &font, qreal scale, QObject *parent) :
    QObject(parent),
    font_(font),
    scale_(scale),
    charMetricsF_(font),
    charMetrics_(font),
    useFloatMetrics_(false)
{
    ASSERT(scale_ > 0);

    // ---------------------------
    // Initialize the text layouts
    QTextOption textOption;
    textOption.setUseDesignMetrics(true);
    const char *x = constants::k7BitCharacters;
    while (*x)
    {
        QTextLayout &textLayout = textLayouts_[*x-33];

        textLayout.setTextOption(textOption);
        textLayout.setCacheEnabled(true);
        textLayout.setText(QChar(*x));
        textLayout.setFont(font_);
        textLayout.beginLayout();
        textLayout.createLine();
        textLayout.endLayout();

        ++x;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param pointF [const QPointF &]
  * @param character [const char character]
  * @param textColorStyle [const textColorStyle &]
  * @param painter [QPainter *]
  */
void TextRenderer::drawChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter) const
{
    ASSERT(painter);
    ASSERT(ch > 32 && ch != 127);

    // Fill in the background. Note, size() returns the scaled size
    if (useFloatMetrics_)
        painter->fillRect(QRectF(pointF, sizeF()), textColorStyle.background_);
    else
        painter->fillRect(QRectF(pointF, size()), textColorStyle.background_);

    // Setup the font, pen, and scale
    painter->save();
//    painter->setPen(textColorStyle.foreground_);
    if (!pointF.isNull())
        painter->translate(pointF);
    painter->scale(scale_, scale_);

//    QPointF point = charMetrics_.layoutOrigin(ch);
//    if (!useFloatMetrics_)
//    {
//        point.rx() = scale_ * charMetrics_.inkOnlyOrigin(ch).x() + ( scale_ * (charMetrics_.width() - charMetrics_.inkSize(ch).width()) / 2.);
//        point.rx() /= scale_;

//        if (ch == 'Y')
//            qDebug() << (static_cast<qreal>(qRound(charMetrics_.width())) - charMetricsF_.width()) / 2.;

        // Add in the error due to rounding
//        point.rx() += (static_cast<qreal>(qRound(scale_ * charMetrics_.width())) - scale_ * charMetricsF_.width()) / 2. / scale_;
//        point.ry() += (static_cast<qreal>(qRound(scale_ * charMetrics_.height())) - scale_ * charMetricsF_.height()) / 2. / scale_;
//        point.rx() += (static_cast<qreal>(qRound(charMetrics_.width())) - charMetricsF_.width()) / 2.;
//        point.ry() += (static_cast<qreal>(qRound(charMetrics_.height())) - charMetricsF_.height()) / 2.;
//    }
//    textLayouts_[ch-33].draw(painter, point);

//    if (useFloatMetrics_)
//        textLayouts_[ch-33].draw(painter, charMetricsF_.layoutOrigin(ch));
//    else
//        textLayouts_[ch-33].draw(painter, charMetrics_.layoutOrigin(ch));


    painter->setPen(Qt::NoPen);
    painter->setBrush(textColorStyle.foreground_);
    QPainterPath textPath;
    textPath.addText(charMetrics_.origin(ch), font_, QChar(ch));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPath(textPath);
    painter->restore();
}

/**
  * @returns QFont
  */
QFont TextRenderer::font() const
{
    return font_;
}

/**
  * @returns qreal
  */
qreal TextRenderer::height() const
{
    // >> Special note!!
    // If useFloatingPoint_ is false and qRound is changed, it is vital to update the AbstractMsaArea::setFont() method
    // to accommodate a different mimimum zoom calculation.
    return (useFloatMetrics_) ? scale_ * charMetricsF_.height() : qRound(scale_ * charMetrics_.height());
}

/**
  * @returns qreal
  */
qreal TextRenderer::scale() const
{
    return scale_;
}

/**
  * @param enable [bool]
  *
  * UNTESTED
  */
void TextRenderer::setUseFloatMetrics(bool enable)
{
    useFloatMetrics_ = enable;
}

/**
  * @returns QSizeF
  */
QSizeF TextRenderer::sizeF() const
{
    return QSizeF(width(),
                  height());
}

/**
  * @returns QSize
  */
QSize TextRenderer::size() const
{
    return QSize(static_cast<int>(ceil(width())),
                 static_cast<int>(ceil(height())));
}

/**
  * @returns qreal
  */
qreal TextRenderer::unscaledHeight() const
{
    return (useFloatMetrics_) ? charMetricsF_.height() : charMetrics_.height();
}

/**
  * @returns qreal
  */
qreal TextRenderer::unscaledWidth() const
{
    return (useFloatMetrics_) ? charMetricsF_.width() : charMetrics_.width();
}

/**
  * @returns QSizeF
  */
QSizeF TextRenderer::unscaledSize() const
{
    return QSizeF(unscaledWidth(),
                  unscaledHeight());
}

/**
  * @returns bool
  */
bool TextRenderer::useFloatMetrics() const
{
    return useFloatMetrics_;
}

/**
  * @returns qreal
  */
qreal TextRenderer::width() const
{
    // >> Special note!!
    // If useFloatingPoint_ is false and qRound is changed, it is vital to update the AbstractMsaArea::setFont() method
    // to accommodate a different mimimum zoom calculation.
    return (useFloatMetrics_) ? scale_ * charMetricsF_.width() : qRound(scale_ * charMetrics_.width());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * If the font is changed, set the font, recalculate its metrics, and then clear the cache.
  *
  * @param font [const QFont &]
  */
void TextRenderer::setFont(const QFont &font)
{
    if (font == font_)
        return;

    font_ = font;
    charMetricsF_ = CharMetricsF(font_);
    charMetrics_ = CharMetrics(font_);

    // Rebuild the layouts, since the font has been updated
    const char *x = constants::k7BitCharacters;
    while (*x)
    {
        QTextLayout &textLayout = textLayouts_[*x-33];
        textLayout.setFont(font_);
        textLayout.beginLayout();
        textLayout.createLine();
        textLayout.endLayout();

        ++x;
    }

    emit fontChanged();
}

/**
  * Scale must be greater than zero. If the scale is changed, then the cache is cleared.
  *
  * @param scale [qreal]
  */
void TextRenderer::setScale(qreal scale)
{
    ASSERT(scale > 0);

    if (qFuzzyCompare(scale, scale_))
        return;

    scale_ = scale;

    emit scaleChanged();
}
