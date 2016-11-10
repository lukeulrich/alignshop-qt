/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QPoint>
#include <QtDebug>

#include "Rect.h"

class TestRect : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void constructorPoints_data();
    void constructorPoints();
    void constructorPointSize_data();
    void constructorPointSize();
    void constructorXYWH_data();
    void constructorXYWH();

    void operator_eqeq();
    void operator_ne();

    void isNull();
    void center();
    void centerF();

    void setBottom();
    void setLeft();
    void setX();
    void setRight();
    void setTop();
    void setY();
    void setRect_data();
    void setRect();
    void setRectPoints_data();
    void setRectPoints();

    void setHeight();
    void setWidth();
    void setSize();

    void containsPoint();
    void containsXY();
    void containsRect();
    void intersects();
    void intersection();

    void moveLeft();
    void moveRight();
    void moveTop();
    void moveBottom();

    void normalized();

    void toQRectF();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestRect::constructor()
{
    Rect rect;
    QVERIFY(rect.isNull());
    QCOMPARE(rect.x(), 0);
    QCOMPARE(rect.y(), 0);
    QCOMPARE(rect.right(), 0);
    QCOMPARE(rect.bottom(), 0);
}

void TestRect::constructorPoints_data()
{
    QTest::addColumn<QPoint>("a");
    QTest::addColumn<QPoint>("b");
    QTest::addColumn<bool>("isNull");

    QTest::newRow("Null rectangle") << QPoint(0, 0) << QPoint(0, 0) << true;
    QTest::newRow("(1,1) -> (10,10)") << QPoint(1, 1) << QPoint(10, 10) << false;
    QTest::newRow("(10,1) -> (1,10)") << QPoint(10, 1) << QPoint(1, 10) << false;

    QTest::newRow("(-1,-1) -> (-10,-10)") << QPoint(-1, -1) << QPoint(-10, -10) << false;
    QTest::newRow("(-10,-1) -> (-1,-10)") << QPoint(-10, -1) << QPoint(-1, -10) << false;
}

void TestRect::constructorPoints()
{
    QFETCH(QPoint, a);
    QFETCH(QPoint, b);
    QFETCH(bool, isNull);

    // Test: can only test two quadrants

    // Quadrant I
    {
        Rect rect(a, b);
        QCOMPARE(rect.isNull(), isNull);
        QCOMPARE(rect.x(), a.x());
        QCOMPARE(rect.y(), a.y());
        QCOMPARE(rect.right(), b.x());
        QCOMPARE(rect.bottom(), b.y());
    }

    // Quadrant IV
    {
        Rect rect(b, a);
        QCOMPARE(rect.isNull(), isNull);
        QCOMPARE(rect.x(), b.x());
        QCOMPARE(rect.y(), b.y());
        QCOMPARE(rect.right(), a.x());
        QCOMPARE(rect.bottom(), a.y());
    }
}

void TestRect::constructorPointSize_data()
{
    QTest::addColumn<QPoint>("topLeft");
    QTest::addColumn<QSize>("size");
    QTest::addColumn<bool>("isNull");

    QTest::newRow("null rectangle") << QPoint(0, 0) << QSize(0, 0) << true;
    QTest::newRow("basic rectangle") << QPoint(0, 0) << QSize(10, 5) << false;
    QTest::newRow("basic rectangle 2") << QPoint(1, 1) << QSize(10, 5) << false;
    QTest::newRow("(0, 0) -> (-10, 5)") << QPoint(0, 0) << QSize(-10, 5) << false;
    QTest::newRow("(1, 1) -> (-10, 5)") << QPoint(1, 1) << QSize(-10, 5) << false;
    QTest::newRow("(0, 0) -> (10, -5)") << QPoint(0, 0) << QSize(10, -5) << false;
    QTest::newRow("(1, 1) -> (10, -5)") << QPoint(1, 1) << QSize(10, -5) << false;
    QTest::newRow("(0, 0) -> (-10, -5)") << QPoint(0, 0) << QSize(-10, -5) << false;

    QTest::newRow("(-10, -5) -> (0, 0)") << QPoint(-10, -5) << QSize(10, 5) << false;
    QTest::newRow("(-10, -5) -> (-20, -10)") << QPoint(-20, -10) << QSize(-10, -5) << false;
}

void TestRect::constructorPointSize()
{
    QFETCH(QPoint, topLeft);
    QFETCH(QSize, size);
    QFETCH(bool, isNull);

    Rect rect(topLeft, size);
    QCOMPARE(rect.isNull(), isNull);
    QCOMPARE(rect.x(), topLeft.x());
    QCOMPARE(rect.y(), topLeft.y());
    QCOMPARE(rect.right(), topLeft.x() + size.width());
    QCOMPARE(rect.bottom(), topLeft.y() + size.height());
}

