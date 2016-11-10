/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../PosiRect.h"
#include "../ClosedIntRange.h"

class TestPosiRect : public QObject
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

    void isNull();

    void horizontalRange();
    void verticalRange();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPosiRect::constructorPointSize()
{
    {
        PosiRect x(QPoint(1, 1), QSize(1, 1));
        QVERIFY(x.left() == x.right());
        QVERIFY(x.top() == x.bottom());
        QCOMPARE(x.width(), 1);
        QCOMPARE(x.height(), 1);
    }

    {
        PosiRect x(QPoint(1, 1), QSize(3, 2));
        QCOMPARE(x.right(), 3);
        QCOMPARE(x.bottom(), 2);
        QCOMPARE(x.width(), 3);
        QCOMPARE(x.height(), 2);
    }
}

void TestPosiRect::constructorXYWH()
{
    {
        PosiRect x(1, 1, 1, 1);
        QVERIFY(x.left() == x.right());
        QVERIFY(x.top() == x.bottom());
        QCOMPARE(x.width(), 1);
        QCOMPARE(x.height(), 1);
    }

    {
        PosiRect x(1, 1, 3, 2);
        QCOMPARE(x.right(), 3);
        QCOMPARE(x.bottom(), 2);
        QCOMPARE(x.width(), 3);
        QCOMPARE(x.height(), 2);
    }
}

void TestPosiRect::isValid()
{
    {
        PosiRect x;

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
        PosiRect x(1, 1, 1, 1);
        QCOMPARE(x.isValid(), true);
    }
}

