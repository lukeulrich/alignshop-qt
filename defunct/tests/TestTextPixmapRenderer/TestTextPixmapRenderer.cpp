/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QPainter>

#include "TextPixmapRenderer.h"

class TestTextPixmapRenderer : public QObject
{
    Q_OBJECT

private slots:
    void drawChar();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTextPixmapRenderer::drawChar()
{
    QFont font("monospace", 18);

    TextRenderer textRenderer(font);

    TextPixmapRenderer x(font);
    QString characters = "ABCDEFGHJIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy1234567890!@#$%^&*()_+{}";
    QList<TextColorStyle> styles;
    styles << TextColorStyle(Qt::white, Qt::transparent)
           << TextColorStyle(Qt::black, Qt::green)
           << TextColorStyle(Qt::red, Qt::magenta);

    // ------------------------------------------------------------------------
    // Render tests at 1x, 2x, and half-x
    QList<qreal> scales;
    scales << 1. << 2. << .5;

    foreach (qreal scale, scales)
    {
        textRenderer.setScale(scale);
        x.setScale(scale);
        foreach (const TextColorStyle &style, styles)
        {
            for (int i=0, z= characters.length(); i<z; ++i)
            {
                char ch = characters.at(i).toAscii();

                QPixmap pixmap = QPixmap::fromImage(x.renderImage(ch, style));

                // ------------------------------------------------------------------------
                // Test: public drawChar method
                QSize size(i + x.width() + 2, i + x.height() + 2);
                QImage canvas(size, QImage::Format_ARGB32_Premultiplied);
                canvas.fill(0);
                QPainter painter2(&canvas);
                QPointF origin(i, i);
                x.drawChar(origin, ch, style, &painter2);
                painter2.end();

                QImage expected(size, QImage::Format_ARGB32_Premultiplied);
                expected.fill(0);
                QPainter painter3(&expected);
                painter3.drawPixmap(origin, pixmap);
                painter3.end();

//                qDebug() << scale << i << ch;
//                QImage expected = renderImage(origin, font, ch, style, size, scale);
//                diffImages(canvas, expected);
                QCOMPARE(canvas, expected);
            }
        }
    }
}


QTEST_MAIN(TestTextPixmapRenderer)
#include "TestTextPixmapRenderer.moc"
