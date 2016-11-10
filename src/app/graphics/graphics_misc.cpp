/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QImage>

#include "graphics_misc.h"

/**
  * Specifically searches for non-zero blue values. Returns a null, uninitialized Rect if no blue pixel data is found.
  *
  * Optimized for 32-bit image data for finding the top and bottom boundaries.
  *
  * @param image [const QImage &]
  * @param threshold [int]
  * @returns Rect
  */
Rect boundingRect(const QImage &image, int threshold)
{
    if (image.width() == 0 || image.height() == 0)
        return Rect();

    int x_min = 0;
    int x_max = image.width()-1;
    int y_min = 0;
    int y_max = image.height()-1;

    int h = image.height();
    int w = image.width();

    // ------------
    // Scan the top
    if (image.depth() == 32)
    {
        for (; y_min < h; ++y_min)
        {
            QRgb *p = (QRgb *)(image.scanLine(y_min));
            for (int x=0; x< w; ++x)
                if (qBlue(*p++) > threshold)
                    goto DONE_TOP;
        }
    }
    else    // Image is not 32 bpp
    {
        for (; y_min < h; ++y_min)
            for (int x=0; x< w; ++x)
                if (qBlue(image.pixel(x, y_min)) > threshold)
                    goto DONE_TOP;
    }
 DONE_TOP:

    // ---------------
    // Scan the bottom
    if (image.depth() == 32)
    {
        for (; y_max > y_min; --y_max)
        {
            QRgb *p = (QRgb *)(image.scanLine(y_max));
            for (int x=0; x< w; ++x)
                if (qBlue(*p++) > threshold)
                    goto DONE_BOTTOM;
        }
    }
    else
    {
        for (; y_max > y_min; --y_max)
            for (int x=0; x< w; ++x)
                if (qBlue(image.pixel(x, y_max)) > threshold)
                    goto DONE_BOTTOM;
    }
 DONE_BOTTOM:

    // -------------
    // Scan the left
    for (; x_min < w; ++x_min)
//         for (int y=0; y< h; ++y)
        // Optimize a bit by only scanning between the y_min and y_max regions
        for (int y=y_min; y<= y_max; ++y)
            if (qBlue(image.pixel(x_min, y)) > threshold)
                goto DONE_LEFT;
 DONE_LEFT:

    // --------------
    // Scan the right
    for (; x_max > x_min; --x_max)
//        for (int y=0; y< h; ++y)
         // Optimize a bit by only scanning between the y_min and y_max regions
         for (int y=y_min; y<= y_max; ++y)
            if (qBlue(image.pixel(x_max, y)) > threshold)
                goto DONE_RIGHT;
 DONE_RIGHT:

    if (x_min > x_max || y_min > y_max)
        return Rect();

    return Rect(x_min, y_min, x_max - x_min, y_max - y_min);
}

