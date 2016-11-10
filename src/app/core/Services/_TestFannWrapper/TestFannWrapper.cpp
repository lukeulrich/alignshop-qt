/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QFile>

#include "../FannWrapper.h"

class TestFannWrapper : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void invalidNnFiles_data();
    void invalidNnFiles();
    void validNnFile();
    void runFann();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestFannWrapper::constructor()
{
    FannWrapper x;

    QVERIFY(x.neuralNetFile().isEmpty());
    QCOMPARE(x.nInputs(), 0);
    QCOMPARE(x.nOutputs(), 0);
    QCOMPARE(x.runFann(QVector<fann_type>()), QVector<double>());
    QCOMPARE(x.isValid(), false);
}

void TestFannWrapper::invalidNnFiles_data()
{
    QTest::addColumn<QString>("file");

    QTest::newRow("missing file") << "missing";
    QTest::newRow("image file") << "molecule.png";
    QTest::newRow("corrupt neural network file") << "corrupt.net";
}

void TestFannWrapper::invalidNnFiles()
{
    QFETCH(QString, file);

    FannWrapper x;

    file = "files/" + file;

    try
    {
        x.setNeuralNetFile(file);
        QVERIFY(0);
    }
    catch(QString &error)
    {
        QVERIFY(1);
        QCOMPARE(x.isValid(), false);
    }
    catch (...)
    {
        QVERIFY(0);
    }
}

void TestFannWrapper::validNnFile()
{
    FannWrapper x;

    QString file = "files/nn-sec-stage1.net";
    QVERIFY(QFile::exists(file));
    try
    {
        x.setNeuralNetFile(file);
        QCOMPARE(x.neuralNetFile(), file);

        QCOMPARE(x.nInputs(), 315);
        QCOMPARE(x.nOutputs(), 3);
        QVERIFY(x.isValid());
    }
    catch(...)
    {
        QVERIFY(0);
    }

    try
    {
        x.setNeuralNetFile("files/missing");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
        QVERIFY(x.neuralNetFile().isEmpty());
        QCOMPARE(x.isValid(), false);
    }
}

