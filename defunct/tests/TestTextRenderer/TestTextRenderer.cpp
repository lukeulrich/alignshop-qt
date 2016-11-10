/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QPainter>

#include "util/CharMetrics.h"
#include "TextRenderer.h"
#include "global.h"

class TestTextRenderer : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void setUseFloatMetrics();
    void setScale();        // Also tests scale
    void setFont();         // Also tests font
    void height();          // Also tests unscaledHeight
    void width();           // Also tests unscaledWidth
    void size();            // Also tests sizeF, unscaledSize

    // Using the floating point metrics
    void floatingHeight();
    void floatingWidth();
    void floatingSize();

    void drawChar();        // Also tests the floating point rendering version
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTextRenderer::constructor()
{
    QFont font("monospace");

    // ------------------------------------------------------------------------
    // Test: defaults
    {
        TextRenderer x(font);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), 1.0);
    }

    // ------------------------------------------------------------------------
    // Test: optional constructor arguments
    {
        TextRenderer x(font, 2.0);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), 2.0);
    }

    {
        TextRenderer x(font, 0.5);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), .5);
    }
}

void TestTextRenderer::setUseFloatMetrics()
{
    TextRenderer x(QFont("monospace"));

    QCOMPARE(x.useFloatMetrics(), false);
    x.setUseFloatMetrics(true);
    QCOMPARE(x.useFloatMetrics(), true);
    x.setUseFloatMetrics(false);
    QCOMPARE(x.useFloatMetrics(), false);
}

void TestTextRenderer::setScale()
{
    QFont font("monospace");
    TextRenderer x(font);

    QSignalSpy spyScaleChanged(&x, SIGNAL(scaleChanged()));

    x.setScale(2.);
    QCOMPARE(x.scale(), 2.);
    QCOMPARE(spyScaleChanged.count(), 1);
    x.setScale(.5);
    QCOMPARE(x.scale(), .5);
    QCOMPARE(spyScaleChanged.count(), 2);
    x.setScale(0.1);
    QCOMPARE(x.scale(), 0.1);
    QCOMPARE(spyScaleChanged.count(), 3);

    // Test: setting scale to same should not emit scaleChanged signal
    x.setScale(x.scale());
    QCOMPARE(spyScaleChanged.count(), 3);
}

void TestTextRenderer::setFont()
{
    QFont font1("monospace");
    TextRenderer x(font1);

    QSignalSpy spyFontChanged(&x, SIGNAL(fontChanged()));

    // ------------------------------------------------------------------------
    // Test: set font to different family
    QFont font2 = QFont("courier new");
    x.setFont(font2);
    QCOMPARE(x.font(), font2);
    QCOMPARE(spyFontChanged.count(), 1);

    // ------------------------------------------------------------------------
    // Test: set font back to original
    x.setFont(font1);
    QCOMPARE(x.font(), font1);
    QCOMPARE(spyFontChanged.count(), 2);

    // ------------------------------------------------------------------------
    // Test: set font to same family but different size
    QFont font3("monospace", 72);
    x.setFont(font3);
    QCOMPARE(x.font(), font3);
    QCOMPARE(spyFontChanged.count(), 3);

    // ------------------------------------------------------------------------
    // Test: setting font to same font should not emit changed signal
    x.setFont(x.font());
    QCOMPARE(spyFontChanged.count(), 3);
}

void TestTextRenderer::height()
{
    QFont font;
    TextRenderer x(font);

    // ------------------------------------------------------------------------
    // Test: With defaults, height should equal unscaledHeight which should equal the font height
    QCOMPARE(x.height(), static_cast<qreal>(qRound(CharMetrics(font).height())));
    QCOMPARE(x.height(), x.unscaledHeight());

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.height(), static_cast<qreal>(qRound(scale * CharMetrics(font).height())));
    QCOMPARE(x.unscaledHeight(), static_cast<qreal>(CharMetrics(font).height()));

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.height(), static_cast<qreal>(qRound(scale * CharMetrics(font).height())));
    QCOMPARE(x.unscaledHeight(), static_cast<qreal>(CharMetrics(font).height()));

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.height(), static_cast<qreal>(qRound(scale * CharMetrics(newFont).height())));
    QCOMPARE(x.unscaledHeight(), static_cast<qreal>(CharMetrics(newFont).height()));

    // ------------------------------------------------------------------------
    // Test: decreased font size
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.height(), static_cast<qreal>(qRound(scale * CharMetrics(newFont).height())));
    QCOMPARE(x.unscaledHeight(), static_cast<qreal>(CharMetrics(newFont).height()));
}

