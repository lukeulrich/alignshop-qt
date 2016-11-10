/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GRAPHICS_MISC_H
#define GRAPHICS_MISC_H

#include "../core/util/Rect.h"

class QImage;

//! Returns the smallest bounding rectangle that fully contains all blue pixel data above threshold in image
Rect boundingRect(const QImage &image, int threshold = 10);

#endif // GRAPHICS_MISC_H