void TestRect::constructorXYWH_data()
{
    QTest::addColumn<int>("x");
    QTest::addColumn<int>("y");
    QTest::addColumn<int>("w");
    QTest::addColumn<int>("h");

    QTest::newRow("(0, 0) -> (0, 0)") << 0 << 0 << 0 << 0;
    QTest::newRow("(3, 2) -> (3, 2)") << 3 << 2 << 0 << 0;
    QTest::newRow("(3, 2) :: [-1, -1]") << 3 << 2 << -1 << -1;
}

void TestRect::constructorXYWH()
{
    QFETCH(int, x);
    QFETCH(int, y);
    QFETCH(int, w);
    QFETCH(int, h);

    Rect rect(x, y, w, h);
    QCOMPARE(rect.x(), x);
    QCOMPARE(rect.y(), y);
    QCOMPARE(rect.width(), w);
    QCOMPARE(rect.height(), h);
    QCOMPARE(rect.right(), x + w);
    QCOMPARE(rect.bottom(), y + h);
}

void TestRect::operator_eqeq()
{
    {
        Rect rect;
        QVERIFY(rect == rect);
        QVERIFY(rect == Rect());
    }

    {
        Rect rect(QPoint(1, 2), QPoint(3, 4));
        QVERIFY(rect == rect);
        QVERIFY(rect == Rect(QPoint(1, 2), QPoint(3, 4)));
    }
}

void TestRect::operator_ne()
{
    {
        Rect rect;
        QVERIFY(rect != Rect(QPoint(1, 1), QSize(2, 2)));
    }

    {
        Rect rect(QPoint(1, 2), QPoint(3, 4));
        QVERIFY(rect != Rect());
        QVERIFY(rect != Rect(QPoint(1, 2), QPoint(4, 4)));
    }
}

void TestRect::isNull()
{
    Rect rect;

    QVERIFY(rect.isNull());
    rect.setWidth(1);
    QVERIFY(rect.isNull() == false);
    rect.setWidth(0);
    QVERIFY(rect.isNull());
    rect.setHeight(1);
    QVERIFY(rect.isNull() == false);
    rect.setHeight(0);
    QVERIFY(rect.isNull());

    rect.setLeft(5);
    rect.setTop(5);
    rect.setWidth(0);
    rect.setHeight(0);
    QVERIFY(rect.isNull());
}

void TestRect::center()
{
    // Default center
    {
        Rect rect;

        QCOMPARE(rect.center(), QPoint(0,0));
    }

    // Quadrant I
    {
        Rect rect;
        rect.setWidth(4);
        QCOMPARE(rect.center(), QPoint(2, 0));
        rect.setHeight(4);
        QCOMPARE(rect.center(), QPoint(2, 2));
        rect.setLeft(1);
        QCOMPARE(rect.center(), QPoint(2, 2));
        rect.setTop(1);
        QCOMPARE(rect.center(), QPoint(2, 2));
        rect.setLeft(2);
        QCOMPARE(rect.center(), QPoint(3, 2));
        rect.setTop(2);
        QCOMPARE(rect.center(), QPoint(3, 3));
    }

    // Quadrant II
    {
        Rect rect;
        rect.setWidth(-4);
        QCOMPARE(rect.center(), QPoint(-2, 0));
        rect.setHeight(4);
        QCOMPARE(rect.center(), QPoint(-2, 2));
        rect.setLeft(-1);
        QCOMPARE(rect.center(), QPoint(-2, 2));
        rect.setTop(1);
        QCOMPARE(rect.center(), QPoint(-2, 2));
        rect.setLeft(-2);
        QCOMPARE(rect.center(), QPoint(-3, 2));
        rect.setTop(2);
        QCOMPARE(rect.center(), QPoint(-3, 3));
    }

    // Quadrant III
    {
        Rect rect;
        rect.setWidth(-4);
        QCOMPARE(rect.center(), QPoint(-2, 0));
        rect.setHeight(-4);
        QCOMPARE(rect.center(), QPoint(-2, -2));
        rect.setLeft(-1);
        QCOMPARE(rect.center(), QPoint(-2, -2));
        rect.setTop(-1);
        QCOMPARE(rect.center(), QPoint(-2, -2));
        rect.setLeft(-2);
        QCOMPARE(rect.center(), QPoint(-3, -2));
        rect.setTop(-2);
        QCOMPARE(rect.center(), QPoint(-3, -3));
    }

    // Quadrant IV
    {
        Rect rect;
        rect.setWidth(4);
        QCOMPARE(rect.center(), QPoint(2, 0));
        rect.setHeight(-4);
        QCOMPARE(rect.center(), QPoint(2, -2));
        rect.setLeft(1);
        QCOMPARE(rect.center(), QPoint(2, -2));
        rect.setTop(-1);
        QCOMPARE(rect.center(), QPoint(2, -2));
        rect.setLeft(2);
        QCOMPARE(rect.center(), QPoint(3, -2));
        rect.setTop(-2);
        QCOMPARE(rect.center(), QPoint(3, -3));
    }
}

