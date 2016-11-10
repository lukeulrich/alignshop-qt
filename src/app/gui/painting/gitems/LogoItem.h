/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LOGOITEM_H
#define LOGOITEM_H

#include <QtCore/QObject>
#include <QtCore/QRectF>
#include <QtCore/QVector>

#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>

#include "../../../core/util/ClosedIntRange.h"
#include "../../../core/global.h"

class QGraphicsItem;
class QGraphicsTextItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class AbstractMsaView;
class AntiToggleLineItem;
class LiveInfoContentDistribution;
class LogoBarsItem;
class PosiRect;

class LogoItem : public QObject,
                 public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit LogoItem(QGraphicsItem *parentItem = nullptr);
    LogoItem(LiveInfoContentDistribution *liveICDistribution, QGraphicsItem *parentItem = nullptr);
    ~LogoItem();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * /* painter */, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */) {}
    LogoBarsItem *logoBarsItem() const;
    void setMsaView(AbstractMsaView *msaView);

private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onBarsAdded(const ClosedIntRange &columns);
    void onBarsRemoved(const ClosedIntRange &columns);
    void onBarsReset();
    void onMsaMouseCursorMoved(const QPoint &msaPoint);
    void onMsaMouseCursorVisibleChanged(bool visible);
    void onMsaToolChanged();
    void onMsaViewSelectionChanged(const PosiRect &selection);

    void onGapToolColumnsInsertStarted(const ClosedIntRange &columns, bool normal);
    void onGapToolColumnsIntermediate(const ClosedIntRange &columns, bool normal);
    void onGapToolColumnsInsertFinished(const ClosedIntRange &columns, bool normal);
    void onGapToolInsertionColumnChanged(int column);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void initialize();               // Called only from the constructor to perform common constructor initialization

    void attachLogoBarSignals();            // Helper method for connecting to the logo bars signals in a single place
    void createCoreDecorations();
    void createDynamicDecorations();
    void createXAxisTicksLabels(const ClosedIntRange &columns);
    void destroyDynamicDecorations();
    double maxYTickLabelWidth() const;
    void updateXAxisLineLength();
    void updateYGridLinesLength();

    LogoBarsItem *logoBarsItem_;
    QFont font_;
    QFont axisLabelFont_;

    // Core decorations
    QGraphicsSimpleTextItem *logoTitle_;
    QGraphicsSimpleTextItem *xAxisLabel_;
    QGraphicsSimpleTextItem *yAxisLabel_;

    // Dynamic decorations
    QGraphicsLineItem *xAxisLine_;
    QGraphicsLineItem *yAxisLine_;
    QVector<QGraphicsSimpleTextItem *> xAxisBarLabels_;
    QVector<AntiToggleLineItem *> xOutsideTicks_;
    QVector<QGraphicsSimpleTextItem *> yAxisTickLabels_;
    QVector<AntiToggleLineItem *> yGridLines_;
    QVector<AntiToggleLineItem *> yOutsideTicks_;

    // Dynamic decoration that is never destroyed
    QGraphicsRectItem *msaHorizSelection_;
    QGraphicsRectItem *msaMouseBar_;        // Current position of the mouse

    // For showing the gap insertion area
    AntiToggleLineItem *msaGapAnchorLine_;
    AntiToggleLineItem *msaGapCurrentLine_;
    QGraphicsRectItem *msaGapRect_;

    // Msa view instance for watching the selection change
    AbstractMsaView *msaView_;
};



#endif // LOGOITEM_H
