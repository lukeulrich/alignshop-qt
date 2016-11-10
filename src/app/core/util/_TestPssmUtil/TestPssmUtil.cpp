/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../PssmUtil.h"
#include "../../Parsers/PssmParser.h"
#include "../../PODs/NormalizedPssm.h"

class TestPssmUtil : public QObject
{
    Q_OBJECT

private slots:
    void normalizePssm();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPssmUtil::normalizePssm()
{
    // Load in a pssm
    QString pssmFile = "files/ag_pssm";
    PssmParser x;
    Pssm pssm;
    try
    {
        pssm = x.parseFile(pssmFile);
    }
    catch(...)
    {
        QVERIFY(0);
    }

    // Test: normalize it and check results
    NormalizedPssm nPssm = ::normalizePssm(pssm);

    QCOMPARE(nPssm.mapping_, pssm.mapping_);
    QCOMPARE(nPssm.rows_.size(), pssm.rows_.size());
    for (int i=0; i< pssm.rows_.size(); ++i)
    {
        const PssmRow &pssmRow = pssm.rows_.at(i);
        const NormalizedPssmRow &row = nPssm.rows_.at(i);
        for (int j=0; j<constants::kPssmWidth; ++j)
        {
            double expScore = 1. / (1. + exp(-.5 * pssmRow.scores_[j] / pssm.positScaleFactor_));
            QCOMPARE(row.scores_[j], expScore);
        }
    }
}


QTEST_APPLESS_MAIN(TestPssmUtil)
#include "TestPssmUtil.moc"