void TestRect::centerF()
{
    // Default center
    {
        Rect rect;

        QCOMPARE(rect.centerF(), QPointF(0,0));
    }

    // Quadrant I
    {
        Rect rect;
        rect.setWidth(4);
        QCOMPARE(rect.centerF(), QPointF(2, 0));
        rect.setHeight(4);
        QCOMPARE(rect.centerF(), QPointF(2, 2));
        rect.setLeft(1);
        QCOMPARE(rect.centerF(), QPointF(2.5, 2));
        rect.setTop(1);
        QCOMPARE(rect.centerF(), QPointF(2.5, 2.5));
        rect.setLeft(2);
        QCOMPARE(rect.centerF(), QPointF(3, 2.5));
        rect.setTop(2);
        QCOMPARE(rect.centerF(), QPointF(3, 3));
    }

    // Quadrant II
    {
        Rect rect;
        rect.setWidth(-4);
        QCOMPARE(rect.centerF(), QPointF(-2, 0));
        rect.setHeight(4);
        QCOMPARE(rect.centerF(), QPointF(-2, 2));
        rect.setLeft(-1);
        QCOMPARE(rect.centerF(), QPointF(-2.5, 2));
        rect.setTop(1);
        QCOMPARE(rect.centerF(), QPointF(-2.5, 2.5));
        rect.setLeft(-2);
        QCOMPARE(rect.centerF(), QPointF(-3, 2.5));
        rect.setTop(2);
        QCOMPARE(rect.centerF(), QPointF(-3, 3));
    }

    // Quadrant III
    {
        Rect rect;
        rect.setWidth(-4);
        QCOMPARE(rect.centerF(), QPointF(-2, 0));
        rect.setHeight(-4);
        QCOMPARE(rect.centerF(), QPointF(-2, -2));
        rect.setLeft(-1);
        QCOMPARE(rect.centerF(), QPointF(-2.5, -2));
        rect.setTop(-1);
        QCOMPARE(rect.centerF(), QPointF(-2.5, -2.5));
        rect.setLeft(-2);
        QCOMPARE(rect.centerF(), QPointF(-3, -2.5));
        rect.setTop(-2);
        QCOMPARE(rect.centerF(), QPointF(-3, -3));
    }

    // Quadrant IV
    {
        Rect rect;
        rect.setWidth(4);
        QCOMPARE(rect.centerF(), QPointF(2, 0));
        rect.setHeight(-4);
        QCOMPARE(rect.centerF(), QPointF(2, -2));
        rect.setLeft(1);
        QCOMPARE(rect.centerF(), QPointF(2.5, -2));
        rect.setTop(-1);
        QCOMPARE(rect.centerF(), QPointF(2.5, -2.5));
        rect.setLeft(2);
        QCOMPARE(rect.centerF(), QPointF(3, -2.5));
        rect.setTop(-2);
        QCOMPARE(rect.centerF(), QPointF(3, -3));
    }
}

void TestRect::setBottom()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setBottom(i);
        QCOMPARE(rect.bottom(), i);
        QCOMPARE(rect.bottomLeft(), QPoint(0, i));
        QCOMPARE(rect.bottomRight(), QPoint(0, i));
        QVERIFY(rect.top() == 0);
        QVERIFY(rect.y() == 0);
    }
}

void TestRect::setLeft()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setLeft(i);
        QCOMPARE(rect.left(), i);
        QCOMPARE(rect.x(), i);
        QCOMPARE(rect.bottomLeft(), QPoint(i, 0));
        QCOMPARE(rect.topLeft(), QPoint(i, 0));
        QVERIFY(rect.right() == 0);
    }
}

void TestRect::setX()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setX(i);
        QCOMPARE(rect.left(), i);
        QCOMPARE(rect.x(), i);
        QCOMPARE(rect.bottomLeft(), QPoint(i, 0));
        QCOMPARE(rect.topLeft(), QPoint(i, 0));
        QVERIFY(rect.right() == 0);
    }
}

