/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QPainter>

#include "FontCharPixmapProvider.h"

class TestFontCharPixmapProvider : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void setScale();        // Also tests scale
    void setFont();         // Also tests font
    void height();          // Also tests unscaledHeight
    void width();           // Also tests unscaledWidth
    void glyph();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestFontCharPixmapProvider::constructor()
{
    QFont font("monospace");

    // Test: defaults
    {
        FontCharPixmapProvider x(font);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), 1.0);
    }

    // Test: optional constructor arguments
    {
        FontCharPixmapProvider x(font, 2.0);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), 2.0);
    }

    {
        FontCharPixmapProvider x(font, 0.5);

        QCOMPARE(x.font(), font);
        QCOMPARE(x.scale(), .5);
    }
}

void TestFontCharPixmapProvider::setScale()
{
    QFont font("monospace");
    FontCharPixmapProvider x(font);

    QSignalSpy spyScaleChanged(&x, SIGNAL(scaleChanged()));
    QSignalSpy spyCacheCleared(&x, SIGNAL(cacheCleared()));

    x.setScale(2.);
    QCOMPARE(x.scale(), 2.);
    QCOMPARE(spyScaleChanged.count(), 1);
    QCOMPARE(spyCacheCleared.count(), 1);
    x.setScale(.5);
    QCOMPARE(x.scale(), .5);
    QCOMPARE(spyScaleChanged.count(), 2);
    QCOMPARE(spyCacheCleared.count(), 2);
    x.setScale(0.1);
    QCOMPARE(x.scale(), 0.1);
    QCOMPARE(spyScaleChanged.count(), 3);
    QCOMPARE(spyCacheCleared.count(), 3);

    // Test: setting scale to same should not emit scaleChanged signal
    x.setScale(x.scale());
    QCOMPARE(spyScaleChanged.count(), 3);
    QCOMPARE(spyCacheCleared.count(), 3);
}

void TestFontCharPixmapProvider::setFont()
{
    QFont font1("monospace");
    FontCharPixmapProvider x(font1);

    QSignalSpy spyFontChanged(&x, SIGNAL(fontChanged()));
    QSignalSpy spyCacheCleared(&x, SIGNAL(cacheCleared()));

    // Test: set font to different family
    QFont font2 = QFont("courier new");
    x.setFont(font2);
    QCOMPARE(x.font(), font2);
    QCOMPARE(spyFontChanged.count(), 1);
    QCOMPARE(spyCacheCleared.count(), 1);

    // Test: set font back to original
    x.setFont(font1);
    QCOMPARE(x.font(), font1);
    QCOMPARE(spyFontChanged.count(), 2);
    QCOMPARE(spyCacheCleared.count(), 2);

    // Test: set font to same family but different size
    QFont font3("monospace", 72);
    x.setFont(font3);
    QCOMPARE(x.font(), font3);
    QCOMPARE(spyFontChanged.count(), 3);
    QCOMPARE(spyCacheCleared.count(), 3);

    // Test: setting font to same font should not emit changed signal
    x.setFont(x.font());
    QCOMPARE(spyFontChanged.count(), 3);
    QCOMPARE(spyCacheCleared.count(), 3);
}

void TestFontCharPixmapProvider::height()
{
    QFont font;
    FontCharPixmapProvider x(font);

    // Test: With defaults, height should equal unscaledHeight which should equal the font height
    QCOMPARE(x.height(), QFontMetricsF(font).height());
    QCOMPARE(x.height(), x.unscaledHeight());

    // Test: doubled scale factor
    qreal scale = 2.;
    x.setScale(scale);
    QCOMPARE(x.height(), scale * QFontMetricsF(font).height());
    QCOMPARE(x.unscaledHeight(), QFontMetricsF(font).height());

    // Test: half scale factor
    scale = .5;
    x.setScale(scale);
    QCOMPARE(x.height(), scale * QFontMetricsF(font).height());
    QCOMPARE(x.unscaledHeight(), QFontMetricsF(font).height());

    // Test: increased font size
    QFont newFont(x.font());
    newFont.setPointSize(font.pointSize() + 12);
    x.setFont(newFont);
    QCOMPARE(x.height(), scale * QFontMetricsF(newFont).height());
    QCOMPARE(x.unscaledHeight(), QFontMetricsF(newFont).height());

    // Test: decreased font size
    newFont.setPointSize(font.pointSize() - 2);
    x.setFont(newFont);
    QCOMPARE(x.height(), scale * QFontMetricsF(newFont).height());
    QCOMPARE(x.unscaledHeight(), QFontMetricsF(newFont).height());
}