void TestFannWrapper::runFann()
{
    FannWrapper x;

    // Test: initialize neural network from valid file
    QString file = "files/nn-sec-stage1.net";
    try
    {
        x.setNeuralNetFile(file);
    }
    catch(...)
    {
        QVERIFY(0);
    }

    try
    {
        x.runFann(QVector<fann_type>());
        QVERIFY(0);
    }
    catch(...)
    {
        QVERIFY(1);
    }

    QVERIFY(x.isValid());

    // Test: actual data run
    fann_type rawInputs[315] = {
        0.2176, 0.1839, 0.0802, 0.1324, 0.3351, 0.0947, 0.1598, 0.4806, 0.1758, 0.7577, 0.9892, 0.1205, 0.1081, 0.2554, 0.1776, 0.1669, 0.2437, 0.4146, 0.1791, 0.2180, 0.0000,
        0.9020, 0.2059, 0.5585, 0.2516, 0.1189, 0.2607, 0.1835, 0.4675, 0.5112, 0.1694, 0.2083, 0.7120, 0.1931, 0.2483, 0.2087, 0.6781, 0.5162, 0.4286, 0.0883, 0.1419, 0.0000,
        0.1715, 0.7451, 0.9507, 0.5505, 0.0672, 0.1824, 0.2297, 0.0762, 0.6295, 0.0679, 0.1000, 0.8991, 0.1484, 0.2896, 0.2223, 0.4894, 0.2324, 0.0857, 0.0522, 0.0941, 0.0000,
        0.1522, 0.1232, 0.4195, 0.6905, 0.3561, 0.0668, 0.2010, 0.8406, 0.1440, 0.8172, 0.8289, 0.1133, 0.0907, 0.1888, 0.1284, 0.1338, 0.1747, 0.3481, 0.2137, 0.9093, 0.0000,
        0.5219, 0.1041, 0.7627, 0.7079, 0.1817, 0.1262, 0.7970, 0.3770, 0.7186, 0.2680, 0.1773, 0.2360, 0.1373, 0.6445, 0.2333, 0.4037, 0.6990, 0.3504, 0.1350, 0.8231, 0.0000,
        0.2104, 0.0915, 0.7820, 0.2675, 0.0811, 0.3095, 0.1986, 0.0771, 0.7685, 0.0748, 0.1125, 0.8578, 0.8198, 0.2592, 0.5757, 0.7690, 0.5244, 0.0960, 0.0677, 0.3787, 0.0000,
        0.2680, 0.1055, 0.0745, 0.3828, 0.9441, 0.0639, 0.1851, 0.2850, 0.1156, 0.7998, 0.3036, 0.0805, 0.3469, 0.1232, 0.3864, 0.1143, 0.1376, 0.5119, 0.2709, 0.9076, 0.0000,
        0.3740, 0.1659, 0.1821, 0.1854, 0.1195, 0.1571, 0.1474, 0.2096, 0.1978, 0.4663, 0.2075, 0.6502, 0.1519, 0.2002, 0.2973, 0.8716, 0.9123, 0.5909, 0.0855, 0.1312, 0.0000,
        0.5945, 0.0887, 0.9307, 0.7089, 0.0683, 0.1462, 0.1545, 0.2428, 0.4298, 0.0832, 0.1062, 0.4403, 0.8535, 0.2414, 0.3122, 0.5169, 0.4152, 0.1141, 0.0577, 0.0883, 0.0000,
        0.5162, 0.0680, 0.5162, 0.9051, 0.0640, 0.1133, 0.1951, 0.2038, 0.8119, 0.1802, 0.1350, 0.3515, 0.1335, 0.8038, 0.7389, 0.4195, 0.3274, 0.1062, 0.0654, 0.1011, 0.0000,
        0.4465, 0.0695, 0.7484, 0.9099, 0.0915, 0.5860, 0.1791, 0.2301, 0.5000, 0.3521, 0.1353, 0.2034, 0.1195, 0.7829, 0.1896, 0.4988, 0.1683, 0.2034, 0.0791, 0.5175, 0.0000,
        0.1321, 0.1007, 0.0771, 0.1096, 0.9343, 0.0663, 0.8168, 0.6916, 0.4323, 0.6142, 0.4682, 0.2511, 0.0736, 0.1422, 0.7531, 0.1153, 0.1459, 0.7685, 0.1598, 0.4969, 0.0000,
        0.5890, 0.0724, 0.6029, 0.7958, 0.0621, 0.3324, 0.6894, 0.0752, 0.3917, 0.1780, 0.1312, 0.6130, 0.5119, 0.8961, 0.7335, 0.2306, 0.6017, 0.0989, 0.0646, 0.1074, 0.0000,
        0.6405, 0.0811, 0.6428, 0.7970, 0.2006, 0.2428, 0.7186, 0.0895, 0.6770, 0.2419, 0.1315, 0.6474, 0.3340, 0.7441, 0.5424, 0.4055, 0.4963, 0.1751, 0.6207, 0.1256, 0.0000,
        0.7517, 0.4900, 0.0766, 0.1089, 0.5714, 0.0913, 0.0915, 0.9195, 0.3464, 0.4465, 0.5156, 0.0924, 0.0905, 0.2958, 0.6559, 0.2645, 0.3379, 0.6536, 0.0828, 0.1452, 0.0000
    };

    QVector<fann_type> inputs(315);
    memcpy(inputs.data(), rawInputs, sizeof(fann_type) * 315);
    QVector<double> result = x.runFann(inputs);

    QVERIFY(fabs(result.at(0) - .8435) < .0001);
    QVERIFY(fabs(result.at(1) - .1708) < .0001);
    QVERIFY(fabs(result.at(2) - .0) < .0001);
}

QTEST_APPLESS_MAIN(TestFannWrapper)
#include "TestFannWrapper.moc"
