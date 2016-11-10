/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "AbstractAlphabetDetector.h"

/**
  * To test the AbstractAlphabetDetector class, we must create a mock class that derives
  * from this super class and defines the detectAlphabet(...) method.
  */
class MockAlphabetDetector : public AbstractAlphabetDetector
{
public:
    // By default just have the detectAlphabet function return eUnknownAlphabet
    Alphabet detectAlphabet(const BioString &/*bioString*/) const
    {
        return eUnknownAlphabet;
    }
};

/**
  * There should be a 1:1 correspondence between the list of bioStrings passed into an AlphabetDetector
  * and the resulting list of alphabets returned by the detectAlphabet function. Thus, we define this
  * mock detector which simply returns the alphabet based on the biostring name.
  */
class MockAlphabetDetector2 : public AbstractAlphabetDetector
{
public:
    Alphabet detectAlphabet(const BioString &bioString) const
    {
        if (bioString == "DNA")
            return eDnaAlphabet;
        else if (bioString == "RNA")
            return eRnaAlphabet;
        else if (bioString == "AMINO")
            return eAminoAlphabet;
        else
            return eUnknownAlphabet;
    }
};


class TestAbstractAlphabetDetector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void inspector();            // also tests setInspectors
    void detectAlphabets();
    void orderedDetectAlphabets();
};

void TestAbstractAlphabetDetector::inspector()
{
    MockAlphabetDetector adet;
    QVERIFY(adet.inspectors().isEmpty());

    AlphabetInspector x(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));

    QList<AlphabetInspector> inspectors;
    inspectors.append(AlphabetInspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters)));
    inspectors.append(AlphabetInspector(eDnaAlphabet, BioStringValidator(constants::kDnaCharacters)));

    adet.setInspectors(inspectors);
    QVERIFY(inspectors == adet.inspectors());
}

void TestAbstractAlphabetDetector::detectAlphabets()
{
    MockAlphabetDetector adet;
    QList<BioString> biostrings;
    biostrings.append("ABC");
    biostrings.append("123");

    QList<Alphabet> result = adet.detectAlphabets(biostrings);
    QVERIFY(result.size() == 2);
    QVERIFY(result.at(0) == eUnknownAlphabet);
    QVERIFY(result.at(1) == eUnknownAlphabet);
}

void TestAbstractAlphabetDetector::orderedDetectAlphabets()
{
    MockAlphabetDetector2 adet;
    QList<BioString> biostrings;
    biostrings.append("DNA");
    biostrings.append("AMINO");
    biostrings.append("Unknown");
    biostrings.append("RNA");

    QList<Alphabet> result = adet.detectAlphabets(biostrings);
    QVERIFY(result.size() == 4);
    QVERIFY(result.at(0) == eDnaAlphabet);
    QVERIFY(result.at(1) == eAminoAlphabet);
    QVERIFY(result.at(2) == eUnknownAlphabet);
    QVERIFY(result.at(3) == eRnaAlphabet);
}

QTEST_MAIN(TestAbstractAlphabetDetector)
#include "TestAbstractAlphabetDetector.moc"
