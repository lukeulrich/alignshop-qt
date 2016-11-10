/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "RebaseParser.h"

class TestRebaseParser: public QObject
{
    Q_OBJECT

private slots:
    void parseRebaseFile();
    void parseRebaseFile_data();
};


//Public methods
/**check parsed data for a series of known enzymes
  */
void TestRebaseParser::parseRebaseFile_data()
{

    QTest::addColumn<QString>("enzymeName");
    QTest::addColumn<QString>("expectedRecognitionSite");
    QTest::addColumn<int>("expectedCut1Position");
    QTest::addColumn<int>("expectedCut2Position");
    QTest::addColumn<QString>("bluntOrSticky");

    QTest::newRow("AlwNI") << "AlwNI" << "CAGNNNCTG" << 6 << 3 << "sticky";
    QTest::newRow("BamHI") << "BamHI" << "GGATCC" << 1 << 5 << "sticky";
    QTest::newRow("BlpI") << "BlpI" << "GCTNAGC" << 2 << 5 << "blunt";

}

void TestRebaseParser::parseRebaseFile()
{
    RebaseParser b;

    QList<restrictionEnzyme> enzymes= b.parseRebaseFile();

        for(int i =0; i < enzymes.size(); ++i)
        {
           QFETCH(QString, enzymeName);
           QFETCH(QString, expectedRecognitionSite);
           QFETCH(int, expectedCut1Position);
           QFETCH(int, expectedCut2Position);
           QFETCH(QString, bluntOrSticky);

           if(enzymes[i].name_ == enzymeName)
           {
               QCOMPARE(enzymes[i].recognitionSequence_, expectedRecognitionSite);
               QCOMPARE(enzymes[i].cut_pos1_, expectedCut1Position);
               QCOMPARE(enzymes[i].cut_pos2_, expectedCut2Position);
               QCOMPARE(enzymes[i].bluntOrSticky_, bluntOrSticky);
           }
       }
}


QTEST_MAIN(TestRebaseParser)
#include "TestRebaseParser.moc"
