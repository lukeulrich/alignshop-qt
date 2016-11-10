/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "../graphics_misc.h"

class Testgraphics_misc : public QObject
{
    Q_OBJECT

private slots:
    void boundingRect();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void Testgraphics_misc::boundingRect()
{
    QColor blue = QColor(0, 0, 255);
    QRgb blueRgb = blue.rgb();

    // ------------------------------------------------------------------------
    // Test: Invalid image
    QCOMPARE(::boundingRect(QImage()), Rect());

    // ------------------------------------------------------------------------
    // Test: Empty image
    QImage image(5, 5, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All red
    QPainter painter(&image);
    painter.fillRect(image.rect(), QColor(255, 0, 0));
    painter.end();
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All green
    painter.begin(&image);
    painter.fillRect(image.rect(), QColor(0, 255, 0));
    painter.end();
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All blue
    painter.begin(&image);
    painter.fillRect(image.rect(), blue);
    painter.end();
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: single pixel tests
    for (int i=0; i< image.height(); ++i)
    {
        for (int j=0; j< image.width(); ++j)
        {
            image.fill(0);
            image.setPixel(j, i, blueRgb);
            QCOMPARE(::boundingRect(image), Rect(j, i, 0, 0));
        }
    }

    // ------------------------------------------------------------------------
    // Test: topleft and bottom right diagonal
    image.fill(0);
    image.setPixel(0, 0, blueRgb);
    image.setPixel(4, 4, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: topright and bottom left
    image.fill(0);
    image.setPixel(0, 4, blueRgb);
    image.setPixel(4, 0, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: inside rectangle
    image.fill(0);
    image.setPixel(1, 3, blueRgb);
    image.setPixel(3, 1, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(1, 1, 2, 2));

    image.fill(0);
    image.setPixel(3, 1, blueRgb);
    image.setPixel(1, 3, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(1, 1, 2, 2));
}


QTEST_APPLESS_MAIN(Testgraphics_misc)
#include "Testgraphics_misc.moc"
