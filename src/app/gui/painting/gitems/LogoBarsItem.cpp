/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVector>

#include <QtGui/QBrush>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsItemGroup>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QPen>

#include "LogoBarsItem.h"
#include "InfoUnitItem.h"
#include "../../gui_misc.h"

#include "../../../core/LiveInfoContentDistribution.h"
#include "../../../core/global.h"
#include "../../../graphics/CharPixelMetricsF.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parentItem [QGraphicsItem *]
  */
LogoBarsItem::LogoBarsItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem),
      liveICDistribution_(nullptr),
      metrics_(nullptr),
      barHeight_(150.),
      barWidth_(25.),
      interBarSpacing_(1.),
      barLabelSpacing_(2.),
      minBarLabelHorzPadding_(4.),
      columnIcLabelsVisible_(false)
{
    font_.setPixelSize(75);
    metrics_ = new CharPixelMetricsF(font_);

    columnIcFont_.setFamily("DejaVuSans");
    int ps = ::estimateLargestPointSizeThatFits(columnIcFont_, "0.00", barWidth_ - (2. * minBarLabelHorzPadding_));
    columnIcFont_.setPointSize(ps);
}

/**
  * @param liveICDistribution [LiveInfoContentDistribution *]
  * @param parentItem [QGraphicsItem *]
  */
LogoBarsItem::LogoBarsItem(LiveInfoContentDistribution *liveICDistribution, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem),
      liveICDistribution_(nullptr),
      metrics_(nullptr),
      barHeight_(150.),
      barWidth_(25.),
      interBarSpacing_(1.),
      barLabelSpacing_(2.),
      minBarLabelHorzPadding_(4.),
      columnIcLabelsVisible_(false)
{
    font_.setPixelSize(75);
    metrics_ = new CharPixelMetricsF(font_);

    columnIcFont_.setFamily("DejaVuSans");
    int ps = ::estimateLargestPointSizeThatFits(columnIcFont_, "0.00", barWidth_ - (2. * minBarLabelHorzPadding_));
    columnIcFont_.setPointSize(ps);

    setLiveInfoContentDistribution(liveICDistribution);
}

/**
  */