void TestFontCharPixmapProvider::width()
{
    QFont font;
    FontCharPixmapProvider x(font);

    QString string = "ABCDEFGHJIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwx1235435234985!@#$!#$%!";
    for (int i=0, z=string.length(); i<z; ++i)
    {
        char ch = string.at(i).toAscii();

        x.setFont(font);
        x.setScale(1.);

        // Test: With defaults, width should equal unscaledWidth which should equal the font width
        QCOMPARE(x.width(ch), QFontMetricsF(font).width(ch));
        QCOMPARE(x.width(ch), x.unscaledWidth(ch));

        // Test: doubled scale factor
        qreal scale = 2.;
        x.setScale(scale);
        QCOMPARE(x.width(ch), scale * QFontMetricsF(font).width(ch));
        QCOMPARE(x.unscaledWidth(ch), QFontMetricsF(font).width(ch));

        // Test: half scale factor
        scale = .5;
        x.setScale(scale);
        QCOMPARE(x.width(ch), scale * QFontMetricsF(font).width(ch));
        QCOMPARE(x.unscaledWidth(ch), QFontMetricsF(font).width(ch));

        // Test: increased font size
        QFont newFont(x.font());
        newFont.setPointSize(font.pointSize() + 12);
        x.setFont(newFont);
        QCOMPARE(x.width(ch), scale * QFontMetricsF(newFont).width(ch));
        QCOMPARE(x.unscaledWidth(ch), QFontMetricsF(newFont).width(ch));

        // Test: decreased font size
        newFont.setPointSize(font.pointSize() - 2);
        x.setFont(newFont);
        QCOMPARE(x.width(ch), scale * QFontMetricsF(newFont).width(ch));
        QCOMPARE(x.unscaledWidth(ch), QFontMetricsF(newFont).width(ch));
    }
}

QPixmap renderGlyph(const QFont &font, char character, const QColor &color, qreal scale = 1.)
{
    QFontMetricsF fm(font);

    // Draw the scaled character on an image surface
    qreal cw = scale * fm.width(character);
    qreal ch = scale * fm.height();
    QImage image(QSize(static_cast<int>(cw), static_cast<int>(ch)), QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);
    painter.setFont(font);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(color);
    painter.scale(scale, scale);

    // Drawing with unscaled parameters because it will scale automatically
//	painter.drawText(fm.leftBearing(character), fm.ascent(), QChar(character));
	painter.drawText(0, fm.ascent(), QChar(character));
	painter.end();

//	qDebug() << ">>>" << character << fm.leftBearing(character) << fm.ascent();

	return QPixmap::fromImage(image);
}

void TestFontCharPixmapProvider::glyph()
{
    QFont font("monospace", 18);
    FontCharPixmapProvider x(font);
    QString characters = "ABCDEFGHJIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy1234567890!@#$%^&*()_+{}";
    QList<QColor> colors;
    colors << Qt::white << Qt::black << Qt::red;

    // Test: rendering basic various glyphs with no scaling
    foreach (QColor color, colors)
    {
        for (int i=0, z= characters.length(); i<z; ++i)
        {
            char ch = characters.at(i).toAscii();

            QPixmap glyph = x.glyph(ch, color);

            /*
            qDebug() << ch << glyph.width() << glyph.height() << glyph.toImage().format();
            for (int i=0, z=glyph.height(); i<z; ++i)
            {
                for (int j=0, y=glyph.width(); j<y; ++j)
                    printf("%3d ", qAlpha(glyph.toImage().pixel(j, i)));
                printf("\n");
            }

            QPixmap blah = renderGlyph(font, ch, color);
            qDebug() << blah.width() << blah.height() << blah.toImage().format();
            for (int i=0, z=blah.height(); i<z; ++i)
            {
                for (int j=0, y=blah.width(); j<y; ++j)
                    printf("%3d ", qAlpha(blah.toImage().pixel(j, i)));
                printf("\n");
            }
            */

            QCOMPARE(glyph.toImage(), renderGlyph(font, ch, color).toImage());
        }
    }

    // Test: rendering at doubled scale
    x.setScale(2.);
    foreach (QColor color, colors)
    {
        for (int i=0, z= characters.length(); i<z; ++i)
        {
            char ch = characters.at(i).toAscii();

            QPixmap glyph = x.glyph(ch, color);
            QCOMPARE(glyph.toImage(), renderGlyph(font, ch, color, 2.).toImage());
        }
    }

    // Test: rendering at half scale
    x.setScale(.5);
    foreach (QColor color, colors)
    {
        for (int i=0, z= characters.length(); i<z; ++i)
        {
            char ch = characters.at(i).toAscii();

            QPixmap glyph = x.glyph(ch, color);
            QCOMPARE(glyph.toImage(), renderGlyph(font, ch, color, .5).toImage());
        }
    }
}


QTEST_MAIN(TestFontCharPixmapProvider)
#include "TestFontCharPixmapProvider.moc"
