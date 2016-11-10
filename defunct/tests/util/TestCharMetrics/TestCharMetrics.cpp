/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QTextLayout>

#include "util/CharMetrics.h"
#include "util/Rect.h"
#include "global.h"

class TestCharMetrics : public QObject
{
    Q_OBJECT

private slots:
    void inkWHS();              // Test ink width, height, and size
    void inkOrigin();
    void wHSOriginInkTopLeft();                 // Test width, height, size, and origin
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestCharMetrics::inkWHS()
{
    QFont font("monospace", 18);
    QFontMetrics fm(font);
    QColor blue(0, 0, 255);
    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);

    // ------------------------------------------------------------------------
    // Test: all characters at monospace size of 18
    {
        CharMetrics x(font);

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127)
            {
                QCOMPARE(x.inkWidth(ch), -1);
                QCOMPARE(x.inkHeight(ch), -1);
                QCOMPARE(x.inkSize(ch), QSize());
                continue;
            }

            // Determine size of character
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHints(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(5, fm.ascent() + 5, font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            QVERIFY(x.inkWidth(ch) == bb.width() ||
                    x.inkWidth(ch) == bb.width() + 1 ||
                    x.inkWidth(ch) == bb.width() + 2);
            QVERIFY(x.inkHeight(ch) == bb.height() || x.inkHeight(ch) == bb.height() + 1);
            QCOMPARE(x.inkSize(ch).width(), x.inkWidth(ch));
            QCOMPARE(x.inkSize(ch).height(), x.inkHeight(ch));
        }
    }

    // ------------------------------------------------------------------------
    // Test: Only those characters in our set
    {
        QString chars = "abcDEF";
        CharMetrics x(font, chars.toAscii());

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127 || chars.indexOf(QChar(ch)) == -1)
            {
                QCOMPARE(x.inkWidth(ch), -1);
                QCOMPARE(x.inkHeight(ch), -1);
                QCOMPARE(x.inkSize(ch), QSize());
                continue;
            }

            // Determine size of character
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHints(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(5, fm.ascent() + 5, font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            QVERIFY(x.inkWidth(ch) == bb.width() ||
                    x.inkWidth(ch) == bb.width() + 1 ||
                    x.inkWidth(ch) == bb.width() + 2);
            QVERIFY(x.inkHeight(ch) == bb.height() || x.inkHeight(ch) == bb.height() + 1);
            QCOMPARE(x.inkSize(ch).width(), x.inkWidth(ch));
            QCOMPARE(x.inkSize(ch).height(), x.inkHeight(ch));
        }
    }
}