LogoBarsItem::~LogoBarsItem()
{
    delete metrics_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
bool LogoBarsItem::areColumnIcLabelsVisible() const
{
    return columnIcLabelsVisible_;
}

/**
  * @returns double
  */
double LogoBarsItem::barHeight() const
{
    return barHeight_;
}

/**
  * column is zero-based.
  *
  * @param column [int]
  * @returns QPointF
  */
QPointF LogoBarsItem::barPosition(int column) const
{
    ASSERT(column >= 0);

    return QPointF(column * (barWidth_ + interBarSpacing_), 0.);
}

/**
  * @returns double
  */
double LogoBarsItem::barWidth() const
{
    return barWidth_;
}

/**
  * @returns QRectF
  */
QRectF LogoBarsItem::boundingRect() const
{
    return QRectF(0, 0, width(), barHeight_);
}

/**
  * @returns QFont
  */
QFont LogoBarsItem::font() const
{
    return font_;
}

/**
  * @returns double
  */
double LogoBarsItem::interBarSpacing() const
{
    return interBarSpacing_;
}

/**
  * @returns LiveInfoContentDistribution *
  */
LiveInfoContentDistribution *LogoBarsItem::liveInfoContentDistribution() const
{
    return liveICDistribution_;
}

/**
  * @returns int
  */
int LogoBarsItem::nBars() const
{
    return bars_.size();
}

/**
  * @param barHeight [double]
  */
void LogoBarsItem::setBarHeight(double barHeight)
{
    ASSERT(barHeight > 0.);

    if (barHeight_ != barHeight)
    {
        barHeight_ = barHeight;
        updateLetterPositions();
    }
}

/**
  * @param barWidth [double]
  */
void LogoBarsItem::setBarWidth(double barWidth)
{
    ASSERT(barWidth > 0.);

    if (barWidth_ != barWidth)
    {
        barWidth_ = barWidth;
        updateBarPositions();
    }
}

/**
  * @param colorScheme [const CharColorScheme &]
  */
void LogoBarsItem::setColorScheme(const CharColorScheme &colorScheme)
{
    colorScheme_ = colorScheme;

    if (scene() != nullptr)
        update(boundingRect());
}

/**
  * @param font [const QFont &]
  */
void LogoBarsItem::setFont(const QFont &font)
{
    if (font_ == font)
        return;

    font_ = font;
    metrics_->setFont(font_);

    // Recreate all the bars
    qDeleteAll(bars_);
    bars_ = createBars();
}

/**
  * @param interBarSpacing [double]
  */
void LogoBarsItem::setInterBarSpacing(double interBarSpacing)
{
    ASSERT(interBarSpacing >= 0.);

    if (interBarSpacing_ == interBarSpacing)
        return;

    interBarSpacing_ = interBarSpacing;
    updateBarPositions();
}

/**
  * @param liveICDistribution [LiveInfoContentDistribution *]
  */
void LogoBarsItem::setLiveInfoContentDistribution(LiveInfoContentDistribution *liveICDistribution)
{
    if (liveICDistribution_ == liveICDistribution)
        return;

    if (liveICDistribution_ != nullptr)
    {
        disconnect(liveICDistribution_, SIGNAL(columnsInserted(ClosedIntRange)), this, SLOT(onSourceColumnsInserted(ClosedIntRange)));
        disconnect(liveICDistribution_, SIGNAL(columnsRemoved(ClosedIntRange)), this, SLOT(onSourceColumnsRemoved(ClosedIntRange)));
        disconnect(liveICDistribution_, SIGNAL(dataChanged(ClosedIntRange)), this, SLOT(onSourceDataChanged(ClosedIntRange)));
    }

    qDeleteAll(bars_);
    bars_.clear();

    liveICDistribution_ = liveICDistribution;
    if (liveICDistribution_ != nullptr)
    {
        bars_ = createBars();
        connect(liveICDistribution_, SIGNAL(columnsInserted(ClosedIntRange)), SLOT(onSourceColumnsInserted(ClosedIntRange)));
        connect(liveICDistribution_, SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(onSourceColumnsRemoved(ClosedIntRange)));
        connect(liveICDistribution_, SIGNAL(dataChanged(ClosedIntRange)), SLOT(onSourceDataChanged(ClosedIntRange)));
    }

    emit barsReset();

    // ISSUE? Update the scene?
}

/**
  * @returns qreal
  */
qreal LogoBarsItem::width() const
{
    if (liveICDistribution_ != nullptr)
        return (barWidth_ + interBarSpacing_) * liveICDistribution_->length() - interBarSpacing_;

    return 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
void LogoBarsItem::setColumnIcLabelsVisible(bool visible)
{
    if (visible)
        showColumnIcLabels();
    else
        hideColumnIcLabels();
}

void LogoBarsItem::showColumnIcLabels()
{
    if (columnIcLabelsVisible_)
        return;

    foreach (BarItem *barItem, bars_)
    {
        ASSERT(barItem != nullptr);
        if (barItem->columnIcLabel_ != nullptr)
            barItem->columnIcLabel_->show();
    }

    columnIcLabelsVisible_ = true;
}

void LogoBarsItem::hideColumnIcLabels()
{
    if (!columnIcLabelsVisible_)
        return;

    foreach (BarItem *barItem, bars_)
    {
        ASSERT(barItem != nullptr);
        if (barItem->columnIcLabel_ != nullptr)
            barItem->columnIcLabel_->hide();
    }

    columnIcLabelsVisible_ = false;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private
/**
  * If the scaledHeight is too small, it is not worth visualizing.
  *
  * @param scaledHeight [double]
  * @returns bool
  */
bool LogoBarsItem::acceptScaledHeight(double scaledHeight) const
{
    return scaledHeight >= 1.;
}

/**
  * @param columns [const ClosedIntRange &]
  * @returns QVector<QGraphicsItemGroup *>
  */
QVector<LogoBarsItem::BarItem *> LogoBarsItem::createBars(const ClosedIntRange &columns)
{
    ASSERT(liveICDistribution_ != nullptr);

    ClosedIntRange actualColumns = columns;
    if (columns.isEmpty())
    {
        // Make the columns span the entire info content distribution range
        actualColumns.begin_ = 1;
        actualColumns.end_ = liveICDistribution_->length();
    }

    ASSERT(actualColumns.begin_ <= actualColumns.end_);
    ASSERT(actualColumns.begin_ > 0 && actualColumns.end_ <= liveICDistribution_->length());

    QVector<BarItem *> newBars(actualColumns.length(), nullptr);
    for (int i=actualColumns.begin_, j=0; i<= actualColumns.end_; ++i, ++j)
    {
        BarItem *barItem = createBar(i);
        newBars[j] = barItem;

        // Now for the label
    }
    return newBars;
}

LogoBarsItem::BarItem *LogoBarsItem::createBar(const int column)
{
    BarItem *barItem = new BarItem(this);
    int zeroBasedColumn = column - 1;
    barItem->setPos(barPosition(zeroBasedColumn));     // i-1 to account for zero-based count
    barItem->setHandlesChildEvents(false);

    VectorInfoUnit infoData = liveICDistribution_->infoContent().at(zeroBasedColumn);
    qreal y = barHeight_;
    VectorInfoUnit::ConstIterator it = infoData.constBegin();   // move to zero-based count
    VectorInfoUnit::ConstIterator end = infoData.constEnd();    // move to zero-based count
    for (; it != end; ++it)
    {
        const InfoUnit &infoUnit = *it;
        double letterHeight = scaledHeight(infoUnit.info_);
        y -= letterHeight;
        if (!acceptScaledHeight(letterHeight))
            continue;

        InfoUnitItem *infoUnitItem = new InfoUnitItem(infoUnit, pathForCharacter(infoUnit.ch_), barItem);
        infoUnitItem->setBrush(colorScheme_.textColorStyle(it->ch_).foreground_);
        infoUnitItem->setPen(Qt::NoPen);

        updateLetterPosition(infoUnitItem, y, letterHeight);
    }

    barItem->columnIcLabel_ = createColumnIcLabel(y, column, barItem);
    return barItem;
}

QString LogoBarsItem::columnInfoContentAsString(const double infoContent) const
{
    return QString::number(infoContent, 'f', 2);
}

QGraphicsSimpleTextItem *LogoBarsItem::createColumnIcLabel(const double yOfTopCharacter, const int column, QGraphicsItem *parentItem)
{
    double columnInfo = liveICDistribution_->columnInfo(column);
    if (columnInfo <= 0.)
        return nullptr;

    QGraphicsSimpleTextItem *icLabel = new QGraphicsSimpleTextItem(parentItem);
    icLabel->setFont(columnIcFont_);
    QString infoContentString = columnInfoContentAsString(columnInfo);
    icLabel->setText(infoContentString);
    QFontMetrics fontMetrics(columnIcFont_);
    double labelY = yOfTopCharacter - barLabelSpacing_ - fontMetrics.height();
    icLabel->setY(labelY);
    // Center label horizontally
    double labelX = (barWidth_ - fontMetrics.width(infoContentString)) / 2.;
    icLabel->setX(labelX);

    icLabel->setVisible(columnIcLabelsVisible_);

    return icLabel;
}

/**
  * @param ch [char]
  * @returns QPainterPath
  */
QPainterPath LogoBarsItem::pathForCharacter(char ch) const
{
    QPainterPath textPath;
    textPath.addText(metrics_->layoutInkOnlyOrigin(ch), font_, QString(ch));
    return textPath;
}

/**
  * @param information [double]
  * @returns double
  */
double LogoBarsItem::scaledHeight(double information) const
{
    return information / liveICDistribution_->maxInfo() * barHeight_;
}

/**
  */
void LogoBarsItem::updateBarPositions()
{
    for (int i=0, z=bars_.size(); i<z; ++i)
        bars_[i]->setPos(barPosition(i));
}

/**
  * columns is 1-based
  *
  * @param columns [const ClosedIntRange &]
  */
void LogoBarsItem::updateBarPositions(const ClosedIntRange &columns)
{
    for (int i=columns.begin_-1, z=columns.end_-1; i<=z; ++i)
        bars_[i]->setPos(barPosition(i));
}

/**
  * @param infoUnitItem [InfoUnitItem *]
  * @param y [double]
  * @param scaledHeight [double]
  */
void LogoBarsItem::updateLetterPosition(InfoUnitItem *infoUnitItem, double y, double scaledHeight) const
{
    ASSERT(infoUnitItem != nullptr);

    // Now position the letter
    double unscaledHeight = metrics_->inkHeight(infoUnitItem->infoUnit_.ch_);
    double unscaledWidth = metrics_->inkWidth(infoUnitItem->infoUnit_.ch_);

    double x = 0.;
    double xscale = 1.;
    if (unscaledWidth > barWidth_)
        xscale = barWidth_ / unscaledWidth;
    else
        x = (barWidth_ - unscaledWidth) / 2.;

    infoUnitItem->setTransform(QTransform::fromScale(xscale, scaledHeight / unscaledHeight));
    infoUnitItem->setPos(x, y);
}

/**
  */
void LogoBarsItem::updateLetterPositions()
{
    if (bars_.isEmpty())
        return;

    for (int i=0, z=bars_.size(); i<z; ++i)
    {
        const QGraphicsItemGroup *group = bars_.at(i);
        const QList<QGraphicsItem *> infoUnitItems = group->childItems();

        qreal y = barHeight_;
        for (int j=0, end=infoUnitItems.size(); j<end; ++j)
        {
            ASSERT(dynamic_cast<InfoUnitItem *>(infoUnitItems.at(j)) != 0);
            InfoUnitItem *item = static_cast<InfoUnitItem *>(infoUnitItems.at(j));

            // item->infoUnit_->info_ / liveICDistribution_->maxInfo() * barHeight_
            double letterHeight = scaledHeight(item->infoUnit_.info_);
            y -= letterHeight;
            if (!acceptScaledHeight(letterHeight))
                continue;

            updateLetterPosition(item, y, letterHeight);
        }
    }
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param columns [const ClosedIntRange &]
  */
void LogoBarsItem::onSourceColumnsInserted(const ClosedIntRange &columns)
{
    ASSERT(liveICDistribution_ != nullptr);
    ASSERT(columns.begin_ <= columns.end_);
    ASSERT(columns.begin_ > 0 && columns.end_ <= liveICDistribution_->length());

    prepareGeometryChange();

    // Create a place for the bars in memory that matches its visual location
    bars_.insert(columns.begin_ - 1, columns.length(), nullptr);

    // Update the x positions for all the bars that occur after the insertion
    updateBarPositions(ClosedIntRange(columns.end_ + 1, liveICDistribution_->length()));

    // Create and copy into their new destination
    QVector<BarItem *> newBars = createBars(columns);
    BarItem **dest = bars_.data() + columns.begin_ - 1;
    memcpy(dest, newBars.constData(), columns.length() * sizeof(BarItem *));

    emit barsAdded(columns);
}

/**
  * @param columns [const ClosedIntRange &]
  */
void LogoBarsItem::onSourceColumnsRemoved(const ClosedIntRange &columns)
{
    ASSERT(columns.begin_ <= columns.end_);
    ASSERT(columns.begin_ - 1 >= 0 && columns.end_ <= bars_.size());

    prepareGeometryChange();

    for (int i=columns.begin_; i<= columns.end_; ++i)
        delete bars_.at(i-1);

    bars_.remove(columns.begin_ - 1, columns.length());

    // Update the x positions for all the bars that occur after the insertion
    if (liveICDistribution_->length() > 0)
        updateBarPositions(ClosedIntRange(columns.begin_, liveICDistribution_->length()));

    emit barsRemoved(columns);
}

/**
  * @param columns [const ClosedIntRange &]
  */
void LogoBarsItem::onSourceDataChanged(const ClosedIntRange &columns)
{
    ASSERT(liveICDistribution_ != nullptr);
    ASSERT(columns.begin_ <= columns.end_);
    ASSERT(columns.begin_ - 1 >= 0 && columns.end_ <= bars_.size());

    for (int i=columns.begin_-1; i<=columns.end_-1; ++i)
    {
        delete bars_.at(i);
        bars_[i] = nullptr;
    }

    QVector<BarItem *> newBars = createBars(columns);

    BarItem **dest = bars_.data() + columns.begin_ - 1;
    memcpy(dest, newBars.constData(), columns.length() * sizeof(BarItem *));
}

