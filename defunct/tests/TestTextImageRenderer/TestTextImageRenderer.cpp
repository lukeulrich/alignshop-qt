/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtGui/QPainter>
#include <QtGui/QFontMetricsF>

#include "TextImageRenderer.h"
#include "BasicTextRenderer.h"


class TestTextImageRenderer : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void cacheKey();
    void drawCharRenderImage();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTextImageRenderer::constructor()
{
    TextImageRenderer x(QFont("monospace"));

    // ------------------------------------------------------------------------
    // Test: signals to clear the cache should be hooked up
    QVERIFY(disconnect(&x, SIGNAL(fontChanged()), &x, SLOT(clearCache())));
    QVERIFY(disconnect(&x, SIGNAL(scaleChanged()), &x, SLOT(clearCache())));
}

void TestTextImageRenderer::cacheKey()
{
    TextImageRenderer x(QFont("courier new"));

    // ------------------------------------------------------------------------
    // Test: different combinations
    QList<TextColorStyle> styles;
    styles << TextColorStyle()
           << TextColorStyle(Qt::red, Qt::green)
           << TextColorStyle(Qt::blue, Qt::yellow);
    QList<char> chars;
    chars << 'a' << 'b' << '%' << 'c';

    foreach (char ch, chars)
        foreach (const TextColorStyle &style, styles)
            QCOMPARE(x.cacheKey(ch, style), QString(ch) + style.foreground_.name() + style.background_.name());
}

void diffImages(const QImage &a, const QImage &b)
{
	qDebug() << a.width() << a.height() << a.format();
	for (int j=0, z=a.height(); j<z; ++j)
	{
		for (int k=0, y=a.width(); k<y; ++k)
			printf("%3d ", qAlpha(a.pixel(k, j)));
		printf("\n");
	}

	qDebug() << b.width() << b.height() << b.format();
	for (int j=0, z=b.height(); j<z; ++j)
	{
		for (int k=0, y=b.width(); k<y; ++k)
			printf("%3d ", qAlpha(b.pixel(k, j)));
		printf("\n");
	}
}

void TestTextImageRenderer::drawCharRenderImage()
{
    QFont font("monospace", 18);

    BasicTextRenderer textRenderer(font);

    TextImageRenderer x(font);
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

                // ------------------------------------------------------------------------
                // Test: protected render image function
                QImage image(x.size().toSize(), QImage::Format_ARGB32_Premultiplied);
                QPainter painter(&image);
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                textRenderer.drawChar(QPointF(0, 0), ch, style, &painter);
                painter.end();
                QCOMPARE(image, x.renderImage(ch, style));

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
                painter3.drawImage(origin, image);
                painter3.end();

//                qDebug() << scale << i << ch;
//                QImage expected = renderImage(origin, font, ch, style, size, scale);
//                diffImages(canvas, expected);
                QCOMPARE(canvas, expected);
            }
        }
    }
}

QTEST_MAIN(TestTextImageRenderer)
#include "TestTextImageRenderer.moc"
