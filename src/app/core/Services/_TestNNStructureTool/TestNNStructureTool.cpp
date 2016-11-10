/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../NNStructureTool.h"
#include "../../Parsers/PssmParser.h"
#include "../../PODs/Pssm.h"
#include "../../PODs/NormalizedPssm.h"
#include "../../PODs/Q3Prediction.h"
#include "../../util/PssmUtil.h"

static const QString nnFile1 = "../../../resources/data/nn-sec-stage1.net";
static const QString nnFile2 = "../../../resources/data/nn-sec-stage2.net";

class TestNNStructureTool : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void predictSecondary();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestNNStructureTool::constructor()
{
    try
    {
        NNStructureTool x(nnFile1, nnFile2);
    }
    catch (...)
    {
        QVERIFY(0);
    }
}

void TestNNStructureTool::predictSecondary()
{
    NNStructureTool x(nnFile1, nnFile2);

    // Test: empty pssm returned empty secondary prediction
    QCOMPARE(x.predictSecondary(NormalizedPssm()), Q3Prediction());

    // Test: real data!
    PssmParser parser;
    Pssm pssm = parser.parseFile("files/1gvn_B.pssm");
    NormalizedPssm nPssm = ::normalizePssm(pssm);
    Q3Prediction pred = x.predictSecondary(nPssm);

    double expectedConfidence[287] = {
        0.9839, 0.8512, 0.8565, 0.8253, 0.8478, 0.8397, 0.8845, 0.8985, 0.8339, 0.9195,
        0.9327, 0.9671, 0.9640, 0.9492, 0.9200, 0.9579, 0.9519, 0.9232, 0.9122, 0.9524,
        0.9369, 0.8669, 0.8633, 0.8077, 0.5381, 0.7126, 0.8238, 0.8401, 0.6721, 0.4930,
        0.5696, 0.8707, 0.8237, 0.7206, 0.9565, 0.9737, 0.9770, 0.9519, 0.5405, 0.8521,
        0.8821, 0.8705, 0.7698, 0.6035, 0.4983, 0.7654, 0.8509, 0.9295, 0.9644, 0.9652,
        0.9686, 0.9644, 0.9748, 0.9631, 0.8994, 0.7370, 0.5335, 0.8029, 0.9590, 0.9422,
        0.8901, 0.9650, 0.9649, 0.9169, 0.5854, 0.5189, 0.6081, 0.6599, 0.9255, 0.8276,
        0.8137, 0.6982, 0.8269, 0.7227, 0.6238, 0.5896, 0.8236, 0.8584, 0.9351, 0.8886,
        0.8038, 0.5672, 0.8761, 0.8241, 0.7013, 0.5220, 0.6334, 0.7884, 0.8868, 0.7875,
        0.7570, 0.8044, 0.8340, 0.8735, 0.9517, 0.9627, 0.9706, 0.9699, 0.9749, 0.9747,
        0.9749, 0.9725, 0.9777, 0.9733, 0.9808, 0.9753, 0.9418, 0.8208, 0.6992, 0.9529,
        0.9405, 0.8205, 0.9034, 0.9653, 0.9640, 0.8932, 0.6014, 0.7849, 0.7350, 0.8366,
        0.9392, 0.7220, 0.7966, 0.8711, 0.9604, 0.9732, 0.9680, 0.9590, 0.9717, 0.9708,
        0.9852, 0.9681, 0.9258, 0.8180, 0.6779, 0.9513, 0.9366, 0.7782, 0.9738, 0.9785,
        0.9788, 0.9843, 0.9749, 0.9705, 0.8408, 0.7256, 0.8480, 0.8376, 0.8019, 0.8126,
        0.8491, 0.9298, 0.9525, 0.9059, 0.9540, 0.9545, 0.9606, 0.9627, 0.9713, 0.9701,
        0.9667, 0.9058, 0.8507, 0.6925, 0.5383, 0.7562, 0.9065, 0.8806, 0.8540, 0.7467,
        0.6989, 0.6004, 0.6990, 0.7740, 0.7629, 0.7168, 0.8045, 0.7191, 0.6890, 0.7611,
        0.7994, 0.8289, 0.8492, 0.8026, 0.8854, 0.9324, 0.9622, 0.9743, 0.9860, 0.9865,
        0.9788, 0.9537, 0.8091, 0.6041, 0.7514, 0.8434, 0.7746, 0.5238, 0.5117, 0.7679,
        0.9423, 0.9707, 0.9710, 0.9556, 0.5432, 0.8191, 0.9489, 0.8513, 0.5857, 0.7976,
        0.8204, 0.5982, 0.5072, 0.4785, 0.5420, 0.5709, 0.8098, 0.9109, 0.8297, 0.6337,
        0.7346, 0.7095, 0.8931, 0.9531, 0.9597, 0.9699, 0.9698, 0.9707, 0.9583, 0.8477,
        0.6755, 0.7675, 0.6837, 0.6871, 0.7568, 0.7539, 0.7844, 0.8394, 0.9540, 0.9631,
        0.9638, 0.9587, 0.9669, 0.9639, 0.9628, 0.9636, 0.9599, 0.9316, 0.8906, 0.8043,
        0.7673, 0.6193, 0.5617, 0.4856, 0.6994, 0.7932, 0.8609, 0.7653, 0.8084, 0.6591,
        0.8724, 0.9501, 0.9639, 0.9658, 0.9633, 0.9580, 0.9658, 0.9556, 0.8799, 0.6754,
        0.7208, 0.8394, 0.8632, 0.8451, 0.7275, 0.5827, 0.5798, 0.6765, 0.7383, 0.8718,
        0.8648, 0.7954, 0.8578, 0.8724, 0.9183, 0.9446, 0.9800
    };
    QByteArray q3 = "LLLLLLLLHHHHHHHHHHHHHHHHHLLLLLLLLEEEEELLLLLLLHHHHHHHHHHHLLLLEEEELLLHHHHHLLLHHHHHHHLLLLHHHHHHHHHHHHHHHHHHHHHHLLLLEEEELLLLLHHHHHHHHHHHHHLLLEEEEEEEELLHHHHHHHHHHHHHHHHHLLLLLLLLLLHHHHHHHHHHHHHHHHHHHHLLLLLEEEEEELLLLEEELLLLLLLLLHHHHHHHHHHLLLLHHHHHHHHHHHHHHHHHHLLLLLLHHHHHHHHHHHLLLLLLLLLLLLLLLLL";

    QCOMPARE(pred.q3_, q3);
    QCOMPARE(pred.confidence_.size(), 287);
    for (int i=0; i<287; ++i)
        QVERIFY(fabs(pred.confidence_.at(i) - expectedConfidence[i]) < .0001);
}


QTEST_APPLESS_MAIN(TestNNStructureTool)
#include "TestNNStructureTool.moc"