void TestTextRenderer::width()
{
    QFont font;
    TextRenderer x(font);

    x.setFont(font);
    x.setScale(1.);

    // ------------------------------------------------------------------------
    // Test: With defaults, width should equal unscaledWidth which should equal the font width
    QCOMPARE(x.width(), static_cast<qreal>(qRound(CharMetrics(font).width())));
    QCOMPARE(x.width(), x.unscaledWidth());

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.width(), static_cast<qreal>(qRound(scale * CharMetrics(font).width())));
    QCOMPARE(x.unscaledWidth(), static_cast<qreal>(CharMetrics(font).width()));

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.width(), static_cast<qreal>(qRound(scale * CharMetrics(font).width())));
    QCOMPARE(x.unscaledWidth(), static_cast<qreal>(CharMetrics(font).width()));

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.width(), static_cast<qreal>(qRound(scale * CharMetrics(newFont).width())));
    QCOMPARE(x.unscaledWidth(), static_cast<qreal>(CharMetrics(newFont).width()));

    // ------------------------------------------------------------------------
    // Test: decreased font size
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.width(), static_cast<qreal>(qRound(scale * CharMetrics(newFont).width())));
    QCOMPARE(x.unscaledWidth(), static_cast<qreal>(CharMetrics(newFont).width()));
}

void TestTextRenderer::size()
{
    QFont font;
    TextRenderer x(font);

    x.setFont(font);
    x.setScale(1.);

    // ------------------------------------------------------------------------
    // Test: With defaults, size should equal unscaledSize which should equal the font size
    QCOMPARE(x.sizeF(), QSizeF(x.width(), x.height()));
    QCOMPARE(x.size(), QSize(x.unscaledWidth(), x.unscaledHeight()));
    QCOMPARE(x.sizeF(), QSizeF(x.unscaledWidth(), x.unscaledHeight()));
    QCOMPARE(x.unscaledSize(), QSizeF(x.unscaledWidth(), x.unscaledHeight()));

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetrics(font).width(), scale * CharMetrics(font).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetrics(font).width())), static_cast<int>(ceil(scale * CharMetrics(font).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetrics(font).width(), CharMetrics(font).height()));

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetrics(font).width(), scale * CharMetrics(font).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetrics(font).width())), static_cast<int>(ceil(scale * CharMetrics(font).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetrics(font).width(), CharMetrics(font).height()));

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetrics(newFont).width(), scale * CharMetrics(newFont).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetrics(newFont).width())), static_cast<int>(ceil(scale * CharMetrics(newFont).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetrics(newFont).width(), CharMetrics(newFont).height()));

    // ------------------------------------------------------------------------
    // Test: decreased font size - not use floating point metrics
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.sizeF(), QSizeF(static_cast<qreal>(qRound(scale * CharMetrics(newFont).width())), static_cast<qreal>(qRound(scale * CharMetrics(newFont).height()))));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetrics(newFont).width())), static_cast<int>(ceil(scale * CharMetrics(newFont).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetrics(newFont).width(), CharMetrics(newFont).height()));
}

void TestTextRenderer::floatingHeight()
{
    QFont font;
    TextRenderer x(font);
    x.setUseFloatMetrics(true);

    // ------------------------------------------------------------------------
    // Test: With defaults, height should equal unscaledHeight which should equal the font height
    QCOMPARE(x.height(), CharMetricsF(font).height());
    QCOMPARE(x.height(), x.unscaledHeight());

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.height(), scale * CharMetricsF(font).height());
    QCOMPARE(x.unscaledHeight(), CharMetricsF(font).height());

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.height(), scale * CharMetricsF(font).height());
    QCOMPARE(x.unscaledHeight(), CharMetricsF(font).height());

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.height(), scale * CharMetricsF(newFont).height());
    QCOMPARE(x.unscaledHeight(), CharMetricsF(newFont).height());

    // ------------------------------------------------------------------------
    // Test: decreased font size
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.height(), scale * CharMetricsF(newFont).height());
    QCOMPARE(x.unscaledHeight(), CharMetricsF(newFont).height());
}

void TestTextRenderer::floatingWidth()
{
    QFont font;
    TextRenderer x(font);
    x.setUseFloatMetrics(true);

    x.setFont(font);
    x.setScale(1.);

    // ------------------------------------------------------------------------
    // Test: With defaults, width should equal unscaledWidth which should equal the font width
    QCOMPARE(x.width(), CharMetricsF(font).width());
    QCOMPARE(x.width(), x.unscaledWidth());

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.width(), scale * CharMetricsF(font).width());
    QCOMPARE(x.unscaledWidth(), CharMetricsF(font).width());

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.width(), scale * CharMetricsF(font).width());
    QCOMPARE(x.unscaledWidth(), CharMetricsF(font).width());

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.width(), scale * CharMetricsF(newFont).width());
    QCOMPARE(x.unscaledWidth(), CharMetricsF(newFont).width());

    // ------------------------------------------------------------------------
    // Test: decreased font size
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.width(), scale * CharMetricsF(newFont).width());
    QCOMPARE(x.unscaledWidth(), CharMetricsF(newFont).width());
}

