/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/primer.h"

using namespace PrimerDesign;

class TestPrimer : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor()
    {
        Primer p;
        QVERIFY(p.tm() == constants::kTmDefault);
        QVERIFY(p.enthalpy() == Primer::DEFAULT_ENTHALPY);
        QVERIFY(p.entropy() == Primer::DEFAULT_ENTROPY);
        QVERIFY(p.homoDimerScore() == Primer::DEFAULT_HOMO_SCORE);
    }

    // ------------------------------------------------------------------------------------------------
    // Getters/setters
    void testSequence()
    {
        QString sequence = "actg";
        Primer p;
        QVERIFY(p.setSequence(sequence));
        QVERIFY(!p.setSequence(""));
        QVERIFY(p.sequence() == sequence.toUpper());
        QVERIFY(!p.setSequence("hello"));
        QVERIFY(p.sequence() == sequence.toUpper());
    }

    void testTm()
    {
        Primer p;
        p.setTm(constants::kTmDefault);

        p.setTm(constants::kTmMax + 1);
        QVERIFY(p.tm() == constants::kTmDefault);

        p.setTm(constants::kTmDefaultMin);
        QVERIFY(p.tm() == constants::kTmDefaultMin);

        p.setTm(constants::kTmMin - 1);
        QVERIFY(p.tm() == constants::kTmDefaultMin);

        p.setTm(constants::kTmMax);
        QVERIFY(p.tm() == constants::kTmMax);

        p.setTm(constants::kTmMin);
        QVERIFY(p.tm() == constants::kTmMin);
    }

    void testEntropy()
    {
        Primer p;
        p.setEntropy(Primer::DEFAULT_ENTROPY + 1);
        QVERIFY(p.entropy() == Primer::DEFAULT_ENTROPY + 1);
    }

    void testEnthalpy()
    {
        Primer p;
        p.setEnthalpy(Primer::DEFAULT_ENTHALPY + 1);
        QVERIFY(p.enthalpy() == Primer::DEFAULT_ENTHALPY + 1);
    }

    void testHomoDimerScore()
    {
        Primer p;
        p.setHomoDimerScore(Primer::DEFAULT_HOMO_SCORE + 1);
        QVERIFY(p.homoDimerScore() == Primer::DEFAULT_HOMO_SCORE + 1);
    }
};

QTEST_MAIN(TestPrimer)
#include "TestPrimer.moc"