void TestRect::setRight()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setRight(i);
        QCOMPARE(rect.right(), i);
        QCOMPARE(rect.topRight(), QPoint(i, 0));
        QCOMPARE(rect.bottomRight(), QPoint(i, 0));
        QVERIFY(rect.left() == 0);
        QVERIFY(rect.x() == 0);
    }
}

void TestRect::setTop()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setTop(i);
        QCOMPARE(rect.top(), i);
        QCOMPARE(rect.y(), i);
        QCOMPARE(rect.topLeft(), QPoint(0, i));
        QCOMPARE(rect.topRight(), QPoint(0, i));
        QVERIFY(rect.bottom() == 0);
    }
}

void TestRect::setY()
{
    Rect rect;

    for (int i=-50; i<50; ++i)
    {
        rect.setY(i);
        QCOMPARE(rect.top(), i);
        QCOMPARE(rect.y(), i);
        QCOMPARE(rect.topLeft(), QPoint(0, i));
        QCOMPARE(rect.topRight(), QPoint(0, i));
        QVERIFY(rect.bottom() == 0);
    }
}

void TestRect::setRect_data()
{
    QTest::addColumn<int>("x");
    QTest::addColumn<int>("y");
    QTest::addColumn<int>("w");
    QTest::addColumn<int>("h");

    QTest::newRow("(0, 0) -> (0, 0)") << 0 << 0 << 0 << 0;
    QTest::newRow("(3, 2) -> (3, 2)") << 3 << 2 << 0 << 0;
    QTest::newRow("(3, 2) :: [-1, -1]") << 3 << 2 << -1 << -1;
}

void TestRect::setRect()
{
    QFETCH(int, x);
    QFETCH(int, y);
    QFETCH(int, w);
    QFETCH(int, h);

    Rect rect;
    rect.setRect(x, y, w, h);
    QCOMPARE(rect.x(), x);
    QCOMPARE(rect.y(), y);
    QCOMPARE(rect.width(), w);
    QCOMPARE(rect.height(), h);
    QCOMPARE(rect.right(), x + w);
    QCOMPARE(rect.bottom(), y + h);
}

void TestRect::setRectPoints_data()
{
    QTest::addColumn<QPoint>("a");
    QTest::addColumn<QPoint>("b");
    QTest::addColumn<bool>("isNull");

    QTest::newRow("Null rectangle") << QPoint(0, 0) << QPoint(0, 0) << true;
    QTest::newRow("(1,1) -> (10,10)") << QPoint(1, 1) << QPoint(10, 10) << false;
    QTest::newRow("(10,1) -> (1,10)") << QPoint(10, 1) << QPoint(1, 10) << false;

    QTest::newRow("(-1,-1) -> (-10,-10)") << QPoint(-1, -1) << QPoint(-10, -10) << false;
    QTest::newRow("(-10,-1) -> (-1,-10)") << QPoint(-10, -1) << QPoint(-1, -10) << false;
}

void TestRect::setRectPoints()
{
    QFETCH(QPoint, a);
    QFETCH(QPoint, b);
    QFETCH(bool, isNull);

    // Test: can only test two quadrants

    // Quadrant I
    {
        Rect rect;
        rect.setRect(a, b);
        QCOMPARE(rect.isNull(), isNull);
        QCOMPARE(rect.x(), a.x());
        QCOMPARE(rect.y(), a.y());
        QCOMPARE(rect.right(), b.x());
        QCOMPARE(rect.bottom(), b.y());
    }

    // Quadrant IV
    {
        Rect rect;
        rect.setRect(b, a);
        QCOMPARE(rect.isNull(), isNull);
        QCOMPARE(rect.x(), b.x());
        QCOMPARE(rect.y(), b.y());
        QCOMPARE(rect.right(), a.x());
        QCOMPARE(rect.bottom(), a.y());
    }
}



void TestRect::setHeight()
{
    Rect rect;

    for (int i=-50; i< 50; ++i)
    {
        rect.setHeight(i);
        QCOMPARE(rect.height(), i);
        QVERIFY(rect.top() == 0);
        QCOMPARE(rect.bottom(), i);
        QCOMPARE(rect.size(), QSize(0, i));
    }
}

void TestRect::setWidth()
{
    Rect rect;

    for (int i=-50; i< 50; ++i)
    {
        rect.setWidth(i);
        QCOMPARE(rect.width(), i);
        QVERIFY(rect.left() == 0);
        QCOMPARE(rect.right(), i);
        QCOMPARE(rect.size(), QSize(i, 0));
    }
}