void TestCharMetrics::inkOrigin()
{
    QFont font("monospace", 18);
    QColor blue(0, 0, 255);

    // ------------------------------------------------------------------------
    // Test: paint ink part only with a 1 pixel artificial border. If the inkBoundingRect does not
    //       exactly reflect this 1-pixel border, something isn't working right
    {
        CharMetrics x(font);

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127)
            {
                QCOMPARE(x.inkOnlyOrigin(ch), QPoint());
                continue;
            }

            // Add 2 to each dimension for the 1 pixel border
            QImage image(x.inkWidth(ch)+2,
                         x.inkHeight(ch)+2,
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHints(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(1 + x.inkOnlyOrigin(ch).x(),
                         1 + x.inkOnlyOrigin(ch).y(),
                         font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            QCOMPARE(bb.x(), 1);
            QCOMPARE(bb.y(), 1);
            QVERIFY(bb.right() == image.width()-2 || bb.right() == image.width()-3);
            QVERIFY(bb.bottom() == image.height()-2 || bb.bottom() == image.height()-3);
        }
    }

    // ------------------------------------------------------------------------
    // Test: this time only with characters we define
    {
        QString validChars = "abc!@#$";
        CharMetrics x(font, validChars.toAscii());

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127 || validChars.indexOf(QChar(ch)) == -1)
            {
                QCOMPARE(x.inkOnlyOrigin(ch), QPoint());
                continue;
            }

            // Add 2 to each dimension for the 1 pixel border
            QImage image(x.inkWidth(ch)+2,
                         x.inkHeight(ch)+2,
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHints(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(1 + x.inkOnlyOrigin(ch).x(),
                         1 + x.inkOnlyOrigin(ch).y(),
                         font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            QCOMPARE(bb.x(), 1);
            QCOMPARE(bb.y(), 1);
            QVERIFY(bb.right() == image.width()-2 || bb.right() == image.width()-3);
            QVERIFY(bb.bottom() == image.height()-2 || bb.bottom() == image.height()-3);
        }
    }
}

void TestCharMetrics::wHSOriginInkTopLeft()
{
    QColor blue(0, 0, 255);
    QFont font("monospace", 18);
    QFontMetrics fm(font);

    int maxWidth = 0;
    int maxAscent = 0;
    int maxDescent = 0;

    // ------------------------------------------------------------------------
    // Test: all characters, width, height, and size
    {
        CharMetrics x(font);
        for (int i=33; i< 128; ++i)
        {
            char ch = (char)i;

            if (x.inkWidth(ch) > maxWidth)
                maxWidth = x.inkWidth(ch);

            // To check the height, we must add both the ascent and descents! ugh.
            QImage image(fm.maxWidth() + 10,
                         fm.height() + 10,
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(5, fm.ascent() + 5, font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            int ascent = fm.ascent() + 5 - bb.top();
            if (ascent > maxAscent)
                maxAscent = ascent;
            int descent = bb.bottom() - (fm.ascent() + 5);
            if (descent > maxDescent)
                maxDescent = descent;
        }

        QCOMPARE(x.width(), maxWidth);
        QCOMPARE(x.height(), maxAscent + maxDescent);
        QCOMPARE(x.size(), QSize(maxWidth, maxAscent + maxDescent));
    }

    // ------------------------------------------------------------------------
    // Test: origin - using maxAscent from previous test
    {
        bool touchedTop = false;

        CharMetrics x(font);
        QImage image(x.size(), QImage::Format_ARGB32_Premultiplied);
//        QImage layoutImage(x.size(), QImage::Format_ARGB32_Premultiplied);
        for (int i=33; i< 127; ++i)
        {
            char ch = (char)i;

            // To check the height, we must add both the ascent and descents! ugh.
            image.fill(0);

            QPainter painter(&image);

            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addText(x.origin(ch), font, QChar(ch));

            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            if (!touchedTop && bb.top() == 0)
                touchedTop = true;

            QVERIFY(bb.top() >= 0);
            int rightAmount = image.width() - 1 - bb.right();
            QVERIFY(bb.left() == rightAmount ||
                    bb.left() + 1 == rightAmount ||
                    bb.left() - 1 == rightAmount);

            // Check the inkTopLeft
            int inkLeft = qRound(x.inkTopLeft(ch).x());
            int inkTop = qRound(x.inkTopLeft(ch).y());
            QVERIFY(bb.left() == inkLeft || bb.left() == inkLeft + 1 || bb.left() == inkLeft - 1);
            QVERIFY(bb.top() == inkTop || bb.top() == inkTop + 1 || bb.top() == inkTop - 1);

            // Draw using the layoutOrigin and check for an identical image
            /*
            QTextLayout layout(QChar(ch), font);
            layout.beginLayout();
            layout.createLine();
            layout.endLayout();

            layoutImage.fill(0);
            painter.begin(&layoutImage);
            painter.setPen(blue);
            layout.draw(&painter, x.layoutOrigin(ch));
            painter.end();

            QCOMPARE(layoutImage, image);
            */
        }
        QVERIFY(touchedTop);
    }
}

QTEST_MAIN(TestCharMetrics)
#include "TestCharMetrics.moc"