void TestTextRenderer::floatingSize()
{
    QFont font;
    TextRenderer x(font);
    x.setUseFloatMetrics(true);

    x.setFont(font);
    x.setScale(1.);

    // ------------------------------------------------------------------------
    // Test: With defaults, size should equal unscaledSize which should equal the font size
    QCOMPARE(x.sizeF(), QSizeF(x.width(), x.height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(x.unscaledWidth())), static_cast<int>(ceil(x.unscaledHeight()))));
    QCOMPARE(x.sizeF(), QSizeF(x.unscaledWidth(), x.unscaledHeight()));
    QCOMPARE(x.unscaledSize(), QSizeF(x.unscaledWidth(), x.unscaledHeight()));

    // ------------------------------------------------------------------------
    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetricsF(font).width(), scale * CharMetricsF(font).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetricsF(font).width())), static_cast<int>(ceil(scale * CharMetricsF(font).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetricsF(font).width(), CharMetricsF(font).height()));

    // ------------------------------------------------------------------------
    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetricsF(font).width(), scale * CharMetricsF(font).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetricsF(font).width())), static_cast<int>(ceil(scale * CharMetricsF(font).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetricsF(font).width(), CharMetricsF(font).height()));

    // ------------------------------------------------------------------------
    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetricsF(newFont).width(), scale * CharMetricsF(newFont).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetricsF(newFont).width())), static_cast<int>(ceil(scale * CharMetricsF(newFont).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetricsF(newFont).width(), CharMetricsF(newFont).height()));

    // ------------------------------------------------------------------------
    // Test: decreased font size - not use floating point metrics
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.sizeF(), QSizeF(scale * CharMetricsF(newFont).width(), scale * CharMetricsF(newFont).height()));
    QCOMPARE(x.size(), QSize(static_cast<int>(ceil(scale * CharMetricsF(newFont).width())), static_cast<int>(ceil(scale * CharMetricsF(newFont).height()))));
    QCOMPARE(x.unscaledSize(), QSizeF(CharMetricsF(newFont).width(), CharMetricsF(newFont).height()));
}

QImage renderImage(const QPointF &pointF, const QFont &font, char character, const TextColorStyle &colorStyle, const QSize imageSize, qreal scale = 1., bool useFloating = false)
{
    qreal width;
    qreal height;
    QPointF origin;
    if (useFloating)
    {
        CharMetricsF cm(font);
        width = cm.width();
        height = cm.height();
        origin = cm.origin(character);
    }
    else
    {
        CharMetrics cm(font);
        width = cm.width();
        height = cm.height();
        origin = cm.origin(character);
    }

    // Draw the scaled character on an image surface
    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);
    painter.fillRect(QRectF(pointF, QSizeF(scale * width, scale * height)), colorStyle.background_);
    painter.setFont(font);
    painter.setPen(colorStyle.foreground_);
    painter.scale(scale, scale);

    // Drawing with unscaled parameters because it will scale automatically
    painter.drawText(pointF + origin, QChar(character));
    painter.end();

//	qDebug() << ">>>" << character << fm.leftBearing(character) << fm.ascent();

    return image;
}

void TestTextRenderer::drawChar()
{
    QFont font("monospace", 18);
    TextRenderer x(font);
    QList<TextColorStyle> colors;
    colors << TextColorStyle(Qt::white, Qt::transparent)
           << TextColorStyle(Qt::black, Qt::green)
           << TextColorStyle(Qt::red, Qt::magenta);

    // ------------------------------------------------------------------------
    // Render tests at 1x, 2x, and half-x
    QList<qreal> scales;
    scales << 1. << 2. << .5;

    foreach (qreal scale, scales)
    {
        x.setScale(scale);
        foreach (const TextColorStyle &color, colors)
        {
            const char *pch = constants::k7BitCharacters;
            while (*pch)
            {
                x.setUseFloatMetrics(false);
                QSize size(x.width() + 2, x.height() + 2);
                QImage glyph(size, QImage::Format_ARGB32_Premultiplied);
                glyph.fill(0);
                QPainter painter(&glyph);
                QPointF origin(0, 0);
                x.drawChar(origin, *pch, color, &painter);
                painter.end();

//                qDebug() << ch << glyph.width() << glyph.height() << glyph.format();
//                for (int j=0, z=glyph.height(); j<z; ++j)
//                {
//                    for (int k=0, y=glyph.width(); k<y; ++k)
//                        printf("%3d ", qAlpha(glyph.pixel(k, j)));
//                    printf("\n");
//                }

//                QImage blah = renderImage(origin, font, ch, color, size, scale);
//                qDebug() << blah.width() << blah.height() << blah.format();
//                for (int j=0, z=blah.height(); j<z; ++j)
//                {
//                    for (int k=0, y=blah.width(); k<y; ++k)
//                        printf("%3d ", qAlpha(blah.pixel(k, j)));
//                    printf("\n");
//                }

//                qDebug() << scale << i << ch;

                QCOMPARE(glyph, renderImage(origin, font, *pch, color, size, scale));

                // Now test the floating point version

                x.setUseFloatMetrics(true);
                glyph.fill(0);
                painter.begin(&glyph);
                x.drawChar(origin, *pch, color, &painter);
                painter.end();

                QCOMPARE(glyph, renderImage(origin, font, *pch, color, size, scale, true));

                ++pch;
            }
        }
    }
}


QTEST_MAIN(TestTextRenderer)
#include "TestTextRenderer.moc"
