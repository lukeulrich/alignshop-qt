/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "MsaRect.h"

class TestMsaRect : public QObject
{
    Q_OBJECT

private slots:
    void constructorPointSize();
    void constructorXYWH();
    void isValid();
    void setHeight();       // Also tests height
    void setWidth();        // Also tests width
    void setRect();
    void setSize();         // Also tests size
    void normalized();

    void intersects();
    void intersection();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestMsaRect::constructorPointSize()
{
    {
        MsaRect x(QPoint(1, 1), QSize(1, 1));
        QVERIFY(x.left() == x.right());
        QVERIFY(x.top() == x.bottom());
        QCOMPARE(x.width(), 1);
        QCOMPARE(x.height(), 1);
    }

    {
        MsaRect x(QPoint(1, 1), QSize(3, 2));
        QCOMPARE(x.right(), 3);
        QCOMPARE(x.bottom(), 2);
        QCOMPARE(x.width(), 3);
        QCOMPARE(x.height(), 2);
    }
}

void TestMsaRect::constructorXYWH()
{
    {
        MsaRect x(1, 1, 1, 1);
        QVERIFY(x.left() == x.right());
        QVERIFY(x.top() == x.bottom());
        QCOMPARE(x.width(), 1);
        QCOMPARE(x.height(), 1);
    }

    {
        MsaRect x(1, 1, 3, 2);
        QCOMPARE(x.right(), 3);
        QCOMPARE(x.bottom(), 2);
        QCOMPARE(x.width(), 3);
        QCOMPARE(x.height(), 2);
    }
}

void TestMsaRect::isValid()
{
    {
        MsaRect x;

        QCOMPARE(x.isValid(), false);
        x.setLeft(1);
        QCOMPARE(x.isValid(), false);
        x.setTop(1);
        QCOMPARE(x.isValid(), false);
        x.setRight(1);
        QCOMPARE(x.isValid(), false);
        x.setBottom(1);
        QCOMPARE(x.isValid(), true);
        x.setLeft(0);
        QCOMPARE(x.isValid(), false);
        x.setTop(0);
        QCOMPARE(x.isValid(), false);
    }

    {
        MsaRect x(1, 1, 1, 1);
        QCOMPARE(x.isValid(), true);
    }
}

void TestMsaRect::setHeight()
{
    MsaRect x(1, 1, 1, 1);
    QCOMPARE(x.height(), 1);

    x.setTop(2);
    QCOMPARE(x.height(), -2);

    x.setTop(1);
    x.setHeight(5);
    QCOMPARE(x.height(), 5);
    QCOMPARE(x.bottom(), 5);

    x.setTop(5);
    QCOMPARE(x.height(), 1);
    x.setHeight(-5);
    QCOMPARE(x.height(), -5);
    QCOMPARE(x.bottom(), 1);
}

void TestMsaRect::setWidth()
{
    MsaRect x(1, 1, 1, 1);
    QCOMPARE(x.width(), 1);

    x.setLeft(2);
    QCOMPARE(x.width(), -2);

    x.setLeft(1);
    x.setWidth(5);
    QCOMPARE(x.width(), 5);
    QCOMPARE(x.right(), 5);

    x.setLeft(5);
    QCOMPARE(x.width(), 1);
    x.setWidth(-5);
    QCOMPARE(x.width(), -5);
    QCOMPARE(x.right(), 1);
}

void TestMsaRect::setRect()
{
    MsaRect x(1, 1, 1, 1);
    QCOMPARE(x.size(), QSize(1, 1));

    x.setRect(2, 2, -2, -2);
    QCOMPARE(x.size(), QSize(-2, -2));

    x.setRect(1, 1, 5, 5);
    QCOMPARE(x.size(), QSize(5, 5));
    QCOMPARE(x.bottom(), 5);
    QCOMPARE(x.right(), 5);

    x.setRect(5, 5, 1, 1);
    QCOMPARE(x.size(), QSize(1, 1));
    x.setRect(5, 5, -5, -5);
    QCOMPARE(x.size(), QSize(-5, -5));
    QCOMPARE(x.bottom(), 1);
    QCOMPARE(x.right(), 1);
}

void TestMsaRect::setSize()
{
    MsaRect x(1, 1, 1, 1);
    QCOMPARE(x.size(), QSize(1, 1));

    x.setTop(2);
    x.setLeft(2);
    QCOMPARE(x.size(), QSize(-2, -2));

    x.setTop(1);
    x.setHeight(5);
    x.setLeft(1);
    x.setWidth(5);
    QCOMPARE(x.size(), QSize(5, 5));
    QCOMPARE(x.bottom(), 5);
    QCOMPARE(x.right(), 5);

    x.setTop(5);
    x.setLeft(5);
    QCOMPARE(x.size(), QSize(1, 1));
    x.setHeight(-5);
    x.setWidth(-5);
    QCOMPARE(x.size(), QSize(-5, -5));
    QCOMPARE(x.bottom(), 1);
    QCOMPARE(x.right(), 1);
}

void TestMsaRect::normalized()
{
    {
        MsaRect x;

        QVERIFY(x.normalized() == x);

        x.setWidth(1);
        x.setHeight(1);

        QVERIFY(x.normalized() == x);
    }

    {
        MsaRect x(QPoint(1, 1), QPoint(3, 3));
        QCOMPARE(x.normalized(), x);
    }

    {
        MsaRect x(QPoint(3, 3), QPoint(1, 1));
        QCOMPARE(x.width(), -3);
        QCOMPARE(x.height(), -3);
        x = x.normalized();

        QCOMPARE(x.normalized(), MsaRect(QPoint(1, 1), QPoint(3, 3)));
    }
}

void TestMsaRect::intersects()
{
    MsaRect x;

    // ------------------------------------------------------------------------
    // Test: Two invalid rectangles
    QCOMPARE(x.intersects(MsaRect()), false);
    x.setX(1);
    QCOMPARE(x.intersects(MsaRect()), false);
    x.setY(1);
    QCOMPARE(x.intersects(MsaRect()), false);
    QVERIFY(x.isValid() == false);

    // ------------------------------------------------------------------------
    // Test: one valid rectangle and one invalid
    x.setWidth(1);
    x.setHeight(1);
    QVERIFY(x.isValid());
    QCOMPARE(x.intersects(MsaRect()), false);

    // ------------------------------------------------------------------------
    // Test: Two valid rectangles that do not intersect
    x.setRect(QPoint(3, 3), QPoint(4, 4));
    QCOMPARE(x.intersects(MsaRect(5, 5, 1, 1)), false);
    QCOMPARE(x.intersects(MsaRect(2, 2, 1, 1)), false);
    QCOMPARE(x.intersects(MsaRect(5, 2, 1, 1)), false);
    QCOMPARE(x.intersects(MsaRect(2, 5, 1, 1)), false);

    // ------------------------------------------------------------------------
    // Test: self intersects
    QCOMPARE(x.intersects(x), true);

    // ------------------------------------------------------------------------
    // Test: One unit overlaps
    QCOMPARE(x.intersects(MsaRect(QPoint(2, 2), QPoint(3, 3))), true);
    QCOMPARE(x.intersects(MsaRect(QPoint(4, 3), QPoint(5, 2))), true);
    QCOMPARE(x.intersects(MsaRect(QPoint(3, 4), QPoint(2, 5))), true);
    QCOMPARE(x.intersects(MsaRect(QPoint(4, 4), QPoint(5, 5))), true);

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersects(MsaRect(QPoint(1, 10), QPoint(10, 12))), true);

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersects(MsaRect(QPoint(10, 10), QPoint(12, 1))), true);

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersects(MsaRect(QPoint(10, 10), QPoint(12, 12))), true);

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(MsaRect(QPoint(2, 2), QPoint(4, 4)).intersects(MsaRect(1, 1, 10, 10)), true);
    QCOMPARE(MsaRect(1, 1, 10, 10).intersects(MsaRect(QPoint(2, 2), QPoint(4, 4))), true);
}

