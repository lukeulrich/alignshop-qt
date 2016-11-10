/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/dnaseq.h"

using namespace PrimerDesign;

class TestDnaSeq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void nameShouldBeNonEmpty()
    {
        DnaSeq seq;
        QVERIFY2(seq.name().length() > 0, "Name should have some default value.");
    }

    // ------------------------------------------------------------------------------------------------
    // Properties
    void testName()
    {
        DnaSeq seq;
        QString blankName = " \t\r ";
        QString validName = "Foo Bar";

        QVERIFY2(!seq.setName(""), "An empty name is disallowed.");
        QVERIFY2(seq.name().length() > 0, "The empty name should not have been set.");

        QVERIFY2(!seq.setName(blankName), "Blank names are not allowed.");
        QVERIFY2(seq.name() != blankName, "The blank name should not have been set.");

        QVERIFY2(seq.setName(validName), "Valid name check.");
        QVERIFY2(seq.name() == validName, "The valid name was set.");
    }

    void testNotes()
    {
        DnaSeq seq;
        QString fullNote = "Foo Bar";

        QVERIFY2(seq.setNotes(fullNote), "Full note check.");
        QVERIFY2(seq.notes() == fullNote, "The full note was set.");

        QVERIFY2(seq.setNotes(""), "An empty note is allowed.");
        QVERIFY2(seq.notes().length() == 0, "The empty note was set.");
    }

    void testSequence()
    {
        QString data = " ACGTTCGG ";
        DnaSeq seq;

        QVERIFY(seq.setSequence(data));
        data = data.trimmed();
        QVERIFY(seq.sequence() == data);

        QVERIFY(!seq.setSequence("hello"));
        QVERIFY(seq.sequence() == data);
    }

    void sequenceShouldRejectEmptyValues()
    {
        DnaSeq seq;
        QVERIFY(seq.setSequence(constants::kDnaCharacters));
        QVERIFY(!seq.setSequence(""));
        QVERIFY(seq.sequence() == constants::kDnaCharacters);
    }
};

QTEST_MAIN(TestDnaSeq)
#include "TestDnaSeq.moc"