void TestRect::setSize()
{
    Rect rect(QPoint(5, 3), QPoint(6, 4));

    for (int i=-10; i< 10; ++i)
    {
        rect.setSize(QSize(i, i*2));
        QCOMPARE(rect.size(), QSize(i, i*2));
        QCOMPARE(rect.width(), i);
        QCOMPARE(rect.height(), i*2);

        QCOMPARE(rect.left(), 5);
        QCOMPARE(rect.top(), 3);
    }
}

void TestRect::containsPoint()
{
    Rect rect;

    // ------------------------------------------------------------------------
    // Test: proper = false; points on edges counts
    QCOMPARE(rect.contains(QPoint(), false), true);
    QCOMPARE(rect.contains(QPoint(1, 1), false), false);
    QCOMPARE(rect.contains(QPoint(-1, 1), false), false);

    rect.setRight(50);
    QCOMPARE(rect.contains(QPoint(), false), true);
    QCOMPARE(rect.contains(QPoint(1, 0), false), true);
    QCOMPARE(rect.contains(QPoint(50, 0), false), true);
    QCOMPARE(rect.contains(QPoint(51, 0), false), false);
    QCOMPARE(rect.contains(QPoint(-1, 0), false), false);

    rect.setBottom(50);
    for (int x=-5; x< 55; ++x)
        for (int y=-5; y< 55; ++y)
            QCOMPARE(rect.contains(QPoint(x, y), false), x >= 0 && x <= 50 && y >= 0 && y <= 50);

    // ------------------------------------------------------------------------
    // Test: proper = true; points on edges do not count
    for (int x=-5; x< 55; ++x)
        for (int y=-5; y< 55; ++y)
            QCOMPARE(rect.contains(QPoint(x, y), true), x > 0 && x < 50 && y > 0 && y < 50);
}

void TestRect::containsXY()
{
    Rect rect;

    // ------------------------------------------------------------------------
    // Test: proper = false; points on edges counts
    QCOMPARE(rect.contains(0, 0, false), true);
    QCOMPARE(rect.contains(1, 1, false), false);
    QCOMPARE(rect.contains(-1, 1, false), false);

    rect.setRight(50);
    QCOMPARE(rect.contains(0, 0, false), true);
    QCOMPARE(rect.contains(1, 0, false), true);
    QCOMPARE(rect.contains(50, 0, false), true);
    QCOMPARE(rect.contains(51, 0, false), false);
    QCOMPARE(rect.contains(-1, 0, false), false);

    rect.setBottom(50);
    for (int x=-5; x< 55; ++x)
        for (int y=-5; y< 55; ++y)
            QCOMPARE(rect.contains(x, y, false), x >= 0 && x <= 50 && y >= 0 && y <= 50);

    // ------------------------------------------------------------------------
    // Test: proper = true; points on edges do not count
    for (int x=-5; x< 55; ++x)
        for (int y=-5; y< 55; ++y)
            QCOMPARE(rect.contains(x, y, true), x > 0 && x < 50 && y > 0 && y < 50);
}

