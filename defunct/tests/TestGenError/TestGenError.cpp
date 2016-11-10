/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "GenError.h"

class TestGenError : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Operators
    void operator_eq();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void message();
    void setMessage();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestGenError::constructor()
{
    GenError e1;
    GenError e2("Test exception");
}

void TestGenError::constructorCopy()
{
    GenError e("Test exception");
    GenError e2(e);

    QVERIFY(e.message() == e2.message());
}

void TestGenError::operator_eq()
{
    GenError e("Test");
    e = "Test equals";
    QVERIFY(e.message() == "Test equals");

    GenError e2 = "Blah";
    QVERIFY(e2.message() == "Blah");
}

void TestGenError::message()
{
    QString error = "Test error message";

    GenError e(error);
    QVERIFY(e.message() == error);
}

void TestGenError::setMessage()
{
    GenError e;

    QVERIFY(e.message().isEmpty());
    e.setMessage("Blargh");
    QVERIFY(e.message() == "Blargh");
}


QTEST_MAIN(TestGenError)
#include "TestGenError.moc"
