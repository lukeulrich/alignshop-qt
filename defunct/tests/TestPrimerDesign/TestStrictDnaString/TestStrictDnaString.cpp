/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/strictdnastring.h"
#include "BioString.h"

using namespace PrimerDesign;

class TestStrictDnaString : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor()
    {
        QString sequence = "AcgT";
        QString valid = StrictDnaString(sequence).value();
        QString invalid = StrictDnaString("hello world").value();

        QVERIFY2(valid == sequence.toUpper(), "Valid dna should be accepted and cased.");
        QVERIFY2(invalid.isEmpty(), "Invalid sequences should result in an empty string.");
    }

    void customValidatorShouldRunIfSpecified()
    {
        StrictDnaString required(StrictDnaString::isNotEmpty);
        StrictDnaString optional;

        QVERIFY(optional.setValue(""));
        QVERIFY(optional.value() == "");

        QVERIFY(required.setValue(constants::kDnaCharacters));
        QVERIFY(!required.setValue(""));
        QVERIFY(required.value() == constants::kDnaCharacters);
    }

    // ------------------------------------------------------------------------------------------------
    // Operators
    void shouldBeCastableToQString()
    {
        StrictDnaString str(constants::kDnaCharacters);
        QVERIFY((QString)str == constants::kDnaCharacters);
    }

    // ------------------------------------------------------------------------------------------------
    // Properties
    void testValue()
    {
        QString sequence = "  ACGT  ";
        StrictDnaString str(sequence);
        QVERIFY(!str.setValue("hello"));
        QVERIFY(str.value() == sequence.trimmed().toUpper());
    }
};

QTEST_MAIN(TestStrictDnaString)
#include "TestStrictDnaString.moc"
