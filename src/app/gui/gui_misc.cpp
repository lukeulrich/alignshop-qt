/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QSet>

#include <QtGui/QFontMetrics>
#include <QtGui/QPaintDevice>
#include <QtGui/QWheelEvent>

#include "gui_misc.h"
#include "../core/global.h"
#include "../core/macros.h"

static const int kInchesPerPoint = 72;

/**
  * @param modelIndexList [const QModelIndexList &]
  * @returns QVector<int>
  */
QVector<int> reduceToUniqueRows(const QModelIndexList &modelIndexList)
{
    QSet<int> observedRows_;

    foreach (const QModelIndex &index, modelIndexList)
        observedRows_ << index.row();

    return observedRows_.toList().toVector();
}

/**
  * Given that a point is equal to 1/72 of an inch and logicalDpiY returns the number of pixels per inch (vertically)
  * for the current device, we can estimate the point size from pixels as follows:
  *
  * X pixels * ( 1 point / (1/72 inch) ) * ( 1 inch / logicalDpiY() pixels) = pointSize
  *
  * After deterimining a reasonable estimate for the pointsize, we subtract 4 from this size and then incrementally
  * increase this pointsize by 1 until it exceeds nPixels.
  *
  * @param fontFamily [const QString &]
  * @param nPixels [const int]
  * @param dpiY [const int]
  * @returns int
  */
int fitPointSizeFromPixels(const QString &fontFamily, const int nPixels, const int dpiY)
{
    ASSERT(nPixels > 0);

    int pointSize = qMax(1., nPixels * static_cast<double>(kInchesPerPoint) / static_cast<double>(dpiY) - 2);
    forever
    {
        QFontMetrics fontMetrics(QFont(fontFamily, pointSize));
        if (fontMetrics.height() > nPixels)
            break;

        ++pointSize;
    }

    return pointSize;
}

/**
  * @param font [const QFont &]
  * @param nPixels [const int]
  * @returns int
  */
int estimatePointSizeFromPixelWidth(const QFont &font, const double nPixels)
{
    ASSERT(nPixels > 0.);
    QFont testFont = font;
    int pointSize = 0;
    forever
    {
        testFont.setPointSize(pointSize + 1);
#ifdef Q_OS_MAC
        int width = QFontMetrics(testFont).width('M');
#else
        int width = QFontMetrics(testFont).maxWidth();
#endif
        if (width >= nPixels)
            break;

        ++pointSize;
    }

    return pointSize;
}

int estimateLargestPointSizeThatFits(const QFont &font, const QString &string, const double nPixels)
{
    ASSERT(nPixels > 0.);
    QFont testFont = font;
    int pointSize = 1;
    forever
    {
        testFont.setPointSize(pointSize);
#ifdef Q_OS_MAC
        int width = QFontMetrics(testFont).width(string);
#else
        int width = QFontMetrics(testFont).width(string);
#endif
        if (width >= nPixels)
            break;

        ++pointSize;
    }

    return pointSize;
}


/**
  * @param wheelEvent [const QWheelEvent *wheelEvent]
  * @returns int
  */
int stepsInWheelEvent(const QWheelEvent *wheelEvent)
{
    ASSERT(wheelEvent != nullptr);

    int numDegrees = wheelEvent->delta() / 8;
    return -numDegrees / 15;
}
