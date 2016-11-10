/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BioSymbol.h"

class TestBioSymbol : public QObject
{
    Q_OBJECT

private slots:
    void constructor_data();
    void constructor();     // Also tests symbol, characters, and threshold
    void characters();      // Also tests characterSet and setCharacters
    void hasCharacter();
    void addCharacters();
    void removeCharacters();
    void setCharacters();
    void setSymbol();       // Also tests symbol
    void setThreshold();    // Also tests threshold

    void operator_eqeq();   // Also tests operator!=
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
QSet<char> stringToSet(const QByteArray &string)
{
    QSet<char> set;
    const char *x = string.constData();
    while (*x)
    {
        set << *x;
        ++x;
    }

    return set;
}

QByteArray setToString(const QSet<char> &set)
{
    QByteArray result;
    foreach (char ch, set.toList())
        result += ch;

    return result;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBioSymbol::constructor_data()
{
    QTest::addColumn<char>("symbol");
    QTest::addColumn<QByteArray>("characters");
    QTest::addColumn<qreal>("threshold");

    QTest::newRow("space symbol and characters, 0 threshold") << ' ' << QByteArray() << 0.;
    QTest::newRow("- symbol, empty characters, 0 threshold") << '-' << QByteArray() << 0.;
    QTest::newRow("% symbol, multiple blank characters, 0 threshold") << '%' << QByteArray("    ") << 0.;
    QTest::newRow("9 symbol, single character, 1 threshold") << '9' << QByteArray("A") << 1.;
    QTest::newRow("^ symbol, multiple same characters, .5 threshold") << '^' << QByteArray("AAAA") << .5;
    QTest::newRow("f symbol, multiple different characters, .25 threshold") << 'f' << QByteArray("ACGTN") << .25;
}


void TestBioSymbol::constructor()
{
    QFETCH(char, symbol);
    QFETCH(QByteArray, characters);
    QFETCH(qreal, threshold);

    BioSymbol x(symbol, characters, threshold);
    QCOMPARE(x.symbol(), symbol);
    QCOMPARE(x.characterSet(), stringToSet(characters));

    // Special case when comparing doubles - will likely fail when comparing doubles/reals
    // thus, add 1 to each value and compare these.
    //
    // http://doc.trolltech.com/latest/qtest.html#QCOMPARE
    QCOMPARE(x.threshold()+1, threshold+1);
}

void TestBioSymbol::characters()
{
    QVector<QByteArray> testStrings;
    testStrings << QByteArray()
                << "  "
                << "ACGT"
                << "jasdlkfj23sdfef--324234"
                << "   ASDF  asf  wer#@$@#"
                << "23489k-adsf2lk  alsjdf;lkjw2jasld   \n";

    foreach (const QByteArray &string, testStrings)
    {
        QSet<char> uniqueChars = stringToSet(string);
        QList<char> sortedUniqueChars = uniqueChars.toList();
        qSort(sortedUniqueChars);
        QByteArray uniqueString;
        foreach (char ch, sortedUniqueChars)
            uniqueString += ch;

        BioSymbol x('-', QByteArray(), 1.);
        x.setCharacters(string);

        // Compare the result for the characters value
        QList<char> charactersResultList = stringToSet(x.characters()).toList();
        qSort(charactersResultList);
        QByteArray charactersResult;
        foreach (char ch, charactersResultList)
            charactersResult += ch;
        QCOMPARE(charactersResult, uniqueString);

        // Compare the character sets
        QList<char> characterSetList = x.characterSet().toList();
        qSort(characterSetList);
        QCOMPARE(characterSetList, sortedUniqueChars);
    }
}

void TestBioSymbol::hasCharacter()
{
    BioSymbol x('-', QByteArray(), 1.);

    QByteArray string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+=";
    const char *data = string.constData();
    while (*data)
    {
        QCOMPARE(x.hasCharacter(*data), false);
        ++data;
    }

    x.setCharacters("AAAA");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('a'), false);
    QCOMPARE(x.hasCharacter('B'), false);

    x.setCharacters("ACGTN");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.hasCharacter('N'), true);
    QCOMPARE(x.hasCharacter('F'), false);
    QCOMPARE(x.hasCharacter('0'), false);
    QCOMPARE(x.hasCharacter('*'), false);

    x.setCharacters("AC  GT");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.hasCharacter(' '), true);
    QCOMPARE(x.hasCharacter('F'), false);
    QCOMPARE(x.hasCharacter('0'), false);
    QCOMPARE(x.hasCharacter('*'), false);

    BioSymbol x2('-', QByteArray("AC"), 0.);
    QCOMPARE(x2.hasCharacter('A'), true);
    QCOMPARE(x2.hasCharacter('C'), true);
    QCOMPARE(x2.hasCharacter('G'), false);
    QCOMPARE(x2.hasCharacter('T'), false);
}

