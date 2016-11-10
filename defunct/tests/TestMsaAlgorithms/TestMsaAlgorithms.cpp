/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "MsaAlgorithms.h"

#include "AnonSeq.h"
#include "BioString.h"
#include "Msa.h"

#include "global.h"

class TestMsaAlgorithms : public QObject
{
    Q_OBJECT

public:
    TestMsaAlgorithms() : QObject()
    {
        qRegisterMetaType<ListHashCharInt>("ListHashCharInt");
    }

private slots:
    void calculateMsaCharCountDistribution_data();
    void calculateMsaCharCountDistribution();

private:
    Msa *createMsa(const QStringList &subseqs) const;   // Helper function for creating a Msa from subseqs
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
Msa *TestMsaAlgorithms::createMsa(const QStringList &subseqStringList) const
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
void TestMsaAlgorithms::calculateMsaCharCountDistribution_data()
{
    QTest::addColumn<Msa *>("msa");
    QTest::addColumn<QRect>("region");
    QTest::addColumn<ListHashCharInt>("expectedResult");

    // ------------------------------------------------------------------------
    QTest::newRow("msa with no sequences - null rect")
            << new Msa(eAminoAlphabet)
            << QRect()
            << ListHashCharInt();

    // ------------------------------------------------------------------------
    {
        QString sequence = "ABCDEF";
        ListHashCharInt dist;
        for (int i=0; i< sequence.length(); ++i)
        {
            dist << QHash<char, int>();
            dist.last().insert(sequence.toAscii().at(i), 1);
        }

        QTest::newRow("msa, 1 sequence - null rect")
                << createMsa(QStringList() << "ABCDEF")
                << QRect()
                << dist;
    }

    // ------------------------------------------------------------------------
    // Test: Numerous randomly created multiple sequence alignments of random dimensions
    {
        // i -> msa length
        for (int l=1; l<= 100; ++l)
        {
            ListHashCharInt dist;
            // Initialize empty dist that is size of the msa length
            for (int i=0; i<l; ++i)
                dist << QHash<char, int>();

            // Generate j random pseudo subseqs, all of length l
            QStringList subseqStringList;

            // j -> number of sequences
            int nSeqs = randomInteger(0, 50);
            if (nSeqs == 0)
                dist.clear();

            for (int j=0; j< nSeqs; ++j)
            {
                // The msa will not be created if it consists of all gaps! Therefore, always make the first character
                // of every sequence an X
                QString subseqString;
                for (int i=0; i< l; ++i)
                {
                    char randomCharacter = constants::kGenericBioStringCharacters[randomInteger(0, qstrlen(constants::kGenericBioStringCharacters)-1)];
                    if (i == 0)
                        randomCharacter = 'X';
                    subseqString.append(randomCharacter);
                    if (!isGapCharacter(randomCharacter))
                        ++dist[i][randomCharacter];
                }
                subseqStringList << subseqString;
            }

            QTest::newRow(QString("Random Msa of length %1, sequences: %2 - null rect").arg(l).arg(nSeqs).toAscii())
                    << createMsa(subseqStringList)
                    << QRect()
                    << dist;
        }
    }

    // ------------------------------------------------------------------------
    // Test: regional computation
    {
        // i -> msa length
        for (int l=1; l<= 100; ++l)
        {
            ListHashCharInt dist;

            // Generate j random pseudo subseqs, all of length l
            QStringList subseqStringList;
            QRect targetRect;

            // j -> number of sequences
            int nSeqs = randomInteger(0, 50);
            if (nSeqs == 0)
            {
                dist.clear();
            }
            else
            {
                // Dimensions: nSeqs x l length
                // Construct random rectangle within these bounds
                targetRect.setLeft(randomInteger(1, l));
                targetRect.setWidth(randomInteger(1, l - targetRect.left() + 1));
                targetRect.setTop(randomInteger(1, nSeqs));
                targetRect.setHeight(randomInteger(1, nSeqs - targetRect.top() + 1));

                // Initialize empty dist that is size of the msa length
                for (int i=0; i<targetRect.width(); ++i)
                    dist << QHash<char, int>();

                for (int j=0; j< nSeqs; ++j)
                {
                    // The msa will not be created if it consists of all gaps! Therefore, always make the first character
                    // of every sequence an X
                    QString subseqString;
                    for (int i=0; i< l; ++i)
                    {
                        char randomCharacter = constants::kGenericBioStringCharacters[randomInteger(0, qstrlen(constants::kGenericBioStringCharacters)-1)];
                        if (i == 0)
                            randomCharacter = 'X';

                        subseqString.append(randomCharacter);

                        if (targetRect.contains(i+1, j+1) && !isGapCharacter(randomCharacter))
                            ++dist[i+1-targetRect.left()][randomCharacter];
                    }
                    subseqStringList << subseqString;
                }
            }

            QTest::newRow(QString("Random Msa of length %1, sequences: %2 - valid rect").arg(l).arg(nSeqs).toAscii())
                    << createMsa(subseqStringList)
                    << targetRect
                    << dist;
        }
    }
}

void TestMsaAlgorithms::calculateMsaCharCountDistribution()
{
    QFETCH(Msa *, msa);
    QFETCH(QRect, region);
    QFETCH(ListHashCharInt, expectedResult);

    QVERIFY(msa);
    QVERIFY(region.isNull() || region.isValid());

    ListHashCharInt result = ::calculateMsaCharCountDistribution(*msa, region);
    if (result != expectedResult)
    {
        qDebug() << msa->length();
        qDebug() << msa->subseqCount();
        qDebug() << QString("(%1, %2), (%3, %4)").arg(region.left()).arg(region.top()).arg(region.right()).arg(region.bottom());

        QStringList bob;
        for (int i=0; i< msa->subseqCount(); ++i)
            bob << msa->at(i+1)->bioString().sequence();

        qDebug() << "\n" << bob.join("\n");
        qDebug() << "Expected" << expectedResult;
        qDebug() << "Actual" << ::calculateMsaCharCountDistribution(*msa, region);
    }

    QCOMPARE(result, expectedResult);

    // Check that the number of msa sequences == sum of characters in each column
    /*
    int rows = (region.isNull()) ? msa->subseqCount() : region.height();
    for (int i=0, z=result.count(); i<z; ++i)
    {
        int sum = 0;
        foreach (int count, result.at(i).values())
            sum += count;

        QCOMPARE(rows, sum);
    }
    */

    delete msa;
    msa = 0;
}

QTEST_APPLESS_MAIN(TestMsaAlgorithms)
#include "TestMsaAlgorithms.moc"
