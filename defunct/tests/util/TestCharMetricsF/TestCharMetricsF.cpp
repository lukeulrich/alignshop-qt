/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include "util/CharMetricsF.h"
#include "util/CharMetrics.h"           // For inkBoundingRect
#include "util/Rect.h"

#include "global.h"

class TestCharMetricsF : public QObject
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
void TestCharMetricsF::inkWHS()
{
    QFont font("monospace", 18);

    // ------------------------------------------------------------------------
    // Test: all characters at monospace size of 18
    {
        CharMetricsF x(font);

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127)
            {
                QCOMPARE(x.inkWidth(ch), -1.);
                QCOMPARE(x.inkHeight(ch), -1.);
                QCOMPARE(x.inkSize(ch), QSizeF());
                continue;
            }

            QPainterPath path;
            path.addText(0, 0, font, QChar(ch));
            QRectF bb = path.boundingRect();

            QCOMPARE(x.inkWidth(ch), bb.width());
            QCOMPARE(x.inkHeight(ch), bb.height());
            QCOMPARE(x.inkSize(ch), bb.size());
        }
    }

    // ------------------------------------------------------------------------
    // Test: Only those characters in our set
    {
        QString chars = "abcDEF";
        CharMetricsF x(font, chars.toAscii());

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127 || chars.indexOf(QChar(ch)) == -1)
            {
                QCOMPARE(x.inkWidth(ch), -1.);
                QCOMPARE(x.inkHeight(ch), -1.);
                QCOMPARE(x.inkSize(ch), QSizeF());
                continue;
            }

            QPainterPath path;
            path.addText(0, 0, font, QChar(ch));
            QRectF bb = path.boundingRect();

            QCOMPARE(x.inkWidth(ch), bb.width());
            QCOMPARE(x.inkHeight(ch), bb.height());
            QCOMPARE(x.inkSize(ch), bb.size());
        }
    }
}

void TestCharMetricsF::inkOrigin()
{
    QFont font("monospace", 10);
    QColor blue(0, 0, 255);

    // ------------------------------------------------------------------------
    // Test: paint ink part only with a 1 pixel artificial border. If the ::boundingRect does not
    //       exactly reflect this 1-pixel border, something isn't working right
    {
        CharMetricsF x(font);

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127)
            {
                QCOMPARE(x.inkOnlyOrigin(ch), QPointF());
                continue;
            }

            // Add 2 to each dimension for the 1 pixel border
            QImage image(ceil(x.inkWidth(ch))+2,
                         ceil(x.inkHeight(ch))+2,
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            QPointF pf(1. + x.inkOnlyOrigin(ch).x(),
                       1. + x.inkOnlyOrigin(ch).y());

            QPainterPath path;
            path.addText(pf, font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = boundingRect(image);

//            image.save(QString("_%1.png").arg(ch));

//            qDebug() << image.size() << ch << pf << bb;

            // Allow a fudge factor of 1 pixel
            QVERIFY(bb.x() == 1 || bb.x() == 2);
            QVERIFY(bb.y() == 1 || bb.y() == 2);

            // Allow a fudge factor of 1 pixel
            QVERIFY(bb.right() == image.width() - 3 || bb.right() == image.width() - 2);
            QVERIFY(bb.bottom() == image.height() - 3 || bb.bottom() == image.height() - 2);
        }
    }

    // ------------------------------------------------------------------------
    // Test: this time only with characters we define
    {
        QString validChars = "abc!@#$";
        CharMetricsF x(font, validChars.toAscii());

        for (int i=-127; i< 128; ++i)
        {
            char ch = (char)i;
            if (i < 33 || i == 127 || validChars.indexOf(QChar(ch)) == -1)
            {
                QCOMPARE(x.inkOnlyOrigin(ch), QPointF());
                continue;
            }

            // Add 2 to each dimension for the 1 pixel border
            QImage image(ceil(x.inkWidth(ch))+2,
                         ceil(x.inkHeight(ch))+2,
                         QImage::Format_ARGB32_Premultiplied);
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);
            QPointF pf(1. + x.inkOnlyOrigin(ch).x(),
                       1. + x.inkOnlyOrigin(ch).y());

            QPainterPath path;
            path.addText(pf, font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = boundingRect(image);

//            qDebug() << image.size() << ch << pf << bb;

            // Allow a fudge factor of 1 pixel
            QVERIFY(bb.x() == 1 || bb.x() == 2);
            QVERIFY(bb.y() == 1 || bb.y() == 2);

            // Allow a fudge factor of 1 pixel
            QVERIFY(bb.right() == image.width() - 3 || bb.right() == image.width() - 2);
            QVERIFY(bb.bottom() == image.height() - 3 || bb.bottom() == image.height() - 2);
        }
    }
}

