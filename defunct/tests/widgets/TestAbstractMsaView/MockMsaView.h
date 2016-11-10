/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKMSAVIEW_H
#define MOCKMSAVIEW_H

#include <QtCore/QVariantList>

#include "AbstractMsaView.h"

struct DrawCall
{
    QString funcName_;
    QVariantList arguments_;

    DrawCall(const QString &funcName = QString()) : funcName_(funcName)
    {
    }
};

/**
  * MockMsaView is a dummy class for testing the AbstractMsaView core.
  *
  * All draw methods are stubbed out and recorded to the drawCalls_ public member which may be cleared and queried
  * for the testing purposes.
  */
class MockMsaView : public AbstractMsaView
{
public:
    MockMsaView(QWidget *parent = 0);
    ~MockMsaView()
    {
        delete abstractTextRenderer_;
    }

    mutable QList<DrawCall> drawCalls_;

protected:
    virtual void drawAll(QPainter *painter);
    virtual void drawBackground(QPainter *painter) const;
    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter);
    virtual void drawSelection(const QRectF &rect, QPainter *painter) const;
    virtual void drawEditCursor(const QRectF &rect, QPainter *painter) const;
    virtual void drawMouseActivePoint(const QRectF &rect, QPainter *painter) const;
    virtual void drawGapInsertionLine(qreal x, QPainter *painter) const;
    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);

    virtual AbstractTextRenderer *abstractTextRenderer() const;

    friend class TestAbstractMsaView;
    friend class TestPointRectMapperPrivate;

private:
    AbstractTextRenderer *abstractTextRenderer_;
};

#endif // MOCKMSAVIEW_H
