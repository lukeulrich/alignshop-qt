/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "SymbolColorProvider.h"

#include "AnonSeq.h"
#include "BioString.h"
#include "LiveMsaCharCountDistribution.h"
#include "LiveSymbolString.h"
#include "Msa.h"
#include "Subseq.h"


class TestSymbolColorProvider : public QObject
{
    Q_OBJECT

private slots:
    void noLiveSymbolString();
    void validSymbolString();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
Msa *createMsa(const QStringList &subseqStringList)
{
    Msa *msa = new Msa;
    foreach (QString subseqString, subseqStringList)
    {
        AnonSeq anonSeq(0, subseqString);
        Subseq *subseq = new Subseq(anonSeq);
        if (!subseq->setBioString(subseqString))
        {
            delete msa;
            return 0;
        }

        if (!msa->append(subseq))
        {
            delete msa;
            return 0;
        }
    }

    return msa;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestSymbolColorProvider::noLiveSymbolString()
{
    TextColorStyle defaultStyle = SymbolColorScheme().defaultTextColorStyle();

    Msa *msa = createMsa(QStringList() << "ABC" << "A-C" << "-B-");
    QVERIFY(msa);

    // ------------------------------------------------------------------------
    // Test: absolute defaults - should return default text color style everytime
    {
        SymbolColorProvider x(0, SymbolColorScheme());

        for (int i=1; i< 4; ++i)
            for (int j=1; j< 4; ++j)
                QCOMPARE(x.color(*msa, i, j), defaultStyle);
    }

    // ------------------------------------------------------------------------
    // Test: valid symbol color scheme, but no live symbol string
    {
        TextColorStyle blue(Qt::blue);
        SymbolColorScheme scheme;
        scheme.setSymbolsTextColorStyle('A', "%", TextColorStyle(Qt::white, Qt::green));
        scheme.setTextColorStyle('C', blue);

        SymbolColorProvider x(0, scheme);
        for (int i=1; i< 4; ++i)
        {
            for (int j=1; j< 4; ++j)
            {
                if (msa->at(i)->bioString().sequence().toAscii().at(j-1) == 'C')
                    QCOMPARE(x.color(*msa, i, j), blue);
                else
                    QCOMPARE(x.color(*msa, i, j), defaultStyle);
            }
        }
    }
}

void TestSymbolColorProvider::validSymbolString()
{
    TextColorStyle defaultStyle = SymbolColorScheme().defaultTextColorStyle();

    Msa *msa = createMsa(QStringList() << "ABC" << "A-C" << "ABD");
    QVERIFY(msa);

    BioSymbolGroup bsg;
    bsg << BioSymbol('a', "A", .7)
        << BioSymbol('b', "B", .9)
        << BioSymbol('c', "C", .1);

    LiveMsaCharCountDistribution *dist = new LiveMsaCharCountDistribution(msa, this);
    LiveSymbolString *liveSymbolString = new LiveSymbolString(dist, bsg, this);

    TextColorStyle cyanBlack(Qt::cyan, Qt::black);
    TextColorStyle yellowRed(Qt::yellow, Qt::red);
    TextColorStyle blueWhite(Qt::blue, Qt::white);
    TextColorStyle grayMagenta(Qt::gray, Qt::magenta);

    // ------------------------------------------------------------------------
    // Test: Valid symbol string, but empty SymbolColorScheme
    {
        SymbolColorProvider x(liveSymbolString, SymbolColorScheme());
        for (int i=1; i< 4; ++i)
            for (int j=1; j< 4; ++j)
                QCOMPARE(x.color(*msa, i, j), defaultStyle);
    }

    // ------------------------------------------------------------------------
    // Test: Valid symbol string, but valid SymbolColorScheme
    {
        SymbolColorScheme scheme;
        scheme.setTextColorStyle('B', cyanBlack);
        scheme.setSymbolsTextColorStyle('B', "b", grayMagenta);
        scheme.setSymbolsTextColorStyle('A', "a", yellowRed);
        scheme.setSymbolsTextColorStyle('C', "c", blueWhite);

        SymbolColorProvider x(liveSymbolString, scheme);

        // Check the first sequence
        QCOMPARE(x.color(*msa, 1, 1), yellowRed);
        QCOMPARE(x.color(*msa, 1, 2), cyanBlack);
        QCOMPARE(x.color(*msa, 1, 3), blueWhite);

        // Check the second sequence
        QCOMPARE(x.color(*msa, 2, 1), yellowRed);
        QCOMPARE(x.color(*msa, 2, 2), defaultStyle);
        QCOMPARE(x.color(*msa, 2, 3), blueWhite);

        // Check the third sequence
        QCOMPARE(x.color(*msa, 3, 1), yellowRed);
        QCOMPARE(x.color(*msa, 3, 2), cyanBlack);
        QCOMPARE(x.color(*msa, 3, 3), defaultStyle);
    }
}


QTEST_APPLESS_MAIN(TestSymbolColorProvider)
#include "TestSymbolColorProvider.moc"
