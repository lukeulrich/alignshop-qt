/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GUI_MISC_H
#define GUI_MISC_H

#include <QtCore/QModelIndex>
#include <QtCore/QVector>

class QWheelEvent;
class QString;

//! Returns a integer vector of unique row numbers present in modelIndexList
QVector<int> reduceToUniqueRows(const QModelIndexList &modelIndexList);
//! Returns the integer point size in the font family, fontFamily, that most closely approximates nPixels given the vertical dots per inch, dpiY
int fitPointSizeFromPixels(const QString &fontFamily, const int nPixels, const int dpiY);
//! Estimates the closest point size for font that fits horizontally within nPixels
int estimatePointSizeFromPixelWidth(const QFont &font, const double nPixels);
int estimateLargestPointSizeThatFits(const QFont &font, const QString &string, const double nPixels);
//! Returns the number of steps in wheelEvent
int stepsInWheelEvent(const QWheelEvent *wheelEvent);

#endif // GUI_MISC_H
