/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QApplication>
#include <QtCore/QString>

#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QPainter>

#include "AntiToggleLineItem.h"
#include "LogoBarsItem.h"
#include "LogoItem.h"
#include "../../MsaTools/IMsaTool.h"
#include "../../MsaTools/MsaToolTypes.h"
#include "../../MsaTools/GapMsaTool.h"
#include "../../widgets/AbstractMsaView.h"
#include "../../../core/util/PosiRect.h"
#include "../../../core/LiveInfoContentDistribution.h"
#include "../../../core/macros.h"


static const int kYTickLength = 5;  // Pixels - these are for the horizontal tick lines on the Y axis
static const int kYPadding = 4;     // Padding between y axis and start of logo bars
static const int kYTickLabelSpacing = 4;
static const int kYTickLabelAxisLabelSpacing = 5;

static const int kXTickLength = 5;  // Pixels - these are for the vertical tick lines on the X axis
static const int kXTickLabelSpacing = 0;    // Vertical space between bottom of x tick mark and top of its label

static const int kColumnsPerLabel = 5;
static const int kLogoLeftPadding = 6;        // Horizontal padding for the entire logo's left edge
static const int kLogoRightPadding = 10;      // Horizontal padding for the entire logo's right edge; we give it more
                                              // than the left edge to account for the head space of the X-axis label

static const int kYAxisLogoBarsSpacing = 1;   // Horiziontal space between the y axis and the start of the logo bars
static const int kSelectionBarHeight = kXTickLength;

// TODO: The kYAxisLogoBarsSpacing actually pushes the logo slightly out of sync with the tick marks. It should be
// accommodated for this amount but right now it is virtually unrecognizable.

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
LogoItem::LogoItem(QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem),
      logoBarsItem_(nullptr),
      logoTitle_(nullptr),
      xAxisLabel_(nullptr),
      yAxisLabel_(nullptr),
      xAxisLine_(nullptr),
      yAxisLine_(nullptr),
      msaHorizSelection_(nullptr),
      msaMouseBar_(nullptr),
      msaGapAnchorLine_(nullptr),
      msaGapCurrentLine_(nullptr),
      msaGapRect_(nullptr),
      msaView_(nullptr)
{
    logoBarsItem_ = new LogoBarsItem(nullptr, this);
    initialize();
}

/**
  * @param liveICDistribution [LiveInfoContentDistribution *]
  * @param parentItem [QGraphicsItem *]
  * @param parentObject [QObject *]
  */
LogoItem::LogoItem(LiveInfoContentDistribution *liveICDistribution, QGraphicsItem *parentItem)
    : QGraphicsItem(parentItem),
      logoBarsItem_(nullptr),
      logoTitle_(nullptr),
      xAxisLabel_(nullptr),
      yAxisLabel_(nullptr),
      xAxisLine_(nullptr),
      yAxisLine_(nullptr),
      msaHorizSelection_(nullptr),
      msaMouseBar_(nullptr),
      msaGapAnchorLine_(nullptr),
      msaGapCurrentLine_(nullptr),
      msaGapRect_(nullptr),
      msaView_(nullptr)
{
    logoBarsItem_ = new LogoBarsItem(liveICDistribution, this);
    initialize();
}

/**
  */
LogoItem::~LogoItem()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QRectF
  */
QRectF LogoItem::boundingRect() const
{
    QRectF rect = childrenBoundingRect();

    // Expand the horizontal borders to accommodate the horizontal padding
    rect.adjust(-kLogoLeftPadding, 0, kLogoRightPadding, 0);
    return rect;
}

/**
  * @returns LogoBarsItem *
  */
LogoBarsItem *LogoItem::logoBarsItem() const
{
    return logoBarsItem_;
}

/**
  * @param msaView [AbstractMsaView *]
  */