void TestPosiRect::setHeight()
{
    PosiRect x(1, 1, 1, 1);
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

void TestPosiRect::setWidth()
{
    PosiRect x(1, 1, 1, 1);
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

void TestPosiRect::setRect()
{
    PosiRect x(1, 1, 1, 1);
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

void TestPosiRect::setSize()
{
    PosiRect x(1, 1, 1, 1);
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

void TestPosiRect::normalized()
{
    {
        PosiRect x;

        QVERIFY(x.normalized() == x);

        x.setWidth(1);
        x.setHeight(1);

        QVERIFY(x.normalized() == x);
    }

    {
        PosiRect x(QPoint(1, 1), QPoint(3, 3));
        QCOMPARE(x.normalized(), x);
    }

    {
        PosiRect x(QPoint(3, 3), QPoint(1, 1));
        QCOMPARE(x.width(), -3);
        QCOMPARE(x.height(), -3);
        x = x.normalized();

        QCOMPARE(x.normalized(), PosiRect(QPoint(1, 1), QPoint(3, 3)));
    }
}

void TestPosiRect::intersects()
{
    PosiRect x;

    // ------------------------------------------------------------------------
    // Test: Two invalid rectangles
    QCOMPARE(x.intersects(PosiRect()), false);
    x.setX(1);
    QCOMPARE(x.intersects(PosiRect()), false);
    x.setY(1);
    QCOMPARE(x.intersects(PosiRect()), false);
    QVERIFY(x.isValid() == false);

    // ------------------------------------------------------------------------
    // Test: one valid rectangle and one invalid
    x.setWidth(1);
    x.setHeight(1);
    QVERIFY(x.isValid());
    QCOMPARE(x.intersects(PosiRect()), false);

    // ------------------------------------------------------------------------
    // Test: Two valid rectangles that do not intersect
    x.setRect(QPoint(3, 3), QPoint(4, 4));
    QCOMPARE(x.intersects(PosiRect(5, 5, 1, 1)), false);
    QCOMPARE(x.intersects(PosiRect(2, 2, 1, 1)), false);
    QCOMPARE(x.intersects(PosiRect(5, 2, 1, 1)), false);
    QCOMPARE(x.intersects(PosiRect(2, 5, 1, 1)), false);

    // ------------------------------------------------------------------------
    // Test: self intersects
    QCOMPARE(x.intersects(x), true);

    // ------------------------------------------------------------------------
    // Test: One unit overlaps
    QCOMPARE(x.intersects(PosiRect(QPoint(2, 2), QPoint(3, 3))), true);
    QCOMPARE(x.intersects(PosiRect(QPoint(4, 3), QPoint(5, 2))), true);
    QCOMPARE(x.intersects(PosiRect(QPoint(3, 4), QPoint(2, 5))), true);
    QCOMPARE(x.intersects(PosiRect(QPoint(4, 4), QPoint(5, 5))), true);

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersects(PosiRect(QPoint(1, 10), QPoint(10, 12))), true);

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersects(PosiRect(QPoint(10, 10), QPoint(12, 1))), true);

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersects(PosiRect(QPoint(10, 10), QPoint(12, 12))), true);

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(PosiRect(QPoint(2, 2), QPoint(4, 4)).intersects(PosiRect(1, 1, 10, 10)), true);
    QCOMPARE(PosiRect(1, 1, 10, 10).intersects(PosiRect(QPoint(2, 2), QPoint(4, 4))), true);
}

void TestPosiRect::intersection()
{
    PosiRect x;

    // ------------------------------------------------------------------------
    // Test: Two invalid rectangles
    QCOMPARE(x.intersection(PosiRect()), PosiRect());
    x.setX(1);
    QCOMPARE(x.intersection(PosiRect()), PosiRect());
    x.setY(1);
    QCOMPARE(x.intersection(PosiRect()), PosiRect());
    QVERIFY(x.isValid() == false);

    // ------------------------------------------------------------------------
    // Test: one valid rectangle and one invalid
    x.setWidth(1);
    x.setHeight(1);
    QVERIFY(x.isValid());
    QCOMPARE(x.intersection(PosiRect()), PosiRect());

    // ------------------------------------------------------------------------
    // Test: Two valid rectangles that do not intersect
    x.setRect(QPoint(3, 3), QPoint(4, 4));
    QCOMPARE(x.intersection(PosiRect(5, 5, 1, 1)), PosiRect());
    QCOMPARE(x.intersection(PosiRect(2, 2, 1, 1)), PosiRect());
    QCOMPARE(x.intersection(PosiRect(5, 2, 1, 1)), PosiRect());
    QCOMPARE(x.intersection(PosiRect(2, 5, 1, 1)), PosiRect());

    // ------------------------------------------------------------------------
    // Test: self intersection
    QCOMPARE(x.intersection(x), x.normalized());

    // ------------------------------------------------------------------------
    // Test: One unit overlaps
    QCOMPARE(x.intersection(PosiRect(QPoint(2, 2), QPoint(3, 3))), PosiRect(3, 3, 1, 1));
    QCOMPARE(x.intersection(PosiRect(QPoint(4, 3), QPoint(5, 2))), PosiRect(4, 3, 1, 1));
    QCOMPARE(x.intersection(PosiRect(QPoint(3, 4), QPoint(2, 5))), PosiRect(3, 4, 1, 1));
    QCOMPARE(x.intersection(PosiRect(QPoint(4, 4), QPoint(5, 5))), PosiRect(4, 4, 1, 1));

    // ------------------------------------------------------------------------
    // Test: Two horizontally touching rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersection(PosiRect(QPoint(1, 10), QPoint(10, 12))), PosiRect(QPoint(1, 10), QPoint(10, 10)));

    // ------------------------------------------------------------------------
    // Test: Two vertically touching non-null rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersection(PosiRect(QPoint(10, 10), QPoint(12, 1))), PosiRect(QPoint(10, 1), QPoint(10, 10)));

    // ------------------------------------------------------------------------
    // Test: Two diagonally touching non-null rectangles
    QCOMPARE(PosiRect(1, 1, 10, 10).intersection(PosiRect(QPoint(10, 10), QPoint(12, 12))), PosiRect(10, 10, 1, 1));

    // ------------------------------------------------------------------------
    // Test: rectangle inside rectangle
    QCOMPARE(PosiRect(QPoint(2, 2), QPoint(4, 4)).intersection(PosiRect(1, 1, 10, 10)), PosiRect(QPoint(2, 2), QPoint(4, 4)));
    QCOMPARE(PosiRect(1, 1, 10, 10).intersection(PosiRect(QPoint(2, 2), QPoint(4, 4))), PosiRect(QPoint(2, 2), QPoint(4, 4)));

    // ------------------------------------------------------------------------
    // Test: one unit inside rectangle
    QCOMPARE(PosiRect(1, 1, 10, 10).intersection(PosiRect(3, 3, 1, 1)), PosiRect(3, 3, 1, 1));
}

void TestPosiRect::isNull()
{
    PosiRect x(QPoint(1, 1), QPoint(1, 1));

    QVERIFY(x.isNull() == false);
    x.setBottom(2);
    QVERIFY(x.isNull() == false);
    x.setTop(2);
    QVERIFY(x.isNull() == false);

    x = PosiRect();
    QVERIFY(x.isNull());
}

void TestPosiRect::horizontalRange()
{
    PosiRect x;

    QCOMPARE(x.horizontalRange(), ClosedIntRange(0, 0));
    x.setRight(10);
    QCOMPARE(x.horizontalRange(), ClosedIntRange(0, 10));
    x.setLeft(5);
    QCOMPARE(x.horizontalRange(), ClosedIntRange(5, 10));
    x.setLeft(15);
    QCOMPARE(x.horizontalRange(), ClosedIntRange(15, 10));
}

void TestPosiRect::verticalRange()
{
    PosiRect x;

    QCOMPARE(x.verticalRange(), ClosedIntRange(0, 0));
    x.setBottom(10);
    QCOMPARE(x.verticalRange(), ClosedIntRange(0, 10));
    x.setTop(5);
    QCOMPARE(x.verticalRange(), ClosedIntRange(5, 10));
    x.setTop(15);
    QCOMPARE(x.verticalRange(), ClosedIntRange(15, 10));
}

QTEST_APPLESS_MAIN(TestPosiRect)
#include "TestPosiRect.moc"
