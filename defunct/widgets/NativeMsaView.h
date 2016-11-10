/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef NATIVEMSAVIEW_H
#define NATIVEMSAVIEW_H

#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QStack>

#include "AbstractMsaView.h"

#include <QtSvg/QSvgGenerator>
#include <QtGui/QFontMetricsF>

// Should be moved to the cpp file!
#include "PointRectMapperPrivate.h"

/**
  * NativeMsaView encapsulates the software rendering (raster) implementation of a MsaView.
  */
class NativeMsaView : public AbstractMsaView
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    NativeMsaView(QWidget *parent = 0);                         //!< Trivial constructor

protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented protected methods
    virtual void drawBackground(QPainter *painter) const;                                           //!< Reimplemented from AbstractMsaView
    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter);         //!< Reimplemented from AbstractMsaView
    virtual void drawSelection(const QRectF &rect, QPainter *painter) const;                        //!< Reimplemented from AbstractMsaView
    virtual void drawEditCursor(const QRectF &rect, QPainter *painter) const;                       //!< Reimplemented from AbstractMsaView
    virtual void drawMouseActivePoint(const QRectF &rect, QPainter *painter) const;                 //!< Reimplemented from AbstractMsaView
    virtual void drawGapInsertionLine(qreal x, QPainter *painter) const;

    virtual AbstractTextRenderer *abstractTextRenderer() const
    {
        return abstractTextRenderer_;
    }

    qreal charWidth_;
    qreal charHeight_;
    MsaRect oldMsaRegionClip_;

private Q_SLOTS:
    void onAbstractMsaViewFontChanged();
    void onAbstractMsaViewZoomChanged(double zoom);

private:
    AbstractTextRenderer *abstractTextRenderer_;
};

/**
  * Uses a single pixmap sized to the current visible (including partially) msaRect. Downside - this does not work with
  * a non-integral zoom value.
  */
class SinglePixmapNativeMsaView : public NativeMsaView
{
    Q_OBJECT

public:
    SinglePixmapNativeMsaView(QWidget *parent = 0) : NativeMsaView(parent)
    {
        connect(this, SIGNAL(msaChanged()), SLOT(onMsaChanged()));
    }

    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter)
    {
        updateMsaRegionPixmap(msaRect);
        painter->drawPixmap(origin, msaRegionClipPixmap_);
    }

protected Q_SLOTS:
    virtual void onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight);
    virtual void onMsaGapColumnsInserted(int column, int count);
    virtual void onMsaGapColumnsRemoved(int count);
    void onMsaExtendOrTrimFinished(int start, int end);
    void onMsaCollapsedLeft(const MsaRect &msaRect, int rightMostModifiedColumn);
    void onMsaCollapsedRight(const MsaRect &msaRect, int leftMostModifiedColumn);

private Q_SLOTS:
    virtual void onMsaChanged()
    {
        msaRegionClipPixmap_ = QPixmap();
    }

private:
    void updateMsaRegionPixmap(const MsaRect &newMsaRegionClip);
    void repaintColumns(int start, int end);

    QPixmap msaRegionClipPixmap_;                                   //!< Cached rendering of the Msa region currently associated with clipRect_
};



/**
  * Uses one pixmap per sequence that begins at the topleft-most point of each sequence. THus as the user scrolls to the
  * right, the pixmap increases size. The left point is never adjusted
  */
class PixmapPerSeqNativeMsaView : public NativeMsaView
{
    Q_OBJECT

public:
    PixmapPerSeqNativeMsaView(QWidget *parent) : NativeMsaView(parent)
    {
    }

protected:
    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter)
    {
        QRectF sourceRect = pointRectMapper()->msaRectToCanvasRect(msaRect);
        sourceRect.setTop(0);
        sourceRect.setBottom(charHeight_);

        QPointF renderPoint(origin);
    //    renderPoint.setX(-(msaRect.left()-1)*charWidth_ + origin.x());
        for (int i=0; i< pixmaps_.count(); ++i)
        {
            painter->drawPixmap(renderPoint, pixmaps_[i], sourceRect);
            renderPoint.setY(renderPoint.y() + charHeight_);
        }
    }

    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);

private:
    QList<QPixmap> pixmaps_;
};




/**
  * Divides the visible msaRect into blocks of arbitrary sizes. Pre-allocates several QPixmaps - one for each block.
  * These are managed by a Stack.
  */
class BlockNativeMsaView : public NativeMsaView
{
    Q_OBJECT

public:
    BlockNativeMsaView(QWidget *parent = 0) : NativeMsaView(parent)
    {
        blockW_ = 300;
        blockH_ = 300;

        for (int i=0; i< 200; ++i)
            prepixes_ << QPixmap(blockW_, blockH_);

        top_ = 0;
        left_ = 0;
        right_ = 0;
        bottom_ = 0;
    }

    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter);
    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);

private:
    QList<QList<QPixmap> > pixmaps_;
    int top_;
    int bottom_;
    int left_;
    int right_;

    int blockW_;
    int blockH_;

    QStack<QPixmap> prepixes_;
};


/**
  * Utilizes the block based approach but stores all individual block pixmaps within a single master pixmap.
  */
class OnePixmapBlockNativeMsaView : public NativeMsaView
{
    Q_OBJECT

public:
    OnePixmapBlockNativeMsaView(QWidget *parent = 0) : NativeMsaView(parent)
    {
        blockW_ = 100;
        blockH_ = 80;

        nHorzBlocks_ = 20;
        nVertBlocks_ = 20;

        // 4000x2000
        psize_.setWidth(blockW_ * nHorzBlocks_);
        psize_.setHeight(blockH_ * nVertBlocks_);

        masterPixmap_ = QPixmap(psize_);
        masterPixmap_.fill();

        rects_.reserve(nHorzBlocks_ * nVertBlocks_);
        for (int i=0; i< nVertBlocks_; ++i)
            for (int j=0; j< nHorzBlocks_; ++j)
                rects_ << Rect(i * blockW_, j * blockH_, blockW_, blockH_);

        topAvailRect_ = rects_.count() - 1;

        top_ = 0;
        left_ = 0;
        right_ = 0;
        bottom_ = 0;
    }

    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter);
    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);

private:
    int blockW_;
    int blockH_;
    QSize psize_;
    int nHorzBlocks_;
    int nVertBlocks_;
    QPixmap masterPixmap_;

    QList<Rect> rects_;
    int topAvailRect_;

    int top_;
    int bottom_;
    int left_;
    int right_;

    QHash<int, QHash<int, Rect> > lookup_;
};

#endif // NATIVEMSAVIEW_H