void TestMsaRect::intersection()
{
    MsaRect x;

    // ------------------------------------------------------------------------
    // Test: Two invalid rectangles
    QCOMPARE(x.intersection(MsaRect()), MsaRect());
    x.setX(1);
    QCOMPARE(x.intersection(MsaRect()), MsaRect());
    x.setY(1);
    QCOMPARE(x.intersection(MsaRect()), MsaRect());
    QVERIFY(x.isValid() == false);

    // ------------------------------------------------------------------------
    // Test: one valid rectangle and one invalid
    x.setWidth(1);
    x.setHeight(1);
    QVERIFY(x.isValid());
    QCOMPARE(x.intersection(MsaRect()), MsaRect());

    // ------------------------------------------------------------------------
    // Test: Two valid rectangles that do not intersect
    x.setRect(QPoint(3, 3), QPoint(4, 4));
    QCOMPARE(x.intersection(MsaRect(5, 5, 1, 1)), MsaRect());
    QCOMPARE(x.intersection(MsaRect(2, 2, 1, 1)), MsaRect());
    QCOMPARE(x.intersection(MsaRect(5, 2, 1, 1)), MsaRect());
    QCOMPARE(x.intersection(MsaRect(2, 5, 1, 1)), MsaRect());

    // ------------------------------------------------------------------------
    // Test: self intersection
    QCOMPARE(x.intersection(x), x.normalized());

    // ------------------------------------------------------------------------
    // Test: One unit overlaps
    QCOMPARE(x.intersection(MsaRect(QPoint(2, 2), QPoint(3, 3))), MsaRect(3, 3, 1, 1));
    QCOMPARE(x.intersection(MsaRect(QPoint(4, 3), QPoint(5, 2))), MsaRect(4, 3, 1, 1));
    QCOMPARE(x.intersection(MsaRect(QPoint(3, 4), QPoint(2, 5))), MsaRect(3, 4, 1, 1));
    QCOMPARE(x.intersection(MsaRect(QPoint(4, 4), QPoint(5, 5))), MsaRect(4, 4, 1, 1));

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersection(MsaRect(QPoint(1, 10), QPoint(10, 12))), MsaRect(QPoint(1, 10), QPoint(10, 10)));

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersection(MsaRect(QPoint(10, 10), QPoint(12, 1))), MsaRect(QPoint(10, 1), QPoint(10, 10)));

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(MsaRect(1, 1, 10, 10).intersection(MsaRect(QPoint(10, 10), QPoint(12, 12))), MsaRect(10, 10, 1, 1));

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(MsaRect(QPoint(2, 2), QPoint(4, 4)).intersection(MsaRect(1, 1, 10, 10)), MsaRect(QPoint(2, 2), QPoint(4, 4)));
    QCOMPARE(MsaRect(1, 1, 10, 10).intersection(MsaRect(QPoint(2, 2), QPoint(4, 4))), MsaRect(QPoint(2, 2), QPoint(4, 4)));

    // ------------------------------------------------------------------------
    // Test: one unit inside rectangle
    QCOMPARE(MsaRect(1, 1, 10, 10).intersection(MsaRect(3, 3, 1, 1)), MsaRect(3, 3, 1, 1));
}

QTEST_APPLESS_MAIN(TestMsaRect)
#include "TestMsaRect.moc"
