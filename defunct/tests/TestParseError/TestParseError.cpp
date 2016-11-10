/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "ParseError.h"

class TestParseError : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void set();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestParseError::constructor()
{
    ParseError e("Test exception");
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line().isEmpty());
    QVERIFY(e.lineNumber() == -1);
    QVERIFY(e.columnNumber() == -1);

    ParseError e1("Test exception", "Your line is invalid");
    QVERIFY(e1.message() == "Test exception");
    QVERIFY(e1.line() == "Your line is invalid");
    QVERIFY(e1.lineNumber() == -1);
    QVERIFY(e1.columnNumber() == -1);

    ParseError e2("Test exception", "Your line is invalid", 2);
    QVERIFY(e2.message() == "Test exception");
    QVERIFY(e1.line() == "Your line is invalid");
    QVERIFY(e2.lineNumber() == 2);
    QVERIFY(e2.columnNumber() == -1);

    ParseError e3("Test exception", "Your line is invalid", 5, 4);
    QVERIFY(e3.message() == "Test exception");
    QVERIFY(e1.line() == "Your line is invalid");
    QVERIFY(e3.lineNumber() == 5);
    QVERIFY(e3.columnNumber() == 4);
}

void TestParseError::constructorCopy()
{
    ParseError e_("Test exception");
    ParseError e(e_);
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line().isEmpty());
    QVERIFY(e.lineNumber() == -1);
    QVERIFY(e.columnNumber() == -1);

    ParseError e1_("Test exception", "Your line is invalid");
    ParseError e1(e1_);
    QVERIFY(e1.message() == "Test exception");
    QVERIFY(e1.line() == "Your line is invalid");
    QVERIFY(e1.lineNumber() == -1);
    QVERIFY(e1.columnNumber() == -1);

    ParseError e2_("Test exception", "Your line is invalid", 2);
    ParseError e2(e2_);
    QVERIFY(e2.message() == "Test exception");
    QVERIFY(e2.line() == "Your line is invalid");
    QVERIFY(e2.lineNumber() == 2);
    QVERIFY(e2.columnNumber() == -1);

    ParseError e3_("Test exception", "Your line is invalid", 5, 4);
    ParseError e3(e3_);
    QVERIFY(e3.message() == "Test exception");
    QVERIFY(e3.line() == "Your line is invalid");
    QVERIFY(e3.lineNumber() == 5);
    QVERIFY(e3.columnNumber() == 4);
}

void TestParseError::set()
{
    ParseError e("Test", "", 10, 20);

    e.set("Test exception");
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line().isEmpty());
    QVERIFY(e.lineNumber() == -1);
    QVERIFY(e.columnNumber() == -1);

    e.set("Test exception", "Your line is invalid");
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line() == "Your line is invalid");
    QVERIFY(e.lineNumber() == -1);
    QVERIFY(e.columnNumber() == -1);

    e.set("Test exception", "Your line is invalid", 5);
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line() == "Your line is invalid");
    QVERIFY(e.lineNumber() == 5);
    QVERIFY(e.columnNumber() == -1);

    e.set("Test exception", "Your line is invalid", 8, 16);
    QVERIFY(e.message() == "Test exception");
    QVERIFY(e.line() == "Your line is invalid");
    QVERIFY(e.lineNumber() == 8);
    QVERIFY(e.columnNumber() == 16);
}

QTEST_MAIN(TestParseError)
#include "TestParseError.moc"
