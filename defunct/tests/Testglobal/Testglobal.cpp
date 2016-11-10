/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "global.h"

class Testglobal : public QObject
{
    Q_OBJECT

private slots:
    void randomInteger();
    void isGapCharacter();
    void divideListHashCharInt_data();
    void divideListHashCharInt();
    void floorPoint();
    void boundingRect();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void Testglobal::randomInteger()
{
    // ------------------------------------------------------------------------
    // Test: Walk each element of a diagonal 1000x1000 matrix and test that randomInteger
    //       returns a value between these bounds
    for (int i=0; i< 1000; ++i)
    {
        for (int j=i+1; j< 1000; ++j)
        {
            int x = ::randomInteger(i, j);
            QVERIFY(x >= i && x <= j);
        }
    }
}

void Testglobal::isGapCharacter()
{
    // ------------------------------------------------------------------------
    // Test: loop through all characters and test whether they should be gaps or not
    for (int i=0; i< 256; ++i)
    {
        char ch = i;
        bool shouldBeGap = false;
        const char *x = constants::kGapCharacters;
        while (*x != '\0')
        {
            if (ch == *x)
            {
                shouldBeGap = true;
                break;
            }

            ++x;
        }

        QCOMPARE(::isGapCharacter(ch), shouldBeGap);
        QCOMPARE(::isGapCharacter(QChar(ch)), shouldBeGap);
    }
}

void Testglobal::divideListHashCharInt_data()
{
    QTest::addColumn<ListHashCharInt>("listHashCharInt");
    QTest::addColumn<int>("divisor");
    QTest::addColumn<ListHashCharDouble>("expectedResult");

    // i -> # of columns in listHashCharInt
    for (int divisor=-10; divisor <=10; ++divisor)
    {
        if (divisor == 0)
            continue;

        for (int i=1; i<=3; ++i)
        {
            ListHashCharInt listHashCharInt;
            ListHashCharDouble listHashCharDouble;
            for (int j=0; j< i; ++j)
            {
                listHashCharInt << QHash<char, int>();
                listHashCharDouble << QHash<char, qreal>();
                int nMax = ::randomInteger(1, 5);
                for (int n=-nMax; n<nMax; ++n)
                {
                    char ch = 'A' + n + nMax;
                    listHashCharInt.last().insert(ch, n);
                    listHashCharDouble.last().insert(ch, static_cast<qreal>(n) / static_cast<qreal>(divisor));
                }
            }

            QTest::newRow(QString("Columns: %1, Divisor: %2").arg(i).arg(divisor).toAscii())
                    << listHashCharInt
                    << divisor
                    << listHashCharDouble;
        }
    }
}

void Testglobal::divideListHashCharInt()
{
    QFETCH(ListHashCharInt, listHashCharInt);
    QFETCH(int, divisor);
    QFETCH(ListHashCharDouble, expectedResult);

    QCOMPARE(::divideListHashCharInt(listHashCharInt, divisor), expectedResult);
}

void Testglobal::floorPoint()
{
    QCOMPARE(::floorPoint(QPointF(.5, 0)), QPoint(0, 0));
    QCOMPARE(::floorPoint(QPointF(0, .5)), QPoint(0, 0));
    QCOMPARE(::floorPoint(QPointF(.5, .5)), QPoint(0, 0));

    QCOMPARE(::floorPoint(QPointF(5.3, 0)), QPoint(5, 0));
    QCOMPARE(::floorPoint(QPointF(0, 5.3)), QPoint(0, 5));
    QCOMPARE(::floorPoint(QPointF(5.3, 5.3)), QPoint(5, 5));

    QCOMPARE(::floorPoint(QPointF(10.9, 0)), QPoint(10, 0));
    QCOMPARE(::floorPoint(QPointF(0, 10.9)), QPoint(0, 10));
    QCOMPARE(::floorPoint(QPointF(10.9, 10.9)), QPoint(10, 10));
}

void Testglobal::boundingRect()
{
    QColor blue = QColor(0, 0, 255);
    QRgb blueRgb = blue.rgb();

    // ------------------------------------------------------------------------
    // Test: Invalid image
    QCOMPARE(::boundingRect(QImage()), Rect());

    // ------------------------------------------------------------------------
    // Test: Empty image
    QImage image(5, 5, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All red
    QPainter painter(&image);
    painter.fillRect(image.rect(), QColor(255, 0, 0));
    painter.end();
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All green
    painter.begin(&image);
    painter.fillRect(image.rect(), QColor(0, 255, 0));
    painter.end();
    QCOMPARE(::boundingRect(image), Rect());

    // ------------------------------------------------------------------------
    // Test: All blue
    painter.begin(&image);
    painter.fillRect(image.rect(), blue);
    painter.end();
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: single pixel tests
    for (int i=0; i< image.height(); ++i)
    {
        for (int j=0; j< image.width(); ++j)
        {
            image.fill(0);
            image.setPixel(j, i, blueRgb);
            QCOMPARE(::boundingRect(image), Rect(j, i, 0, 0));
        }
    }

    // ------------------------------------------------------------------------
    // Test: topleft and bottom right diagonal
    image.fill(0);
    image.setPixel(0, 0, blueRgb);
    image.setPixel(4, 4, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: topright and bottom left
    image.fill(0);
    image.setPixel(0, 4, blueRgb);
    image.setPixel(4, 0, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(0, 0, 4, 4));

    // ------------------------------------------------------------------------
    // Test: inside rectangle
    image.fill(0);
    image.setPixel(1, 3, blueRgb);
    image.setPixel(3, 1, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(1, 1, 2, 2));

    image.fill(0);
    image.setPixel(3, 1, blueRgb);
    image.setPixel(1, 3, blueRgb);
    QCOMPARE(::boundingRect(image), Rect(1, 1, 2, 2));
}

QTEST_APPLESS_MAIN(Testglobal)
#include "Testglobal.moc"