void TestCharMetricsF::wHSOriginInkTopLeft()
{
    QColor blue(0, 0, 255);
    QFont font("monospace", 18);
    QFontMetrics fm(font);

    qreal maxWidth = 0;
    qreal maxAscent = 0;
    qreal maxDescent = 0;

    // ------------------------------------------------------------------------
    // Test: all characters, width, height, and size
    {
        CharMetricsF x(font);
        for (int i=33; i< 128; ++i)
        {
            char ch = (char)i;

            if (x.inkWidth(ch) > maxWidth)
                maxWidth = x.inkWidth(ch);

            QPainterPath path;
            path.addText(0, 0, font, QChar(ch));

            QRectF bb = path.boundingRect();
            if (-bb.top() > maxAscent)
                maxAscent = -bb.top();
            if (bb.bottom() > maxDescent)
                maxDescent = bb.bottom();
        }

        QCOMPARE(x.width(), maxWidth);
        QCOMPARE(x.height(), maxAscent + maxDescent);
        QCOMPARE(x.size(), QSizeF(maxWidth, maxAscent + maxDescent));
    }

    // ------------------------------------------------------------------------
    // Test: origin - using maxAscent from previous test
    {
        bool touchedTop = false;

        CharMetricsF x(font);
        QImage image(QSize(ceil(x.size().width()), ceil(x.size().height())), QImage::Format_ARGB32_Premultiplied);
        for (int i=33; i< 127; ++i)
        {
            char ch = (char)i;

            // To check the height, we must add both the ascent and descents! ugh.
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);

            QPainterPath path;
            path.addText(x.origin(ch), font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            if (!touchedTop && bb.top() == 0)
                touchedTop = true;

            QVERIFY(bb.top() >= 0);
            // Check that the left and right amounts are within 1 pixel of each other
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
            // Layouts use the drawText method for rendering text and will create a different result
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

    // ------------------------------------------------------------------------
    // Test: specific set of characters
    maxWidth = 0;
    maxAscent = 0;
    maxDescent = 0;
    QString chars = "!ilq";
    {
        CharMetricsF x(font, chars.toAscii());

        for (int i=0; i< chars.length(); ++i)
        {
            char ch = chars.at(i).toAscii();

            if (x.inkWidth(ch) > maxWidth)
                maxWidth = x.inkWidth(ch);

            // To check the height, we must add both the ascent and descents! ugh.
            QPainterPath path;
            path.addText(0, 0, font, QChar(ch));

            QRectF bb = path.boundingRect();
            if (-bb.top() > maxAscent)
                maxAscent = -bb.top();
            if (bb.bottom() > maxDescent)
                maxDescent = bb.bottom();
        }

        QCOMPARE(x.width(), maxWidth);
        QCOMPARE(x.height(), maxAscent + maxDescent);
        QCOMPARE(x.size(), QSizeF(maxWidth, maxAscent + maxDescent));
    }

    // ------------------------------------------------------------------------
    // Test: origin - using maxAscent from previous tests and specific characters
    {
        bool touchedTop = false;

        CharMetricsF x(font, chars.toAscii());
        QImage image(QSize(ceil(x.size().width()), ceil(x.size().height())), QImage::Format_ARGB32_Premultiplied);
        for (int i=0; i< chars.length(); ++i)
        {
            char ch = chars.at(i).toAscii();

            // To check the height, we must add both the ascent and descents! ugh.
            image.fill(0);

            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(blue);

            QPainterPath path;
            path.addText(x.origin(ch), font, QChar(ch));
            painter.drawPath(path);
            painter.end();

            Rect bb = ::boundingRect(image);

            if (!touchedTop && bb.top() == 0)
                touchedTop = true;

            QVERIFY(bb.top() >= 0);
            // Check that the left and right amounts are within 1 pixel of each other
            int rightAmount = image.width() - 1 - bb.right();
            QVERIFY(bb.left() == rightAmount ||
                    bb.left() + 1 == rightAmount ||
                    bb.left() - 1 == rightAmount);

            // Check the inkTopLeft
            int inkLeft = qRound(x.inkTopLeft(ch).x());
            int inkTop = qRound(x.inkTopLeft(ch).y());
            QVERIFY(bb.left() == inkLeft || bb.left() == inkLeft + 1 || bb.left() == inkLeft - 1);
            QVERIFY(bb.top() == inkTop || bb.top() == inkTop + 1 || bb.top() == inkTop - 1);
        }
        QVERIFY(touchedTop);
    }
}

QTEST_MAIN(TestCharMetricsF)
#include "TestCharMetricsF.moc"
