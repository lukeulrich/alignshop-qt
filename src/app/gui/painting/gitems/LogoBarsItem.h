/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LOGOBARSITEM_H
#define LOGOBARSITEM_H

#include <QtCore/QObject>
#include <QtCore/QRectF>

#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>
#include <QtGui/QPainterPath>

#include "../../../core/util/ClosedIntRange.h"
#include "../../../graphics/CharColorScheme.h"
#include "../../../core/global.h"

class QGraphicsItem;
class QGraphicsSimpleTextItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class CharPixelMetricsF;
class ClosedIntRange;
class InfoUnitItem;
class LiveInfoContentDistribution;

class LogoBarsItem : public QObject,
                     public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    LogoBarsItem(QGraphicsItem *parentItem = nullptr);
    LogoBarsItem(LiveInfoContentDistribution *liveICDistribution, QGraphicsItem *parentItem = nullptr);
    ~LogoBarsItem();

    bool areColumnIcLabelsVisible() const;
    double barHeight() const;
    QPointF barPosition(int column) const;
    double barWidth() const;
    virtual QRectF boundingRect() const;
    QFont font() const;
    double interBarSpacing() const;
    LiveInfoContentDistribution *liveInfoContentDistribution() const;
    int nBars() const;
    virtual void paint(QPainter * /* painter */, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */) {}
    void setBarHeight(double barHeight);
    void setBarWidth(double barWidth);
    void setColorScheme(const CharColorScheme &colorScheme);
    void setFont(const QFont &font);
    void setInterBarSpacing(double interBarSpacing);
    void setLiveInfoContentDistribution(LiveInfoContentDistribution *liveICDistribution);
    qreal width() const;


public Q_SLOTS:
    void setColumnIcLabelsVisible(bool visible = true);
    void showColumnIcLabels();
    void hideColumnIcLabels();


Q_SIGNALS:
    void barsReset();
    void barsAdded(const ClosedIntRange &columns);
    void barsRemoved(const ClosedIntRange &columns);


private Q_SLOTS:
    void onSourceColumnsInserted(const ClosedIntRange &columns);
    void onSourceColumnsRemoved(const ClosedIntRange &columns);
    void onSourceDataChanged(const ClosedIntRange &columns);


private:
    class BarItem : public QGraphicsItemGroup
    {
    public:
        BarItem(QGraphicsItem *parent = nullptr)
            : QGraphicsItemGroup(parent),
              columnIcLabel_(nullptr)
        {
        }

        ~BarItem()
        {
            delete columnIcLabel_;
        }

        QGraphicsSimpleTextItem *columnIcLabel_;
    };

    bool acceptScaledHeight(double scaledHeight) const;
    QVector<BarItem *> createBars(const ClosedIntRange &columns = ClosedIntRange());
    BarItem *createBar(const int column);
    QString columnInfoContentAsString(const double infoContent) const;
    QGraphicsSimpleTextItem *createColumnIcLabel(const double yOfTopCharacter, const int column, QGraphicsItem *parentItem);
    QPainterPath pathForCharacter(char ch) const;
    double scaledHeight(double information) const;
    void updateBarPositions();
    void updateBarPositions(const ClosedIntRange &columns);
    void updateLetterPosition(InfoUnitItem *infoUnitItem, double y, double scaledHeight) const;
    void updateLetterPositions();

    LiveInfoContentDistribution *liveICDistribution_;
    QFont font_;
    CharPixelMetricsF *metrics_;

    double barHeight_;
    double barWidth_;
    double interBarSpacing_;
    double barLabelSpacing_;        // Vertical spacing between the topmost bottom of the logo letters and its corresponding column info sum label
    double minBarLabelHorzPadding_;    // Min horizontal space between column sides and label

    QVector<BarItem *> bars_;
    QFont columnIcFont_;
    bool columnIcLabelsVisible_;

    CharColorScheme colorScheme_;
};

#endif // LOGOBARSITEM_H