void TestRect::containsRect()
{
    Rect rect;

    // ------------------------------------------------------------------------
    // Test: proper = false; points on edges counts
    QCOMPARE(rect.contains(Rect(), false), true);
    QCOMPARE(rect.contains(Rect(1, 1, 1, 1), false), false);

    rect.setRight(10);
    for (int w=-3; w< 15; ++w)
        for (int h=-1; h<3; ++h)
            QCOMPARE(rect.contains(Rect(0, 0, w, h), false), h == 0 && w >= 0 && w <= 10);

    rect.setBottom(10);
    for (int w=0; w< 11; ++w)
        for (int h=0; h< 11; ++h)
            for (int x=-5; x<10; ++x)
                for (int y=-5; y<10; ++y)
                    QCOMPARE(rect.contains(Rect(x, y, w, h), false),
                             (x >= 0 && x + w <= 10) &&
                             (y >= 0 && y + h <= 10));

    // ------------------------------------------------------------------------
    // Test: proper = true; points on edges do not count
    QCOMPARE(rect.contains(Rect(), true), false);
    rect.setBottom(2);
    rect.setRight(2);

    QCOMPARE(rect.contains(Rect(0, 0, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(0, 1, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(0, 2, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(1, 0, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(1, 2, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(2, 0, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(2, 1, 0, 0), true), false);
    QCOMPARE(rect.contains(Rect(2, 2, 0, 0), true), false);

    QCOMPARE(rect.contains(Rect(1, 1, 0, 0), true), true);

    // ------------------------------------------------------------------------
    // Test: smaller inside larger and vice versa
    QCOMPARE(Rect(0, 0, 10, 10).contains(Rect(2, 2, 5, 5)), true);
    QCOMPARE(Rect(2, 2, 5, 5).contains(Rect(0, 0, 10, 10)), false);
}

void TestRect::intersects()
{
    Rect rect;

    // ------------------------------------------------------------------------
    // Test: Two null rectangles
    QCOMPARE(rect.intersects(Rect()), false);

    // ------------------------------------------------------------------------
    // Test: one null rectangle and one non-null
    rect.setWidth(1);
    QCOMPARE(rect.intersects(Rect()), false);

    // ------------------------------------------------------------------------
    // Test: two null rectangles with one dimension non-null
    QCOMPARE(rect.intersects(Rect(QPoint(1, 0), QPoint(2, 0))), false);

    // ------------------------------------------------------------------------
    // Test: Two non-null, completely overlapping rectangles
    QCOMPARE(Rect(0, 0, 1, 1).intersects(Rect(0, 0, 1, 1)), true);

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersects(Rect(QPoint(10, 10), QPoint(12, 0))), false);

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersects(Rect(QPoint(0, 10), QPoint(10, 12))), false);

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersects(Rect(QPoint(10, 10), QPoint(12, 12))), false);

    // ------------------------------------------------------------------------
    // Test: overlap by one unit and vice versa
    QCOMPARE(Rect(0, 0, 1, 1).intersects(Rect(0, 0, 2, 1)), true);
    QCOMPARE(Rect(0, 0, 2, 1).intersects(Rect(0, 0, 1, 1)), true);

    QCOMPARE(Rect(0, 0, 1, 1).intersects(Rect(QPoint(0, -1), QPoint(1, 1))), true);
    QCOMPARE(Rect(QPoint(0, -1), QPoint(1, 1)).intersects(Rect(0, 0, 1, 1)), true);

    QCOMPARE(Rect(0, 0, 1, 1).intersects(Rect(QPoint(-1, 0), QPoint(1, 1))), true);
    QCOMPARE(Rect(QPoint(-1, 0), QPoint(1, 1)).intersects(Rect(0, 0, 1, 1)), true);

    QCOMPARE(Rect(0, 0, 1, 1).intersects(Rect(0, 0, 1, 2)), true);
    QCOMPARE(Rect(0, 0, 1, 2).intersects(Rect(0, 0, 1, 1)), true);

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(Rect(QPoint(2, 2), QPoint(4, 4)).intersects(Rect(0, 0, 10, 10)), true);
    QCOMPARE(Rect(0, 0, 10, 10).intersects(Rect(QPoint(2, 2), QPoint(4, 4))), true);

    // ------------------------------------------------------------------------
    // Test: self intersects
    rect.setRect(QPoint(-2, 2), QPoint(2, -2));
    QCOMPARE(rect.intersects(rect), true);

    // ------------------------------------------------------------------------
    // Test: corners

    // Top left
    QCOMPARE(rect.intersects(Rect(QPoint(-5, 5), QPoint(-1, 1))), true);
    QCOMPARE(Rect(QPoint(-5, 5), QPoint(-1, 1)).intersects(rect), true);

    // Top right
    QCOMPARE(rect.intersects(Rect(QPoint(5, 5), QPoint(1, 1))), true);
    QCOMPARE(Rect(QPoint(5, 5), QPoint(1, 1)).intersects(rect), true);

    // Bottom right
    QCOMPARE(rect.intersects(Rect(QPoint(5, -5), QPoint(1, -1))), true);
    QCOMPARE(Rect(QPoint(5, -5), QPoint(1, -1)).intersects(rect), true);

    // Bottom left
    QCOMPARE(rect.intersects(Rect(QPoint(-5, -5), QPoint(-1, -1))), true);
    QCOMPARE(Rect(QPoint(-5, -5), QPoint(-1, -1)).intersects(rect), true);

    // ------------------------------------------------------------------------
    // Test: sides

    // Left
    QCOMPARE(rect.intersects(Rect(QPoint(-4, 4), QPoint(-1, -4))), true);
    QCOMPARE(Rect(QPoint(-4, 4), QPoint(-1, -4)).intersects(rect), true);

    // Top
    QCOMPARE(rect.intersects(Rect(QPoint(-4, 4), QPoint(4, 1))), true);
    QCOMPARE(Rect(QPoint(-4, 4), QPoint(4, 1)).intersects(rect), true);

    // Right
    QCOMPARE(rect.intersects(Rect(QPoint(4, 4), QPoint(1, -4))), true);
    QCOMPARE(Rect(QPoint(4, 4), QPoint(1, -4)).intersects(rect), true);

    // Bottom
    QCOMPARE(rect.intersects(Rect(QPoint(4, -4), QPoint(-4, -1))), true);
    QCOMPARE(Rect(QPoint(4, -4), QPoint(-4, -1)).intersects(rect), true);
}

void TestRect::intersection()
{
    Rect rect;

    // ------------------------------------------------------------------------
    // Test: Two null rectangles
    QCOMPARE(rect.intersection(Rect()), Rect());

    // ------------------------------------------------------------------------
    // Test: one null rectangle and one non-null
    rect.setWidth(1);
    QCOMPARE(rect.intersection(Rect()), Rect());

    // ------------------------------------------------------------------------
    // Test: two null rectangles with one dimension non-null
    QCOMPARE(rect.intersection(Rect(QPoint(1, 0), QPoint(2, 0))), Rect());

    // ------------------------------------------------------------------------
    // Test: Two non-null, completely overlapping rectangles
    QCOMPARE(Rect(0, 0, 1, 1).intersection(Rect(0, 0, 1, 1)), Rect(0, 0, 1, 1));

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersection(Rect(QPoint(10, 10), QPoint(12, 0))), Rect());

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersection(Rect(QPoint(0, 10), QPoint(10, 12))), Rect());

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(Rect(0, 0, 10, 10).intersection(Rect(QPoint(10, 10), QPoint(12, 12))), Rect());

    // ------------------------------------------------------------------------
    // Test: overlap by one unit and vice versa
    QCOMPARE(Rect(0, 0, 1, 1).intersection(Rect(0, 0, 2, 1)), Rect(0, 0, 1, 1));
    QCOMPARE(Rect(0, 0, 2, 1).intersection(Rect(0, 0, 1, 1)), Rect(0, 0, 1, 1));

    QCOMPARE(Rect(0, 0, 1, 1).intersection(Rect(QPoint(0, -1), QPoint(1, 1))), Rect(0, 0, 1, 1));
    QCOMPARE(Rect(QPoint(0, -1), QPoint(1, 1)).intersection(Rect(0, 0, 1, 1)), Rect(0, 0, 1, 1));

    QCOMPARE(Rect(0, 0, 1, 1).intersection(Rect(QPoint(-1, 0), QPoint(1, 1))), Rect(0, 0, 1, 1));
    QCOMPARE(Rect(QPoint(-1, 0), QPoint(1, 1)).intersection(Rect(0, 0, 1, 1)), Rect(0, 0, 1, 1));

    QCOMPARE(Rect(0, 0, 1, 1).intersection(Rect(0, 0, 1, 2)), Rect(0, 0, 1, 1));
    QCOMPARE(Rect(0, 0, 1, 2).intersection(Rect(0, 0, 1, 1)), Rect(0, 0, 1, 1));

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(Rect(QPoint(2, 2), QPoint(4, 4)).intersection(Rect(0, 0, 10, 10)), Rect(QPoint(2, 2), QPoint(4, 4)));
    QCOMPARE(Rect(0, 0, 10, 10).intersection(Rect(QPoint(2, 2), QPoint(4, 4))), Rect(QPoint(2, 2), QPoint(4, 4)));

    // ------------------------------------------------------------------------
    // Test: self intersection
    rect.setRect(QPoint(-2, 2), QPoint(2, -2));
    QCOMPARE(rect.intersection(rect), rect.normalized());

    // ------------------------------------------------------------------------
    // Test: corners

    // Top left
    QCOMPARE(rect.intersection(Rect(QPoint(-5, 5), QPoint(-1, 1))), Rect(QPoint(-2, 1), QPoint(-1, 2)));
    QCOMPARE(Rect(QPoint(-5, 5), QPoint(-1, 1)).intersection(rect), Rect(QPoint(-2, 1), QPoint(-1, 2)));

    // Top right
    QCOMPARE(rect.intersection(Rect(QPoint(5, 5), QPoint(1, 1))), Rect(QPoint(2, 2), QPoint(1, 1)).normalized());
    QCOMPARE(Rect(QPoint(5, 5), QPoint(1, 1)).intersection(rect), Rect(QPoint(2, 2), QPoint(1, 1)).normalized());

    // Bottom right
    QCOMPARE(rect.intersection(Rect(QPoint(5, -5), QPoint(1, -1))), Rect(QPoint(2, -2), QPoint(1, -1)).normalized());
    QCOMPARE(Rect(QPoint(5, -5), QPoint(1, -1)).intersection(rect), Rect(QPoint(2, -2), QPoint(1, -1)).normalized());

    // Bottom left
    QCOMPARE(rect.intersection(Rect(QPoint(-5, -5), QPoint(-1, -1))), Rect(QPoint(-2, -2), QPoint(-1, -1)).normalized());
    QCOMPARE(Rect(QPoint(-5, -5), QPoint(-1, -1)).intersection(rect), Rect(QPoint(-2, -2), QPoint(-1, -1)).normalized());

    // ------------------------------------------------------------------------
    // Test: sides

    // Left
    QCOMPARE(rect.intersection(Rect(QPoint(-4, 4), QPoint(-1, -4))), Rect(QPoint(-2, 2), QPoint(-1, -2)).normalized());
    QCOMPARE(Rect(QPoint(-4, 4), QPoint(-1, -4)).intersection(rect), Rect(QPoint(-2, 2), QPoint(-1, -2)).normalized());

    // Top
    QCOMPARE(rect.intersection(Rect(QPoint(-4, 4), QPoint(4, 1))), Rect(QPoint(-2, 2), QPoint(2, 1)).normalized());
    QCOMPARE(Rect(QPoint(-4, 4), QPoint(4, 1)).intersection(rect), Rect(QPoint(-2, 2), QPoint(2, 1)).normalized());

    // Right
    QCOMPARE(rect.intersection(Rect(QPoint(4, 4), QPoint(1, -4))), Rect(QPoint(2, 2), QPoint(1, -2)).normalized());
    QCOMPARE(Rect(QPoint(4, 4), QPoint(1, -4)).intersection(rect), Rect(QPoint(2, 2), QPoint(1, -2)).normalized());

    // Bottom
    QCOMPARE(rect.intersection(Rect(QPoint(4, -4), QPoint(-4, -1))), Rect(QPoint(-2, -1), QPoint(2, -2)).normalized());
    QCOMPARE(Rect(QPoint(4, -4), QPoint(-4, -1)).intersection(rect), Rect(QPoint(-2, -1), QPoint(2, -2)).normalized());
}

void TestRect::moveLeft()
{
    Rect rect(0, 0, 5, 5);

    for (int i=-5; i< 5; ++i)
    {
        rect.moveLeft(i);
        QCOMPARE(rect.left(), i);
        QCOMPARE(rect.width(), 5);
        QCOMPARE(rect.right(), i+5);
    }
}

void TestRect::moveRight()
{
    Rect rect(0, 0, 5, 5);

    for (int i=-5; i< 5; ++i)
    {
        rect.moveRight(i);
        QCOMPARE(rect.right(), i);
        QCOMPARE(rect.width(), 5);
        QCOMPARE(rect.left(), i-5);
    }
}

void TestRect::moveTop()
{
    Rect rect(0, 0, 5, 5);

    for (int i=-5; i< 5; ++i)
    {
        rect.moveTop(i);
        QCOMPARE(rect.top(), i);
        QCOMPARE(rect.height(), 5);
        QCOMPARE(rect.bottom(), i+5);
    }
}

void TestRect::moveBottom()
{
    Rect rect(0, 0, 5, 5);

    for (int i=-5; i< 5; ++i)
    {
        rect.moveBottom(i);
        QCOMPARE(rect.bottom(), i);
        QCOMPARE(rect.height(), 5);
        QCOMPARE(rect.top(), i-5);
    }
}

void TestRect::normalized()
{
    {
        Rect rect;

        QVERIFY(rect.normalized() == rect);

        rect.setWidth(1);
        rect.setHeight(1);

        QVERIFY(rect.normalized() == rect);
    }

    {
        Rect rect(QPoint(1, 1), QPoint(3, 3));
        QCOMPARE(rect.normalized(), rect);
    }

    {
        Rect rect(QPoint(3, 3), QPoint(1, 1));
        QCOMPARE(rect.width(), -2);
        QCOMPARE(rect.height(), -2);
        rect = rect.normalized();

        QCOMPARE(rect.normalized(), Rect(QPoint(1, 1), QPoint(3, 3)));
    }
}

void TestRect::toQRectF()
{
    Rect r;
    QCOMPARE(r.toQRectF(), QRectF());

    for (int i=-5; i< 5; ++i)
    {
        r.setLeft(i);
        r.setTop(i);
        QCOMPARE(r.toQRectF(), QRectF(QPointF(i, i), QPointF()));
    }

    r.setTop(0);
    r.setLeft(0);

    for (int i=-5; i< 5; ++i)
    {
        r.setRight(i);
        r.setBottom(i);
        QCOMPARE(r.toQRectF(), QRectF(QPointF(), QPointF(i, i)));
    }
}

QTEST_APPLESS_MAIN(TestRect)
#include "TestRect.moc"