void TestBioSymbol::addCharacters()
{
    BioSymbol x('-', QByteArray(), 0.);

    QCOMPARE(x.hasCharacter('A'), false);
    QCOMPARE(x.hasCharacter('C'), false);
    x.addCharacters("AC");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    QCOMPARE(x.hasCharacter('0'), false);
    QCOMPARE(x.hasCharacter('-'), false);
    x.addCharacters("A");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    x.addCharacters("GT ");
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.hasCharacter(' '), true);
    QCOMPARE(x.hasCharacter('*'), false);
    QCOMPARE(x.hasCharacter('O'), false);

    QVERIFY(x.characterSet().contains('A'));
    QVERIFY(x.characterSet().contains('C'));
    QVERIFY(x.characterSet().contains('G'));
    QVERIFY(x.characterSet().contains('T'));
    QVERIFY(x.characterSet().contains(' '));
    QCOMPARE(x.characterSet().size(), 5);

    BioSymbol x2('-', QByteArray("AC"), 0.);
    QCOMPARE(x2.hasCharacter('A'), true);
    QCOMPARE(x2.hasCharacter('C'), true);
    QCOMPARE(x2.hasCharacter('G'), false);
    QCOMPARE(x2.hasCharacter('T'), false);
    x2.addCharacters("GT ");
    QCOMPARE(x2.hasCharacter('G'), true);
    QCOMPARE(x2.hasCharacter('T'), true);
    QCOMPARE(x2.hasCharacter(' '), true);
    QCOMPARE(x2.characterSet().size(), 5);
}

void TestBioSymbol::removeCharacters()
{
    BioSymbol x('-', QByteArray(), 0.);

    x.addCharacters("ACGT");
    QCOMPARE(x.hasCharacter('N'), false);
    x.removeCharacters("N");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.hasCharacter('N'), false);
    QCOMPARE(x.characterSet().size(), 4);

    x.removeCharacters("C");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), false);
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.characterSet().size(), 3);

    x.removeCharacters("GGAA");
    QCOMPARE(x.hasCharacter('A'), false);
    QCOMPARE(x.hasCharacter('C'), false);
    QCOMPARE(x.hasCharacter('G'), false);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.characterSet().size(), 1);

    x.addCharacters("xyz");
    x.removeCharacters("Tz");
    QCOMPARE(x.hasCharacter('x'), true);
    QCOMPARE(x.hasCharacter('y'), true);
    QCOMPARE(x.hasCharacter('T'), false);
    QCOMPARE(x.hasCharacter('z'), false);
    QCOMPARE(x.characterSet().size(), 2);

    BioSymbol x2('-', QByteArray("AC"), 0.);
    QCOMPARE(x2.hasCharacter('A'), true);
    QCOMPARE(x2.hasCharacter('C'), true);
    x2.removeCharacters("A ");
    QCOMPARE(x2.hasCharacter('A'), false);
    QCOMPARE(x2.hasCharacter('C'), true);
    QCOMPARE(x2.hasCharacter(' '), false);
    QCOMPARE(x2.characterSet().size(), 1);
}

void TestBioSymbol::setCharacters()
{
    BioSymbol x('-', QByteArray(), 0.);
    x.addCharacters("AC");
    QCOMPARE(x.hasCharacter('A'), true);
    QCOMPARE(x.hasCharacter('C'), true);
    QCOMPARE(x.characterSet().size(), 2);

    x.setCharacters("GTN");
    QCOMPARE(x.hasCharacter('A'), false);
    QCOMPARE(x.hasCharacter('C'), false);
    QCOMPARE(x.hasCharacter('G'), true);
    QCOMPARE(x.hasCharacter('T'), true);
    QCOMPARE(x.hasCharacter('N'), true);
    QCOMPARE(x.characterSet().size(), 3);

    x.setCharacters(QByteArray());
    QCOMPARE(x.characterSet().size(), 0);
    QCOMPARE(x.characters(), QByteArray());
}

void TestBioSymbol::setSymbol()
{
    BioSymbol x('-', QByteArray(), 0.);
    QCOMPARE(x.symbol(), '-');

    QByteArray symbols = "AbcDEFghijkLMNOPQAURTYS1232534@#$";
    const char *data = symbols.constData();
    while (*data)
    {
        x.setSymbol(*data);
        QCOMPARE(x.symbol(), *data);
        ++data;
    }
}

void TestBioSymbol::setThreshold()
{
    BioSymbol x('-', QByteArray(), 0.);
    QCOMPARE(x.threshold(), 0.);

    for (qreal i=0.; i< 1.; i += .01)
    {
        x.setThreshold(i);
        QCOMPARE(x.threshold(), i);
    }
}

void TestBioSymbol::operator_eqeq()
{
    BioSymbol x('-', QByteArray(), 0);
    QVERIFY(x == x);

    BioSymbol y('-', QByteArray(), 0);
    QVERIFY(x == y);

    BioSymbol z('-', "A", 1);
    QVERIFY(x != z);

    y.setCharacters("ACGT");
    y.setThreshold(.5);
    z.setCharacters("ACGT");
    z.setThreshold(.5);

    QVERIFY(y == z);

    z.setSymbol('X');
    QVERIFY(y != z);
}



QTEST_APPLESS_MAIN(TestBioSymbol)
#include "TestBioSymbol.moc"