void LogoItem::setMsaView(AbstractMsaView *msaView)
{
    if (msaView_ != nullptr)
    {
        disconnect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), this, SLOT(onMsaMouseCursorMoved(QPoint)));
        disconnect(msaView_, SIGNAL(mouseCursorVisibleChanged(bool)), this, SLOT(onMsaMouseCursorVisibleChanged(bool)));
        disconnect(msaView_, SIGNAL(msaToolChanged()), this, SLOT(onMsaToolChanged()));
        disconnect(msaView_, SIGNAL(selectionChanged(PosiRect,PosiRect)), this, SLOT(onMsaViewSelectionChanged(PosiRect)));
    }

    msaView_ = msaView;

    if (msaView_ != nullptr)
    {
        connect(msaView_, SIGNAL(mouseCursorMoved(QPoint,QPoint)), SLOT(onMsaMouseCursorMoved(QPoint)));
        connect(msaView_, SIGNAL(mouseCursorVisibleChanged(bool)), SLOT(onMsaMouseCursorVisibleChanged(bool)));
        connect(msaView_, SIGNAL(msaToolChanged()), SLOT(onMsaToolChanged()));
        connect(msaView_, SIGNAL(selectionChanged(PosiRect,PosiRect)), SLOT(onMsaViewSelectionChanged(PosiRect)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param columns [const ClosedIntRange &]
  */
void LogoItem::onBarsAdded(const ClosedIntRange &columns)
{
    prepareGeometryChange();
    updateXAxisLineLength();
    updateYGridLinesLength();

    // columns pertains exactly to the location where columns were added. In this case, we only care how many were added
    // - not where they were. Thus, translate this amount and add new columns to the end
    int nBars = logoBarsItem_->nBars();
    createXAxisTicksLabels(ClosedIntRange(nBars - columns.length() + 1, nBars));
}

/**
  * @param columns [const ClosedIntRange &]
  */
void LogoItem::onBarsRemoved(const ClosedIntRange &columns)
{
    prepareGeometryChange();
    updateXAxisLineLength();
    updateYGridLinesLength();

    // Remove columns.length amount from the end
    int nBars = logoBarsItem_->nBars();
    ClosedIntRange rangeRemoved(nBars + 1, nBars + columns.length());
    for (int i=rangeRemoved.begin_; i<= rangeRemoved.end_; ++i)
    {
        AntiToggleLineItem *tick = xOutsideTicks_.last();
        if (scene() != nullptr)
            scene()->removeItem(tick);
        delete tick;
        tick = nullptr;
        xOutsideTicks_.pop_back();

        if (i % kColumnsPerLabel == 0)
        {
            QGraphicsSimpleTextItem *label = xAxisBarLabels_.last();
            if (scene() != nullptr)
                scene()->removeItem(label);
            delete label;
            label = nullptr;

            xAxisBarLabels_.pop_back();
        }
    }
}

/**
  */
void LogoItem::onBarsReset()
{
    prepareGeometryChange();

    destroyDynamicDecorations();
    createDynamicDecorations();

    // Create core decorations such as the axis labels *after* the dynamic decorations so that it is possible to
    // position these using their information.
    createCoreDecorations();
}

/**
  * @param msaPoint [const Point &]
  */
void LogoItem::onMsaMouseCursorMoved(const QPoint &msaPoint)
{
    if (msaPoint.x() < 1)
        return;

    // Update the mouse bar location
    if (msaView_->currentMsaTool()->type() == Ag::kSelectMsaTool)
    {
        QRectF rect = msaMouseBar_->rect();
        rect.setX(logoBarsItem_->barPosition(msaPoint.x() - 1).x());
        rect.setRight(logoBarsItem_->barPosition(msaPoint.x()).x());
        msaMouseBar_->setRect(rect);
    }
}

/**
  * Always show the mouse bar if the mouse cursor is visible including when the mouse cursor is over the selection.
  *
  * @param visible [bool]
  */
void LogoItem::onMsaMouseCursorVisibleChanged(bool visible)
{
    msaMouseBar_->setVisible(visible || msaView_->currentMsaTool()->type() == Ag::kSelectMsaTool);
}

/**
  */
void LogoItem::onMsaToolChanged()
{
    IMsaTool *msaTool = msaView_->currentMsaTool();
    msaMouseBar_->setVisible(msaTool->type() == Ag::kSelectMsaTool);

    if (msaTool->type() == Ag::kGapMsaTool)
    {
        msaGapAnchorLine_->setVisible(true);

        GapMsaTool *gapTool = static_cast<GapMsaTool *>(msaTool);
        connect(gapTool, SIGNAL(gapColumnsInsertStarted(ClosedIntRange,bool)), SLOT(onGapToolColumnsInsertStarted(ClosedIntRange,bool)), Qt::UniqueConnection);
        connect(gapTool, SIGNAL(gapColumnsIntermediate(ClosedIntRange,bool)), SLOT(onGapToolColumnsIntermediate(ClosedIntRange,bool)), Qt::UniqueConnection);
        connect(gapTool, SIGNAL(gapColumnsInsertFinished(ClosedIntRange,bool)), SLOT(onGapToolColumnsInsertFinished(ClosedIntRange,bool)), Qt::UniqueConnection);
        connect(gapTool, SIGNAL(gapInsertionColumnChanged(int)), SLOT(onGapToolInsertionColumnChanged(int)), Qt::UniqueConnection);
    }
    else
    {
        msaGapAnchorLine_->setVisible(false);
    }
}

/**
  * Update the horizontal bar delineating the columns that have been selected in the msa view.
  *
  * @param selection [const PosiRect &]
  */
void LogoItem::onMsaViewSelectionChanged(const PosiRect &selection)
{
    if (selection.isNull() ||
        selection.normalized().horizontalRange().isEmpty())
    {
        msaHorizSelection_->setVisible(false);
        return;
    }

    ClosedIntRange columns = selection.normalized().horizontalRange();

    msaHorizSelection_->setVisible(true);
    QRectF rect = msaHorizSelection_->rect();
    rect.setX(logoBarsItem_->barPosition(columns.begin_ - 1).x());
    rect.setRight(logoBarsItem_->barPosition(columns.end_).x());
    msaHorizSelection_->setRect(rect);
}

/**
  * @param columns [const ClosedIntRange &columns]
  * @param normal [bool]
  */
void LogoItem::onGapToolColumnsInsertStarted(const ClosedIntRange &columns, bool normal)
{
    Q_UNUSED(normal);

    msaGapCurrentLine_->setVisible(true);
    msaGapRect_->setVisible(true);

    msaGapAnchorLine_->setX(logoBarsItem_->barPosition(columns.begin_ - 1).x());
    msaGapCurrentLine_->setX(logoBarsItem_->barPosition(columns.begin_ - 1).x());

    QRectF rect = msaGapRect_->rect();
    rect.setLeft(logoBarsItem_->barPosition(columns.begin_ - 1).x());
    rect.setWidth(0);
    msaGapRect_->setRect(rect);
}

/**
  * @param columns [const ClosedIntRange &columns]
  * @param normal [bool]
  */
void LogoItem::onGapToolColumnsIntermediate(const ClosedIntRange &columns, bool normal)
{
    if (normal)
    {
        msaGapAnchorLine_->setX(logoBarsItem_->barPosition(columns.begin_ - 1).x());
        int currentColumn = (!columns.isEmpty()) ? columns.end_:
                                                   columns.begin_ - 1;
        msaGapCurrentLine_->setX(logoBarsItem_->barPosition(currentColumn).x());
    }
    else
    {
        int anchorColumn = (!columns.isEmpty()) ? columns.end_:
                                                  columns.begin_ - 1;
        msaGapAnchorLine_->setX(logoBarsItem_->barPosition(anchorColumn).x());
        msaGapCurrentLine_->setX(logoBarsItem_->barPosition(columns.begin_ - 1).x());
    }

    QRectF rect = msaGapRect_->rect();
    if (!columns.isEmpty())
        rect.setRight(logoBarsItem_->barPosition(columns.end_).x());
    else
        rect.setWidth(0);
    msaGapRect_->setRect(rect);
}

/**
  * @param columns [const ClosedIntRange &columns]
  * @param normal [bool]
  */
void LogoItem::onGapToolColumnsInsertFinished(const ClosedIntRange &columns, bool normal)
{
    Q_UNUSED(columns);
    Q_UNUSED(normal);

    msaGapCurrentLine_->setVisible(false);
    msaGapRect_->setVisible(false);
}

/**
  * @param column [int]
  */
void LogoItem::onGapToolInsertionColumnChanged(int column)
{
    msaGapAnchorLine_->setX(logoBarsItem_->barPosition(column - 1).x());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Should only be called from constructors!
  */
void LogoItem::initialize()
{
    logoBarsItem_->setX(kYAxisLogoBarsSpacing);

    logoBarsItem_->setZValue(1);
    font_.setPointSize(font_.pointSize() - 1);
    axisLabelFont_.setBold(true);
    attachLogoBarSignals();

    msaHorizSelection_ = new QGraphicsRectItem(this);
    msaHorizSelection_->setVisible(false);
    msaHorizSelection_->setRect(0, logoBarsItem_->barHeight() + 1, 0, kSelectionBarHeight);
    msaHorizSelection_->setBrush(QApplication::palette().color(QPalette::Highlight));
    msaHorizSelection_->setPen(Qt::NoPen);

    msaMouseBar_ = new QGraphicsRectItem(this);
    msaMouseBar_->setRect(0, logoBarsItem_->barHeight() + 1, logoBarsItem_->barWidth(), kSelectionBarHeight);
    msaMouseBar_->setBrush(Qt::black);
    msaMouseBar_->setPen(Qt::NoPen);

    // Gap anchor lines
    msaGapAnchorLine_ = new AntiToggleLineItem(this);
    msaGapAnchorLine_->setPen(QColor(96, 96, 96));
    msaGapAnchorLine_->setVisible(false);
    msaGapAnchorLine_->setLine(0, 0, 0, logoBarsItem_->barHeight());
    msaGapAnchorLine_->setZValue(1);

    msaGapCurrentLine_ = new AntiToggleLineItem(this);
    msaGapCurrentLine_->setPen(QColor(0, 0, 0));
    msaGapCurrentLine_->setVisible(false);
    msaGapCurrentLine_->setLine(0, 0, 0, logoBarsItem_->barHeight());
    msaGapCurrentLine_->setZValue(1);

    msaGapRect_ = new QGraphicsRectItem(this);
    msaGapRect_->setPen(Qt::NoPen);
    msaGapRect_->setBrush(QColor(0, 0, 0, 96));
    msaGapRect_->setVisible(false);
    msaGapRect_->setRect(0, 0, 0, logoBarsItem_->barHeight());
    msaGapRect_->setZValue(1);
}

/**
  */
void LogoItem::attachLogoBarSignals()
{
    connect(logoBarsItem_, SIGNAL(barsAdded(ClosedIntRange)), SLOT(onBarsAdded(ClosedIntRange)));
    connect(logoBarsItem_, SIGNAL(barsRemoved(ClosedIntRange)), SLOT(onBarsRemoved(ClosedIntRange)));
    connect(logoBarsItem_, SIGNAL(barsReset()), SLOT(onBarsReset()));
}

/**
  */
void LogoItem::createCoreDecorations()
{
    delete yAxisLabel_;
    yAxisLabel_ = nullptr;

    // Y-axis label
    QString yAxisLabelText = "Information (bits)";
    QFontMetrics fontMetrics(axisLabelFont_);

    yAxisLabel_ = new QGraphicsSimpleTextItem(yAxisLabelText, this);
    yAxisLabel_->setFont(axisLabelFont_);
    yAxisLabel_->setRotation(-90);
    yAxisLabel_->setPos(0 - kYTickLength - kYTickLabelSpacing - maxYTickLabelWidth() - kYTickLabelAxisLabelSpacing - fontMetrics.height(),
                        logoBarsItem_->barHeight() - (logoBarsItem_->barHeight() - fontMetrics.width(yAxisLabelText)) / 2.);
}

/**
  */
void LogoItem::createDynamicDecorations()
{
    int nBars = logoBarsItem_->nBars();
    if (nBars == 0)
        return;

    ASSERT(logoBarsItem_->liveInfoContentDistribution() != nullptr);
    double maxInfo = logoBarsItem_->liveInfoContentDistribution()->maxInfo();
    double barHeight = logoBarsItem_->barHeight();
    QFontMetrics fontMetrics(font_);

    xAxisLine_ = new AntiToggleLineItem(0, barHeight, logoBarsItem_->width(), barHeight, this);
    yAxisLine_ = new AntiToggleLineItem(0, 0, 0, barHeight, this);

    // Y axis ticks, labels, and grid lines
    for (int i=0; i<= maxInfo; ++i)
    {
        qreal y = barHeight - (static_cast<qreal>(i) / maxInfo) * barHeight;

        // Grid line - all but the bottom most line at the position
        // Draw grid lines before tick marks, so that tick mark is visually above the grid line marks
        if (i > 0)
        {
            yGridLines_ << new AntiToggleLineItem(0, y, logoBarsItem_->width(), y, this);
            yGridLines_.last()->setPen(QColor::fromHsl(0, 0, 240));
        }

        // Tick mark
        yOutsideTicks_ << new AntiToggleLineItem(-kYTickLength, y, 0, y, this);

        // Tick label
        QString label = QString::number(i);
        yAxisTickLabels_ << new QGraphicsSimpleTextItem(label, this);
        yAxisTickLabels_.last()->setFont(font_);
        yAxisTickLabels_.last()->setPos(0 - kYTickLength - kYTickLabelSpacing - fontMetrics.width(label),
                                        // Note the use of ascent instead of height. Technically, height is the proper
                                        // way to center align all characters; however, because height() includes the
                                        // descent and numbers do not typically have any descent, the end result is
                                        // a slightly off-center result (actually, the number is a tad too high). Thus,
                                        // to compensate, we use ascent which more accurately positions numbers.
                                        y - (fontMetrics.ascent() / 2.));
    }

    createXAxisTicksLabels(ClosedIntRange(1, nBars));
}

/**
  * @param columns [const ClosedIntRange &]
  */
void LogoItem::createXAxisTicksLabels(const ClosedIntRange &columns)
{
    QFontMetrics fontMetrics(font_);
    double barWidth = logoBarsItem_->barWidth();
    double barHeight = logoBarsItem_->barHeight();

    if (columns.begin_ == 1)
    {
        xAxisBarLabels_ << new QGraphicsSimpleTextItem("1", this);
        xAxisBarLabels_.last()->setFont(font_);
        xAxisBarLabels_.at(0)->setPos(logoBarsItem_->barPosition(0).x() + (barWidth - fontMetrics.width("1")) / 2.,
                                      barHeight + kXTickLength + kXTickLabelSpacing);

        xOutsideTicks_ << new AntiToggleLineItem(0, barHeight, 0, barHeight + kXTickLength, this);
    }

    for (int i=columns.begin_; i<= columns.end_; ++i)
    {
        QPointF barPos = logoBarsItem_->barPosition(i);
        double x = barPos.x();

        // Tick mark
        xOutsideTicks_ << new AntiToggleLineItem(x, barHeight, x, barHeight + kXTickLength, this);

        // Tick label - center inside the bar width
        if (i % kColumnsPerLabel == 0)
        {
            QString label = QString::number(i);
            xAxisBarLabels_ << new QGraphicsSimpleTextItem(label, this);
            xAxisBarLabels_.last()->setFont(font_);
            xAxisBarLabels_.last()->setPos(x - barWidth + (barWidth - fontMetrics.width(label)) / 2.,
                                           barHeight + kXTickLength + kXTickLabelSpacing);
        }
    }
}

/**
  */
void LogoItem::destroyDynamicDecorations()
{
    QGraphicsScene *scene = this->scene();
    if (scene != nullptr)
    {
        if (xAxisLine_ != nullptr)
            scene->removeItem(xAxisLine_);
        if (yAxisLine_ != nullptr)
            scene->removeItem(yAxisLine_);

        foreach (QGraphicsSimpleTextItem *item, xAxisBarLabels_)
            scene->removeItem(item);

        foreach (AntiToggleLineItem *item, xOutsideTicks_)
            scene->removeItem(item);

        foreach (QGraphicsSimpleTextItem *item, yAxisTickLabels_)
            scene->removeItem(item);

        foreach (AntiToggleLineItem *item, yGridLines_)
            scene->removeItem(item);

        foreach (AntiToggleLineItem *item, yOutsideTicks_)
            scene->removeItem(item);
    }

    delete xAxisLine_;
    xAxisLine_ = nullptr;

    delete yAxisLine_;
    yAxisLine_ = nullptr;

    qDeleteAll(xAxisBarLabels_);
    xAxisBarLabels_.clear();

    qDeleteAll(xOutsideTicks_);
    xOutsideTicks_.clear();

    qDeleteAll(yAxisTickLabels_);
    yAxisTickLabels_.clear();

    qDeleteAll(yGridLines_);
    yGridLines_.clear();

    qDeleteAll(yOutsideTicks_);
    yOutsideTicks_.clear();
}

/**
  * @returns double
  */
double LogoItem::maxYTickLabelWidth() const
{
    double maxWidth = 0.;
    foreach (QGraphicsSimpleTextItem *item, yAxisTickLabels_)
    {
        if (item->boundingRect().width() > maxWidth)
            maxWidth = item->boundingRect().width();
    }

    return maxWidth;
}

/**
  */
void LogoItem::updateXAxisLineLength()
{
    QLineF line = xAxisLine_->line();
    line.setLength(logoBarsItem_->width());
    xAxisLine_->setLine(line);
}

/**
  */
void LogoItem::updateYGridLinesLength()
{
    double newWidth = logoBarsItem_->width();
    foreach (AntiToggleLineItem *lineItem, yGridLines_)
    {
        QLineF line = lineItem->line();
        line.setLength(newWidth);
        lineItem->setLine(line);
    }
}

