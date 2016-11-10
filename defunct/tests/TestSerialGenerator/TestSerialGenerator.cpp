/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "SerialGenerator.h"

class TestSerialGenerator : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor();
    void serialGeneration();
    void constructorWithCurrentValue();
    void constructorWithParams();
    void copyConstructor();
    void assignment();
    void settingCalled();
    void settingValue();
};

void TestSerialGenerator::defaultConstructor()
{
    SerialGenerator sg;
    QVERIFY2(sg.currentValue() == 1, "Default value does not equal 1");
    QVERIFY2(sg.isCalled() == false, "Default called state is true");
}

void TestSerialGenerator::serialGeneration()
{
    SerialGenerator sg;
    QVERIFY2(sg.nextValue() == 1, "First value from fresh SerialGenerator is not 1");
    for (int i=2; i< 100; ++i)
        QVERIFY2(sg.nextValue() == i, "for(i=2..100) nextValue() != i");
}

void TestSerialGenerator::constructorWithCurrentValue()
{
    SerialGenerator sg(0);
    QVERIFY2(sg.currentValue() == 0, "Value does not equal 0");

    SerialGenerator sg2(-50);
    QVERIFY2(sg2.currentValue() == -50, "Value does not equal -50");

    SerialGenerator sg3(1020);
    QVERIFY2(sg3.currentValue() == 1020, "Value does not equal 1020");
}

void TestSerialGenerator::constructorWithParams()
{
    SerialGenerator sg(50, false);
    QVERIFY2(sg.currentValue() == 50 || !sg.isCalled(), "Invalid default initialization of all parameters");
    QVERIFY2(sg.nextValue() == 50, "Next value is not 50");
    QVERIFY2(sg.nextValue() == 51, "Next value is not 51");

    // Test an intialized sequence and called
    SerialGenerator sg2(50, true);
    QVERIFY2(sg2.currentValue() == 50 || sg.isCalled(), "Invalid default initialization of all parameters");
    QVERIFY2(sg2.nextValue() == 51, "Next value is not 51");
}

void TestSerialGenerator::copyConstructor()
{
    SerialGenerator sg1(75, true);
    SerialGenerator sg2(sg1);
    QVERIFY2(sg2.currentValue() == 75, "Copied currentValue is incorrect");
    QVERIFY2(sg2.isCalled() == true, "Copied isCalled is incorrect");
    QVERIFY2(sg2.nextValue() == 76, "Next value is not 76");

    SerialGenerator sg3(100, false);
    SerialGenerator sg4(sg3);
    QVERIFY2(sg4.currentValue() == 100, "Copied currentValue is incorrect");
    QVERIFY2(sg4.isCalled() == false, "Copied isCalled is incorrect");
    QVERIFY2(sg4.nextValue() == 100, "Next value is not 1");
    QVERIFY2(sg4.nextValue() == 101, "Next value is not 2");
}

void TestSerialGenerator::assignment()
{
    SerialGenerator sg1;
    QVERIFY2(sg1.currentValue() == 1, "Default value does not equal 1");
    QVERIFY2(sg1.isCalled() == false, "Default called state is true");

    SerialGenerator sg2(100, true);
    sg1 = sg2;
    QVERIFY2(sg1.currentValue() == 100, "Assigned currentValue is incorrect");
    QVERIFY2(sg1.isCalled() == true, "Assigned isCalled is incorrect");
    QVERIFY2(sg1.nextValue() == 101, "Next value is not 101");
}

void TestSerialGenerator::settingCalled()
{
    SerialGenerator sg(100, false);
    QVERIFY2(sg.isCalled() == false, "Invalid isCalled value");
    sg.setCalled(false);
    QVERIFY2(sg.isCalled() == false, "Invalid isCalled value");
    sg.setCalled(true);
    QVERIFY2(sg.isCalled() == true, "Invalid isCalled value");
    sg.setCalled(false);
    QVERIFY2(sg.isCalled() == false, "Invalid isCalled value");
}

void TestSerialGenerator::settingValue()
{
    SerialGenerator sg(100, true);
    QVERIFY2(sg.currentValue() == 100, "Invalid currentValue");

    sg.setValue(-33);
    QVERIFY2(sg.currentValue() == -33, "Invalid currentValue");
    QVERIFY2(sg.nextValue() == -32, "nextValue after setValue failed");

    sg.setValue(999, false);
    QVERIFY2(sg.currentValue() == 999, "Invalid currentValue");
    QVERIFY2(sg.nextValue() == 999, "nextValue after setValue failed");

    sg.setValue(999, true);
    QVERIFY2(sg.nextValue() == 1000, "nextValue after setValue failed");
}

QTEST_MAIN(TestSerialGenerator)
 #include "TestSerialGenerator.moc"
