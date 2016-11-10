/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "AbstractCharPixmapProvider.h"

/**
  * Mock implemetation of the AbstractCharPixmapProvider interface that simply returns a X by X pixmap (where X is the
  * number of times the renderGlyph method has been called multipled by 2) regardless of the character and color
  * parameters.
  */
class MockCharPixmapProvider : public AbstractCharPixmapProvider
{
public:
    MockCharPixmapProvider(QObject *parent = 0) : AbstractCharPixmapProvider(parent)
    {
        renderGlyphCalls_ = 0;
    }

protected:
    QPixmap renderGlyph(char /* character */, const QColor & /* color */)
    {
        renderGlyphCalls_++;
        return QPixmap(renderGlyphCalls_ * 2, renderGlyphCalls_ * 2);
    }

public:
    int renderGlyphCalls_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TestAbstractCharPixmapProvider : public QObject
{
    Q_OBJECT

private slots:
    void setCachingEnabled();
    void clearCache();
    void glyphWithoutCache();
    void glyphWithCache();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAbstractCharPixmapProvider::setCachingEnabled()
{
    MockCharPixmapProvider x;

    // Test: by default, caching is enabled
    QVERIFY(x.isCachingEnabled());

    // Test: setting cache enabled to false
    x.setCachingEnabled(false);
    QVERIFY(!x.isCachingEnabled());

    // Test: setting cache enabled to true
    x.setCachingEnabled(true);
    QVERIFY(x.isCachingEnabled());
}

void TestAbstractCharPixmapProvider::clearCache()
{
    MockCharPixmapProvider x;

    QSignalSpy spyCacheCleared(&x, SIGNAL(cacheCleared()));

    x.clearCache();
    QCOMPARE(spyCacheCleared.count(), 1);
}

void TestAbstractCharPixmapProvider::glyphWithoutCache()
{
    MockCharPixmapProvider x;

    x.setCachingEnabled(false);

    // Test: pixmap returned should be a 2x2 glyph
    QPixmap pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 2);
    QCOMPARE(pixmap.width(), 2);
    QCOMPARE(x.renderGlyphCalls_, 1);

    // Test: calling glyph should always call renderGlyph - even with the same arguments
    pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 4);
    QCOMPARE(pixmap.width(), 4);
    QCOMPARE(x.renderGlyphCalls_, 2);

    // Test: glyph with different arguments
    pixmap = x.glyph('b', Qt::white);
    QCOMPARE(pixmap.height(), 6);
    QCOMPARE(pixmap.width(), 6);
    QCOMPARE(x.renderGlyphCalls_, 3);
}

void TestAbstractCharPixmapProvider::glyphWithCache()
{
    MockCharPixmapProvider x;

    QVERIFY(x.isCachingEnabled());

    // Test: pixmap returned should be a 2x2 glyph
    QPixmap pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 2);
    QCOMPARE(pixmap.width(), 2);
    QCOMPARE(x.renderGlyphCalls_, 1);

    // Test: with cache enabled, calling glyph with same arguments should return cached result
    pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 2);
    QCOMPARE(pixmap.width(), 2);
    QCOMPARE(x.renderGlyphCalls_, 1);

    // Test: glyph with different arguments
    pixmap = x.glyph('b', Qt::black);
    QCOMPARE(pixmap.height(), 4);
    QCOMPARE(pixmap.width(), 4);
    QCOMPARE(x.renderGlyphCalls_, 2);

    pixmap = x.glyph('b', Qt::black);
    QCOMPARE(pixmap.height(), 4);
    QCOMPARE(pixmap.width(), 4);
    QCOMPARE(x.renderGlyphCalls_, 2);

    pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 2);
    QCOMPARE(pixmap.width(), 2);
    QCOMPARE(x.renderGlyphCalls_, 2);

    // Test: clear the cache and retry
    x.clearCache();
    pixmap = x.glyph('a', Qt::black);
    QCOMPARE(pixmap.height(), 6);
    QCOMPARE(pixmap.width(), 6);
    QCOMPARE(x.renderGlyphCalls_, 3);

    pixmap = x.glyph('b', Qt::black);
    QCOMPARE(pixmap.height(), 8);
    QCOMPARE(pixmap.width(), 8);
    QCOMPARE(x.renderGlyphCalls_, 4);
}

QTEST_MAIN(TestAbstractCharPixmapProvider)
#include "TestAbstractCharPixmapProvider.moc"
