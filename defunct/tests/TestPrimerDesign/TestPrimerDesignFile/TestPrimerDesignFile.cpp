/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore>
#include <PrimerDesign/PrimerDesignFile.h>

class TestPrimerDesignFile : public QObject
{
    Q_OBJECT

private:
    // For some reason, the regex (<ParamId>.*?</ParamId>) which works in JavaScript/C# didn't work in QT.
    QString replaceParamTag(const QString &data)
    {
        QString startTag = "<ParamId>";
        QString endTag = "</ParamId>";
        QString result = data.trimmed().replace("\r", "");
        for (int i = result.indexOf(startTag); i >= 0; i = result.indexOf(startTag))
        {
            result = result.replace(i, endTag.length() + result.indexOf(endTag) - i, "");
        }

        return result;
    }

private slots:
    void deserialize()
    {
        QFile file(":/xml/TestData.xml");
        file.open(QIODevice::ReadOnly);

        QTextStream reader(&file);
        QString data = reader.readAll();

        ObservablePrimerPairGroupList list;
        PrimerDesignFile::deserialize(list, data);

        QVERIFY(!data.isEmpty());
        QVERIFY(list.length() == 2);

        QVERIFY(list.at(0)->sequence.name() == "g seq");
        QVERIFY(list.at(0)->sequence.sequence() == "GGGGGGGGGGGGGGGGGGGGGGGGGGGG");
        QVERIFY(list.at(0)->sequence.notes() == "my notes");

        QVERIFY(list.at(1)->sequence.name() == "t seq");
        QVERIFY(list.at(1)->sequence.sequence() == "TTTTTTTTTTTTTTTT");
        QVERIFY(list.at(1)->sequence.notes() == "tt notes");

        QVERIFY(list.at(0)->pairs.length() == 3);
        QVERIFY(list.at(1)->pairs.length() == 1);

        PrimerPair *pair = list.at(0)->pairs.at(0);
        ParameterSet *param = pair->params();
        QVERIFY(param == list.at(0)->pairs.at(2)->params());

        QVERIFY(param->ampliconBounds().min() == 1);
        QVERIFY(param->ampliconBounds().max() == 100);
        QVERIFY(param->ampliconSizeRange().min() == 5);
        QVERIFY(param->ampliconSizeRange().max() == 50);
        QVERIFY(param->primerRange().min() == 10);
        QVERIFY(param->primerRange().max() == 50);
        QVERIFY(param->tmRange().min() == 55.5);
        QVERIFY(param->tmRange().max() == 65.6);
        QVERIFY(param->forwardPrefix() == "GGG");
        QVERIFY(param->reversePrefix() == "TTT");

        QVERIFY(pair->name() == "P1");
        QVERIFY(pair->forwardPrimer().sequence() == "CCGG");
        QVERIFY(pair->forwardPrimer().tm() == 51);
        QVERIFY(pair->reversePrimer().sequence() == "GGTT");
        QVERIFY(pair->reversePrimer().tm() == 51.5);

        pair = list.at(1)->pairs.at(0);
        param = pair->params();

        QVERIFY(param->forwardPrefix() == "CAT");
        QVERIFY(param->reversePrefix() == "GAA");
        QVERIFY(pair->name() == "P4");
        QVERIFY(pair->reversePrimer().tm() == 45.456);

        QString reserialized = PrimerDesignFile::serialize(list);
        reserialized = replaceParamTag(reserialized);
        qDebug() << reserialized;
        qDebug() << replaceParamTag(data);
        QVERIFY(replaceParamTag(data) == reserialized);
    }
};

QTEST_MAIN(TestPrimerDesignFile)
#include "TestPrimerDesignFile.moc"
