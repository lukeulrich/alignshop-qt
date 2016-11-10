/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

#include "LinearRulerItem.h"
#include "../../gui_misc.h"

// Note: if these are changed, the corresponding code that utilizes it also needs updated (paint)
static const int kMajorTickMarkStep = 10.;
static const int kMinorTickMarkStep = 5.;

static const double kDefaultMajorTickHeight = 6.;
static const double kDefaultLabelTickPadding = 2.;
static const double kDefaultMinorTickHeight = kDefaultMajorTickHeight / 2.;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param length [int]
  * @param parentItem [QGraphicsItem *]
  */
LinearRulerItem::LinearRulerItem(int length, QGraphicsItem *parentItem)
    : AbstractLinearItem(length, parentItem),
      font_("Monospace", 8),
      fontMetrics_(font_),
      majorTickHeight_(kDefaultMajorTickHeight),
      minorTickHeight_(kDefaultMinorTickHeight)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns double
  */
double LinearRulerItem::height() const
{
    return fontMetrics_.height() + kDefaultLabelTickPadding + majorTickHeight_;
}


/**
  * @param painter [QPainter *]
  * @param option [const QStyleOptionGraphicsItem *]
  * @param widget [QWidget *]
  */
void LinearRulerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */)
{
    // TODO: Optimize to only draw the exposed rectangle portion
    QPen pen = painter->pen();
    painter->setFont(font_);

    // Determine the label frequency in units
    int labelIncrement = 10;
    // 0 -> 1
    // 1 -> 2
    // 2 -> 5
    // 3 -> 10 ** seed labelIncrement
    for (int i=4; labelIncrement <= length() && !labelIncrementFits(labelIncrement); ++i)
        labelIncrement = nextLabelIncrement(i);

    double baseLineY = fontMetrics_.ascent();

    bool drawFirstLabel = true;
    double rightSideOfFirstLabel = fontMetrics_.width("1");
    if (labelIncrement > length())
    {
        // Can we fit the end label without cramping the first label?
        QString endLabel = QString::number(length());
        double leftSideOfLastLabel = rightPositionFromUnit(length()) - fontMetrics_.width(endLabel);

        bool drawLastLabel = (leftSideOfLastLabel - rightSideOfFirstLabel >= minimumWidthBetweenLabels()) ||
                             leftSideOfLastLabel >= x();
        if (drawLastLabel)
            painter->drawText(leftSideOfLastLabel, baseLineY, endLabel);
        drawFirstLabel = !drawLastLabel && rightSideOfFirstLabel <= width();
    }

    if (drawFirstLabel)
        painter->drawText(leftPositionFromUnit(1), baseLineY, "1");

    pen.setColor(Qt::black);
    painter->setPen(pen);
    double x = middlePositionFromUnit(1);
    painter->drawLine(x, height() - majorTickHeight_, x, height());

    for (int i=kMinorTickMarkStep, z=length(); i<= z; i += kMinorTickMarkStep)
    {
        bool isMajorTick = (i % kMajorTickMarkStep == 0);

        x = middlePositionFromUnit(i);
        if (isMajorTick)
        {
            pen.setColor(Qt::black);
            painter->setPen(pen);
            painter->drawLine(x, height() - majorTickHeight_, x, height());

            // Draw a label if possible
            if (i % labelIncrement == 0)
            {
                double textWidth = fontMetrics_.width(QString::number(i));

                // Special case: last label should be moved inside of width
                double textX = 0;
                if (i != z)
                {
                    textX = middlePositionFromUnit(i) - (textWidth / 2);
                    if (textX + textWidth >= width())
                        textX = width() - textWidth;
                }
                else
                {
                    textX = rightPositionFromUnit(i) - textWidth;
                }
                painter->drawText(textX, baseLineY, QString::number(i));
            }
        }
        // Only draw the minor ticks if the pixels per unit spans at least three pixels
        else if (pixelsPerUnit() * kMinorTickMarkStep >= 3.)
        {
            pen.setColor(QColor(128, 128, 128));
            painter->setPen(pen);
            painter->drawLine(x, height() - minorTickHeight_, x, height());
        }
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param labelIncrement [const int]
  * @returns bool
  */
bool LinearRulerItem::labelIncrementFits(const int labelIncrement) const
{
    int lastNumber = length() - (length() % labelIncrement);
    QString lastLabel = QString::number(lastNumber);
    int nextLastNumber = qMax(1, lastNumber - labelIncrement);      // Can never be below 1
    QString nextLastLabel = QString::number(nextLastNumber);

    double a = middlePositionFromUnit(nextLastNumber) + fontMetrics_.width(nextLastLabel) / 2.;
    double b = middlePositionFromUnit(lastNumber) - fontMetrics_.width(lastLabel) / 2.;

    return (b - a >= minimumWidthBetweenLabels());
}

double LinearRulerItem::minimumWidthBetweenLabels() const
{
    return fontMetrics_.width('A') * 2.;
}

/**
  * @param rank [const int]
  * @returns int
  */
int LinearRulerItem::nextLabelIncrement(const int rank) const
{
    int rootRank = rank % 3 + 1;
    if (rootRank == 3)
        rootRank = 5;

    return rootRank * pow(10., int(rank / 3));
}
