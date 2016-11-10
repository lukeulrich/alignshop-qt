/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SINGLEPIXMAPMSAVIEW_H
#define SINGLEPIXMAPMSAVIEW_H

#include <QtGui/QPixmap>

#include "NativeMsaView.h"
#include "../../core/global.h"

/**
  * Maintains a single pixmap for rendering purposes.
  *
  * Because certain views depend upon information across all columns, in many cases it is necessary to redraw entire
  * columns regardless if they do not intersect the current cached msa rect.
  */
class SinglePixmapMsaView : public NativeMsaView
{
    Q_OBJECT

public:
    SinglePixmapMsaView(QWidget *parent = nullptr);

public Q_SLOTS:
    void clearCache();
    void repaintRow(int msaRow);

protected:
    virtual void drawMsa(const QPointF &origin, const PosiRect &msaRect, QPainter *painter);

protected Q_SLOTS:
    void onMsaGapColumnsInserted(const ClosedIntRange &columns);
    void onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &columnRanges);
    void onMsaRectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange);
    void onMsaCollapsedLeft(const PosiRect &msaRect);
    void onMsaCollapsedRight(const PosiRect &msaRect);
    void onMsaRowsInserted(const ClosedIntRange &rows);
    void onMsaRowsMoved(const ClosedIntRange &rows, int finalRow);
    void onMsaRowsRemoved(const ClosedIntRange &rows);
    void onMsaRowsSorted();
    void onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods);

private:
    void repaintColumns(const ClosedIntRange &columns);
    void updateMsaCachedPixmap(const PosiRect &msaRect);

    QPixmap cachedPixmap_;
    PosiRect cachedMsaRect_;
};

#endif // SINGLEPIXMAPMSAVIEW_H
