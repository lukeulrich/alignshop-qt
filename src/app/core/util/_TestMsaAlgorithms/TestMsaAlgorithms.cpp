/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../MsaAlgorithms.h"

#include "../../Alphabet.h"
#include "../../BioString.h"
#include "../../Msa.h"
#include "../../Seq.h"
#include "../../global.h"
#include "../../misc.h"

class TestMsaAlgorithms : public QObject
{
    Q_OBJECT

public:
    TestMsaAlgorithms() : QObject()
    {
        qRegisterMetaType<VectorHashCharInt>("VectorHashCharInt");
    }

private slots:
    void calculateMsaCharCountDistribution_data();
    void calculateMsaCharCountDistribution();

private:
    Msa *createMsa(const QVector<QByteArray> &subseqs) const;   // Helper function for creating a Msa from subseqs
};

Q_DECLARE_METATYPE(Msa *);
Q_DECLARE_METATYPE(PosiRect);
Q_DECLARE_METATYPE(VectorHashCharInt);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
Msa *TestMsaAlgorithms::createMsa(const QVector<QByteArray> &subseqVectorList) const
{
    Msa *msa = new Msa;
    foreach (QByteArray subseqByteArray, subseqVectorList)
    {
        Seq seq(subseqByteArray);
        Subseq *subseq = new Subseq(seq);
        if (!subseq->setBioString(subseqByteArray))
        {
            delete msa;
            return nullptr;
        }

        if (!msa->append(subseq))
        {
            delete msa;
            return nullptr;
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
    QTest::addColumn<PosiRect>("msaRect");
    QTest::addColumn<VectorHashCharInt>("expectedResult");

    QByteArray characters = constants::kAminoAmbiguousAlphabet.characters();

    // ------------------------------------------------------------------------
    QTest::newRow("msa with no sequences - null rect")
            << new Msa(eAminoGrammar)
            << PosiRect()
            << VectorHashCharInt();

    // ------------------------------------------------------------------------
    {
        QString sequence = "ABCDEF";
        VectorHashCharInt dist;
        for (int i=0; i< sequence.length(); ++i)
        {
            dist << QHash<char, int>();
            dist.last().insert(sequence.toAscii().at(i), 1);
        }

        QTest::newRow("msa, 1 sequence - null rect")
                << createMsa(QVector<QByteArray>() << "ABCDEF")
                << PosiRect()
                << dist;
    }

    // ------------------------------------------------------------------------
    // Test: Numerous randomly created multiple sequence alignments of random dimensions
    {
        // i -> msa length
        for (int l=1; l<= 100; ++l)
        {
            VectorHashCharInt dist;
            // Initialize empty dist that is size of the msa length
            for (int i=0; i<l; ++i)
                dist << QHash<char, int>();

            // Generate j random pseudo subseqs, all of length l
            QVector<QByteArray> subseqByteArrayVector;

            // j -> number of sequences
            int nSeqs = ::randomInteger(0, 50);
            if (nSeqs == 0)
                dist.clear();

            for (int j=0; j< nSeqs; ++j)
            {
                // The msa will not be created if it consists of all gaps! Therefore, always make the first character
                // of every sequence an X
                QByteArray subseqByteArray;
                for (int i=0; i< l; ++i)
                {
                    char randomCharacter = characters.at(::randomInteger(0, characters.length()-1));
                    if (i == 0)
                        randomCharacter = 'X';
                    subseqByteArray.append(randomCharacter);
                    if (!::isGapCharacter(randomCharacter))
                        ++dist[i][randomCharacter];
                }
                subseqByteArrayVector << subseqByteArray;
            }

            QTest::newRow(QString("Random Msa of length %1, sequences: %2 - null rect").arg(l).arg(nSeqs).toAscii())
                    << createMsa(subseqByteArrayVector)
                    << PosiRect()
                    << dist;
        }
    }

    // ------------------------------------------------------------------------
    // Test: regional computation
    {
        // l -> msa length
        for (int l=1; l<= 100; ++l)
        {
            VectorHashCharInt dist;

            // Generate j random pseudo subseqs, all of length l
            QVector<QByteArray> subseqByteArrayVector;
            PosiRect targetRect;

            // j -> number of sequences
            int nSeqs = ::randomInteger(0, 50);
            if (nSeqs == 0)
            {
                dist.clear();
            }
            else
            {
                // Dimensions: nSeqs x l length
                // Construct random rectangle within these bounds
                targetRect.setLeft(::randomInteger(1, l));
                targetRect.setWidth(::randomInteger(1, l - targetRect.left() + 1));
                targetRect.setTop(::randomInteger(1, nSeqs));
                targetRect.setHeight(::randomInteger(1, nSeqs - targetRect.top() + 1));

                // Initialize empty dist that is size of the msa length
                for (int i=0; i<targetRect.width(); ++i)
                    dist << QHash<char, int>();

                for (int j=0; j< nSeqs; ++j)
                {
                    // The msa will not be created if it consists of all gaps! Therefore, always make the first character
                    // of every sequence an X
                    QByteArray subseqString;
                    for (int i=0; i< l; ++i)
                    {
                        char randomCharacter = characters.at(::randomInteger(0, characters.length()-1));
                        if (i == 0)
                            randomCharacter = 'X';

                        subseqString.append(randomCharacter);

                        if (targetRect.contains(i+1, j+1) && !::isGapCharacter(randomCharacter))
                            ++dist[i+1-targetRect.left()][randomCharacter];
                    }
                    subseqByteArrayVector << subseqString;
                }
            }

            QTest::newRow(QString("Random Msa of length %1, sequences: %2 - valid rect").arg(l).arg(nSeqs).toAscii())
                    << createMsa(subseqByteArrayVector)
                    << targetRect
                    << dist;
        }
    }
}

void TestMsaAlgorithms::calculateMsaCharCountDistribution()
{
    QFETCH(Msa *, msa);
    QFETCH(PosiRect, msaRect);
    QFETCH(VectorHashCharInt, expectedResult);

    QVERIFY(msa);
    QVERIFY(msaRect.isNull() || msaRect.normalized().isValid());

    CharCountDistribution result = ::calculateMsaCharCountDistribution(*msa, msaRect);
    if (result.charCounts() != expectedResult)
    {
        qDebug() << msa->length();
        qDebug() << msa->subseqCount();
        qDebug() << QString("(%1, %2), (%3, %4)").arg(msaRect.left()).arg(msaRect.top()).arg(msaRect.right()).arg(msaRect.bottom());

        QStringList bob;
        for (int i=0; i< msa->subseqCount(); ++i)
            bob << msa->at(i+1)->asByteArray() << "\n";

        qDebug() << "\n" << bob;
        qDebug() << "Expected" << expectedResult;
        qDebug() << "Actual" << result.charCounts();
    }

    QCOMPARE(result.charCounts(), expectedResult);
    int divisor = (msaRect.isNull()) ? msa->rowCount() : msaRect.normalized().height();

    // Divisor may never be zero
    if (divisor == 0)
        divisor = 1;
    QCOMPARE(result.divisor(), divisor);

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
