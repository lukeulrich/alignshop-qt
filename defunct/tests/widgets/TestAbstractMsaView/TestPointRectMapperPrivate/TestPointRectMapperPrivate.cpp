/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QtGlobal>
#include <QtGui/QFontInfo>
#include <QtGui/QScrollBar>
#include <typeinfo>
#include <cmath>

#include "AbstractMsaView.h"
#include "../MockMsaView.h"
#include "Msa.h"

class TestPointRectMapperPrivate : public QObject
{
    Q_OBJECT

private slots:
    void constructor();             // Tests {set,}abstractMsaView, {set,}resolution
    void canvasPointToMsaPoint();   // Also tests canvasPointToMsaPointF
    void canvasRectToMsaRect();
    void canvasRectFToMsaRect();
    void msaPointToCanvasPoint();
    void msaPointToCanvasRect();
    void msaRectToCanvasRect();
    void canvasPointToViewPoint();
    void canvasRectToViewRect();
    void viewPointToCanvasPoint();
    void viewRectToCanvasRect();

    // Convenience functions
    void viewPointToMsaPoint();     // Also tests the floating point version
    void msaPointToViewPoint();
    void msaPointToViewRect();
    void viewRectToMsaRect();
    void msaRectToViewRect();
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

Msa *createMsaLargerThan(qreal cw, qreal ch, QSize size, int extraHorz, int extraVert)
{
    int horzThresh = static_cast<int>(size.width() / cw);
    int vertThresh = static_cast<int>(size.height() / ch);

    QStringList subseqs;
    for (int i=0; i< vertThresh + extraVert; ++i)
        subseqs << QString("B").repeated(horzThresh + extraHorz);

    return createMsa(subseqs);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPointRectMapperPrivate::constructor()
{
    MockMsaView y;
    PointRectMapperPrivate x(0);

    QVERIFY(x.abstractMsaView() == 0);
    x.setAbstractMsaView(&y);
    QVERIFY(x.abstractMsaView() == &y);
    x.setAbstractMsaView(0);
    QVERIFY(x.abstractMsaView() == 0);
}

void TestPointRectMapperPrivate::canvasPointToMsaPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.canvasPointToMsaPoint(QPointF(34.9, 28.2)), QPoint());
    QCOMPARE(mapper.canvasPointToMsaPoint(QPointF(0, 0)), QPoint());
    QCOMPARE(mapper.canvasPointToMsaPoint(QPointF(1567.98, -34.20)), QPoint());
    QCOMPARE(mapper.canvasPointToMsaPointF(QPointF(34.9, 28.2)), QPointF());
    QCOMPARE(mapper.canvasPointToMsaPointF(QPointF(0, 0)), QPointF());
    QCOMPARE(mapper.canvasPointToMsaPointF(QPointF(1567.98, -34.20)), QPointF());

    // Setup
    mapper.setAbstractMsaView(&msaView);

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: every pixel within the canvas should map to the appropriate point in the Msa
    QSize canvasSize = msaView.canvasSize();
    for (int y=0; y< canvasSize.height(); ++y)
        for (int x=0; x< canvasSize.width(); ++x)
            QCOMPARE(mapper.canvasPointToMsaPoint(QPointF(x, y)), QPoint(1 + static_cast<int>(static_cast<double>(x) / cw),
                                                                         1 + static_cast<int>(static_cast<double>(y) / ch)));

    QSizeF canvasSizeF = msaView.canvasSizeF();
    for (qreal y=0; y< canvasSizeF.height(); y += .4)
        for (qreal x=0; x< canvasSizeF.width(); x += .4)
            QCOMPARE(mapper.canvasPointToMsaPointF(QPointF(x, y)), QPointF(1. + x / cw,
                                                                           1. + y / ch));
    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::canvasRectToMsaRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(-3, 0, 39, 23)), MsaRect());
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(0, 3, 4, 5)), MsaRect());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    double intPart;

    // Check that the charWidth and charHeight are some factor + .5
    QCOMPARE(modf(cw * 2., &intPart), 0.);
    QCOMPARE(modf(ch * 2., &intPart), 0.);

    // ------------------------------------------------------------------------
    // Test: explicit boundary tests
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw * 2, ch * 2))), MsaRect(1, 1, 2, 2));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw * 2 + 1, ch * 2))), MsaRect(1, 1, 3, 2));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw * 2, ch * 2 + 1))), MsaRect(1, 1, 2, 3));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw * 2 + 1, ch * 2 + 1))), MsaRect(1, 1, 3, 3));

    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw+1, ch))), MsaRect(1, 1, 2, 1));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw, ch+1))), MsaRect(1, 1, 1, 2));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(0, 0), QPoint(cw+1, ch+1))), MsaRect(1, 1, 2, 2));

    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(cw / 2., ch / 2.), QPoint(cw+1, ch))), MsaRect(1, 1, 2, 1));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(cw / 2., ch / 2.), QPoint(cw, ch+1))), MsaRect(1, 1, 1, 2));
    QCOMPARE(mapper.canvasRectToMsaRect(Rect(QPoint(cw / 2., ch / 2.), QPoint(cw+1, ch+1))), MsaRect(1, 1, 2, 2));

    // ------------------------------------------------------------------------
    // Test: every pixel within the canvas should map to the appropriate point in the Msa
    Rect input;
    Rect nInput;
    MsaRect expect;

    QSize canvasSize = msaView.canvasSize();
    for (qreal w=1; w< canvasSize.width(); ++w)
    {
        for (qreal h=1; h< canvasSize.height(); ++h)
        {
            // Quadrant I
            for (int x=0; x<canvasSize.width()-w; ++x)
                for (int y=0; y<canvasSize.height()-h; ++y)
                {
                    QCOMPARE(mapper.canvasRectToMsaRect(Rect(x,    y,    w,    h)),
                                                        MsaRect(QPoint(1 + static_cast<int>(x/cw), 1 + static_cast<int>(y/ch)),
                                                                QPoint(ceil((x+w) / cw), ceil((y+h) / ch))));
                }

            // Quadrant II
            for (int x=canvasSize.width()-1; x>=w; --x)
                for (int y=0; y<canvasSize.height()-h; ++y)
                {
                    input.setRect(x, y, -w, h);
                    nInput = input.normalized();
                    expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                   QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                    QCOMPARE(mapper.canvasRectToMsaRect(input), expect);
                }

            // Quadrant III
            for (int x=canvasSize.width()-1; x>=w; --x)
                for (int y=canvasSize.height()-1; y>=h; --y)
                {
                    input.setRect(x, y, -w, -h);
                    nInput = input.normalized();
                    expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                   QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                    QCOMPARE(mapper.canvasRectToMsaRect(input), expect);
                }

            // Quadrant IV
            for (int x=0; x<canvasSize.width()-w; ++x)
                for (int y=canvasSize.height()-1; y>=h; --y)
                {
                    input.setRect(x, y, w, -h);
                    nInput = input.normalized();
                    expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                   QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                    QCOMPARE(mapper.canvasRectToMsaRect(input), expect);
                }
        }
    }

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::canvasRectFToMsaRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(0, 0, 3.4, 9.2)), MsaRect());
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(-3.2, 0, 98.3, 19343.03)), MsaRect());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-D" << "-BCD" << "-BC-");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    // ------------------------------------------------------------------------
    // Test: zero width input should return default Rect()
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(.5, .5, 0, 10)), MsaRect());
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(.5, .5, 10, 0)), MsaRect());

    // ------------------------------------------------------------------------
    // Test: explicit boundary tests
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(0., 0., cw, ch)), MsaRect(1, 1, 1, 1));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(0., 0., cw + .0001, ch)), MsaRect(1, 1, 2, 1));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(0., 0., cw, ch + .0001)), MsaRect(1, 1, 1, 2));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(0., 0., cw + .0001, ch + .0001)), MsaRect(1, 1, 2, 2));

    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(QPointF(cw / 2., ch / 2.), QPointF(cw * 3, ch * 3))), MsaRect(1, 1, 3, 3));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(QPointF(cw / 2., ch / 2.), QPointF(cw * 3 + .0001, ch * 3))), MsaRect(1, 1, 4, 3));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(QPointF(cw / 2., ch / 2.), QPointF(cw * 3, ch * 3 + .0001))), MsaRect(1, 1, 3, 4));
    QCOMPARE(mapper.canvasRectFToMsaRect(QRectF(QPointF(cw / 2., ch / 2.), QPointF(cw * 3 + .0001, ch * 3 + .0001))), MsaRect(1, 1, 4, 4));

    // ------------------------------------------------------------------------
    // Test: every pixel within the canvas should map to the appropriate point in the Msa
    QRectF input;
    QRectF nInput;
    MsaRect expect;

    QSizeF canvasSize = msaView.canvasSizeF();
    for (qreal w=1.; w< canvasSize.width(); w += 1.3)
        for (qreal h=1.; h< canvasSize.height(); h += 1.3)
            for (qreal x=0; x<canvasSize.width()-w; x += .95)
                for (qreal y=0; y<canvasSize.height()-h; y += .95)
                {
                    // Quadrant I
                    input.setRect(x, y, w, h);
                    expect.setRect(QPoint(1 + static_cast<int>(x/cw), 1 + static_cast<int>(y/ch)),
                                   QPoint(ceil(input.right() / cw), ceil(input.bottom() / ch)));
//                    if (mapper.canvasRectFToMsaRect(input) != expect)
//                    {
//                        qDebug() << "Input" << input;
//                        qDebug() << "Expect" << expect;
//                        qDebug() << "Actual" << mapper.canvasRectFToMsaRect(input);
//                    }
                    QCOMPARE(mapper.canvasRectFToMsaRect(input), expect);

                    // Quadrant II
                    if (x >= w)
                    {
                        input.setRect(x, y, -w, h);
                        nInput = input.normalized();
                        expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                       QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                        QCOMPARE(mapper.canvasRectFToMsaRect(input), expect);
                    }

                    // Quadrant III
                    if (x >= w && y >= h)
                    {
                        input.setRect(x, y, -w, -h);
                        nInput = input.normalized();
                        expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                       QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                        QCOMPARE(mapper.canvasRectFToMsaRect(input), expect);
                    }

                    // Quadrant IV
                    if (y >= h)
                    {
                        input.setRect(x, y, w, -h);
                        nInput = input.normalized();
                        expect.setRect(QPoint(1 + static_cast<int>(nInput.x()/cw), 1 + static_cast<int>(nInput.y()/ch)),
                                       QPoint(ceil(nInput.right() / cw), ceil(nInput.bottom() / ch)));
                        QCOMPARE(mapper.canvasRectFToMsaRect(input), expect);
                    }
                }

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaPointToCanvasPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaPointToCanvasPoint(QPoint(1, 0)), QPointF());
    QCOMPARE(mapper.msaPointToCanvasPoint(QPoint(34, -2)), QPointF());
    QCOMPARE(mapper.msaPointToCanvasPoint(QPointF(1., 99.)), QPointF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    // ------------------------------------------------------------------------
    // Test: even with no msa defined, should return the correct result
    QCOMPARE(mapper.msaPointToCanvasPoint(QPoint(1, 1)), QPointF(0, 0));
    QCOMPARE(mapper.msaPointToCanvasPoint(QPointF(1.5, 1.5)), QPointF(cw * .5, ch * .5));

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: basic msa
    for (qreal x=1; x<= msa->length(); x += .5)
        for (qreal y=1; y<= msa->subseqCount(); y += .423)
            QCOMPARE(mapper.msaPointToCanvasPoint(QPointF(x, y)), QPointF((x-1.) * cw, (y-1.) * ch));

    // ------------------------------------------------------------------------
    // Test: Outside boundaries of Msa should work fine
    QCOMPARE(mapper.msaPointToCanvasPoint(QPoint(6, 6)), QPointF(5*cw, 5*ch));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaPointToCanvasRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaPointToCanvasRect(QPoint(1, 0)), QRectF());
    QCOMPARE(mapper.msaPointToCanvasRect(QPoint(34, -2)), QRectF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    // ------------------------------------------------------------------------
    // Test: even with no msa defined, should return the correct result
    QCOMPARE(mapper.msaPointToCanvasRect(QPoint(1, 1)), QRectF(0, 0, cw, ch));
    QCOMPARE(mapper.msaPointToCanvasRect(QPoint(3, 4)), QRectF(2 * cw, 3 * ch, cw, ch));

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: basic msa
    for (int x=1; x<= msa->length(); ++x)
        for (int y=1; y<= msa->subseqCount(); ++y)
            QCOMPARE(mapper.msaPointToCanvasRect(QPoint(x, y)), QRectF((x-1)*cw, (y-1)*ch, cw, ch));

    // ------------------------------------------------------------------------
    // Test: Outside boundaries of Msa should work fine
    QCOMPARE(mapper.msaPointToCanvasRect(QPoint(6, 6)), QRectF(5*cw, 5*ch, cw, ch));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaRectToCanvasRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(0, 0, 3, 4)), QRectF());
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(34, 23, 12, 5)), QRectF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    // ------------------------------------------------------------------------
    // Test: No msa defined, should still work as expected - all quadrants (mirrored vertically)
    //       Note, Msa coords are inclusive!
    // Quadrant I
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(QPoint(1, 1), QPoint(1, 1))), QRectF(0, 0, cw, ch));
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(1, 1, 1, 1)), QRectF(0, 0, 2 * cw, 2 * ch));
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(1, 1, 2, 1)), QRectF(0, 0, 3 * cw, 2 * ch));
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(1, 1, 1, 2)), QRectF(0, 0, 2 * cw, 3 * ch));
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(1, 1, 2, 2)), QRectF(0, 0, 3 * cw, 3 * ch));

    QCOMPARE(mapper.msaRectToCanvasRect(Rect(2, 1, 1, 1)), QRectF(cw, 0, 2 * cw, 2 * ch));
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(2, 1, -1, 0)), QRectF(0, 0, 2 * cw, ch));

    // Quadrant II
    QCOMPARE(mapper.msaRectToCanvasRect(Rect(2, 1, 1, 2)), QRectF(cw, 0,  2 * cw, 3 * ch));

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: Defined msa, normal valid rectangle, all quadrants, varying heights and width, all mirrored vertically
    //       from default cartesian system
    for (int w=0; w<= 3; ++w)
    {
        for (int h=0; h<= 3; ++h)
        {
            // Quadrant I
            for (int x=1; x<=4-w; ++x)  // X: 1..4; 1..3; 1..2; ...
                for (int y=1; y<=4-h; ++y)
                    QCOMPARE(mapper.msaRectToCanvasRect(Rect(x,        y,        w,    h)),
                                                        QRectF((x-1)*cw, (y-1)*ch, (w+1)*cw, (h+1)*ch));

            // Quadrant II
            for (int x=3; x>=(w+1); --x)
                for (int y=1; y<=4-h; ++y)
                    QCOMPARE(mapper.msaRectToCanvasRect(Rect(x,        y,       -w,    h)),
                                                        QRectF((x-1-w)*cw, (y-1)*ch, (w+1)*cw, (h+1)*ch));

            // Quadrant III
            for (int x=3; x>=(w+1); --x)
                for (int y=3; y>=(h+1); --y)
                    QCOMPARE(mapper.msaRectToCanvasRect(Rect(x,        y,       -w,   -h)),
                                                        QRectF((x-1-w)*cw, (y-1-h)*ch, (w+1)*cw, (h+1)*ch));

            // Quadrant IV
            for (int x=1; x<=4-w; ++x)
                for (int y=3; y>=(h+1); --y)
                    QCOMPARE(mapper.msaRectToCanvasRect(Rect(x,        y,        w,   -h)),
                                                        QRectF((x-1)*cw, (y-1-h)*ch, (w+1)*cw, (h+1)*ch));
        }
    }

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::canvasPointToViewPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.canvasPointToViewPoint(QPointF(84, 32)), QPointF());
    QCOMPARE(mapper.canvasPointToViewPoint(QPointF(-34, 23)), QPointF());
    QCOMPARE(mapper.canvasPointToViewPoint(QPointF(12453, 3423.)), QPointF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: unscrolled view - all returned points will simply be equal to the input
    QCOMPARE(msaView.horizontalScrollBar()->value(), 0);
    QCOMPARE(msaView.verticalScrollBar()->value(), 0);
    for (qreal x=0.; x< msaView.canvasSizeF().width(); x+= .5)
        for (qreal y=0.; y< msaView.canvasSizeF().height(); y += 1./3.)
             QCOMPARE(mapper.canvasPointToViewPoint(QPointF(x, y)), QPointF(x, y));

    // ------------------------------------------------------------------------
    // Test: scrolled view
    msaView.setMsa(0);
    delete msa;
    msa = 0;

    msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 20, 20);
    msaView.setMsa(msa);

    QVERIFY(msaView.horizontalScrollBar()->maximum() > 10);
    QVERIFY(msaView.verticalScrollBar()->maximum() > 20);

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    QVERIFY(canvasWidthF > msaView.viewport()->width());
    QVERIFY(canvasHeightF > msaView.viewport()->height());

    // Scroll a bit to the left and down
    msaView.horizontalScrollBar()->setValue(10);
    msaView.verticalScrollBar()->setValue(20);

    for (qreal x=0.; x< msaView.canvasSizeF().width(); x+= .6)
        for (qreal y=0.; y< msaView.canvasSizeF().height(); y += 1.2)
            QCOMPARE(mapper.canvasPointToViewPoint(QPointF(x, y)), QPointF(x - 10, y - 20));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::canvasRectToViewRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.canvasRectToViewRect(QRectF(1, 2, 3, 4)), QRectF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: unscrolled view - all returned points will simply be equal to the input
    QCOMPARE(msaView.horizontalScrollBar()->value(), 0);
    QCOMPARE(msaView.verticalScrollBar()->value(), 0);
    for (qreal x=0.; x< msaView.canvasSizeF().width() - 3.2; x+= .5)
        for (qreal y=0.; y< msaView.canvasSizeF().height() - 3.2; y += 1./3.)
             QCOMPARE(mapper.canvasRectToViewRect(QRectF(x, y, 3.2, 3.2)), QRectF(x, y, 3.2, 3.2));

    // ------------------------------------------------------------------------
    // Test: scrolled view
    msaView.setMsa(0);
    delete msa;
    msa = 0;

    msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 20, 20);
    msaView.setMsa(msa);

    QVERIFY(msaView.horizontalScrollBar()->maximum() > 10);
    QVERIFY(msaView.verticalScrollBar()->maximum() > 20);

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    QVERIFY(canvasWidthF > msaView.viewport()->width());
    QVERIFY(canvasHeightF > msaView.viewport()->height());

    // Scroll a bit to the left and down
    msaView.horizontalScrollBar()->setValue(10);
    msaView.verticalScrollBar()->setValue(20);

    for (qreal x=0.; x< msaView.canvasSizeF().width() - 3.2; x+= .6)
        for (qreal y=0.; y< msaView.canvasSizeF().height() - 3.2; y += 1.2)
            QCOMPARE(mapper.canvasRectToViewRect(QRectF(x, y, 3.2, 3.2)), QRectF(x-10, y-20, 3.2, 3.2));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::viewPointToCanvasPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(3., 4)), QPointF());
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(-98, 54)), QPointF());
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, 3000)), QPointF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: unscrolled view - all valid viewpoints within canvas
    QCOMPARE(msaView.horizontalScrollBar()->value(), 0);
    QCOMPARE(msaView.verticalScrollBar()->value(), 0);
    for (qreal x=0.; x< msaView.canvasSizeF().width() - 3.2; x+= .5)
        for (qreal y=0.; y< msaView.canvasSizeF().height() - 3.2; y += 1./3.)
             QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(x, y)), QPointF(x, y));

    // ------------------------------------------------------------------------
    // Test: outside boundaries should be clamped
    // Left side
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(-.0001, 0)), QPointF(0, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(-1, 0)), QPointF(0, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(-5, 0)), QPointF(0, 0));

    // Top
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, -.0001)), QPointF(0, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, -1)), QPointF(0, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, -5)), QPointF(0, 0));

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    // Right
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(canvasWidthF, 0)), QPointF(canvasWidthF - .0001, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(canvasWidthF + 1, 0)), QPointF(canvasWidthF - .0001, 0));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(canvasWidthF + 5, 0)), QPointF(canvasWidthF - .0001, 0));

    // Bottom
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, canvasHeightF)), QPointF(0, canvasHeightF - .0001));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, canvasHeightF + 1)), QPointF(0, canvasHeightF - .0001));
    QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(0, canvasHeightF + 5)), QPointF(0, canvasHeightF - .0001));

    // ------------------------------------------------------------------------
    // Test: scrolled view - all positive, visible viewpoints
    msaView.setMsa(0);
    delete msa;
    msa = 0;

    msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 20, 20);
    msaView.setMsa(msa);

    QVERIFY(msaView.horizontalScrollBar()->maximum() > 10);
    QVERIFY(msaView.verticalScrollBar()->maximum() > 20);

    canvasWidthF = msaView.canvasSizeF().width();
    canvasHeightF = msaView.canvasSizeF().height();

    QVERIFY(canvasWidthF > msaView.viewport()->width());
    QVERIFY(canvasHeightF > msaView.viewport()->height());

    // Scroll a bit to the left and down
    msaView.horizontalScrollBar()->setValue(10);
    msaView.verticalScrollBar()->setValue(20);

    // Span the entire area
    // -15 = -10 for scrollbar position - an additional 5 pixels
    for (qreal x=-15; x< canvasWidthF - 10 + 5; x += .6)
        for (qreal y=-25; y< canvasHeightF - 20 + 5; y += 1.2)
            QCOMPARE(mapper.viewPointToCanvasPoint(QPointF(x, y)),
                     QPointF(qBound(0., x+10, canvasWidthF-.0001), qBound(0., y+20, canvasHeightF-.0001)));
}

void TestPointRectMapperPrivate::viewRectToCanvasRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.viewRectToCanvasRect(QRectF(0, 1, 2, 3)), QRectF());
    QCOMPARE(mapper.viewRectToCanvasRect(QRectF(3, 2, 9, 18)), QRectF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    QCOMPARE(msaView.horizontalScrollBar()->value(), 0);
    QCOMPARE(msaView.verticalScrollBar()->value(), 0);

    qreal vh = msaView.viewport()->height();
    qreal vw = msaView.viewport()->width();

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    QVERIFY(vw > canvasWidthF + 5);
    QVERIFY(vh > canvasHeightF + 5);

    for (qreal h=0; h< canvasHeightF + 5; ++h)
        for (qreal w=0; w< canvasWidthF + 5; ++w)
            for (qreal y=-3; y<= h; y += .5)
                for (qreal x=-3; x<= w; x += .5)
                    QCOMPARE(mapper.viewRectToCanvasRect(QRectF(x, y, w, h)),
                             QRectF(mapper.viewPointToCanvasPoint(QPointF(x, y)),
                                    mapper.viewPointToCanvasPoint(QPointF(x + w, y + h))));

    // ------------------------------------------------------------------------
    // Test: viewport is < than canvas and scrolled
    msaView.setMsa(0);
    delete msa;
    msa = 0;

    msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 20, 20);
    msaView.setMsa(msa);

    QVERIFY(msaView.horizontalScrollBar()->maximum() > 10);
    QVERIFY(msaView.verticalScrollBar()->maximum() > 20);

    canvasWidthF = msaView.canvasSizeF().width();
    canvasHeightF = msaView.canvasSizeF().height();

    // Now that scrolling is in place, the viewport dimensions have changed
    vh = msaView.viewport()->height();
    vw = msaView.viewport()->width();

    QVERIFY(canvasWidthF > vw);
    QVERIFY(canvasHeightF > vh);

    // Scroll a bit to the left and down
    msaView.horizontalScrollBar()->setValue(10);
    msaView.verticalScrollBar()->setValue(20);

    for (qreal h=0; h< canvasHeightF + 5; h += 25)
        for (qreal w=0; w< canvasWidthF + 5; w += 25)
            for (qreal y=-25; y<= h; y += 4.5)
                for (qreal x=-15; x<= w; x += 10.5)
                    QCOMPARE(mapper.viewRectToCanvasRect(QRectF(x, y, w, h)),
                             QRectF(mapper.viewPointToCanvasPoint(QPointF(x, y)),
                                    mapper.viewPointToCanvasPoint(QPointF(x + w, y + h))));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::viewPointToMsaPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.viewPointToMsaPointF(QPointF(3, 3)), QPointF());
    QCOMPARE(mapper.viewPointToMsaPointF(QPointF(0, 0)), QPointF());

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    // ------------------------------------------------------------------------
    // Test: explicit tests
    QCOMPARE(mapper.viewPointToMsaPointF(QPointF(0, 0)), QPointF(1, 1));
    QCOMPARE(mapper.viewPointToMsaPointF(QPointF(cw, ch)), QPointF(2, 2));

    QCOMPARE(mapper.viewPointToMsaPoint(QPointF(0, 0)), QPoint(1, 1));
    QCOMPARE(mapper.viewPointToMsaPoint(QPointF(cw-1, ch-1)), QPoint(1, 1));
    QCOMPARE(mapper.viewPointToMsaPoint(QPointF(cw + 5, ch-1)), QPoint(2, 1));
    QCOMPARE(mapper.viewPointToMsaPoint(QPointF(cw-1, ch + 5)), QPoint(1, 2));
    QCOMPARE(mapper.viewPointToMsaPoint(QPointF(cw + 5, ch + 5)), QPoint(2, 2));

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    for (int x=-5; x< canvasWidthF+5; x++)
        for (int y=-5; y< canvasHeightF+5; y++)
        {
            QPointF expect = mapper.canvasPointToMsaPointF(mapper.viewPointToCanvasPoint(QPointF(x, y)));
            QCOMPARE(mapper.viewPointToMsaPointF(QPointF(x, y)), expect);
            QCOMPARE(mapper.viewPointToMsaPoint(QPointF(x, y)), floorPoint(expect));
        }

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaPointToViewPoint()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaPointToViewPoint(QPointF(3, 3)), QPointF());
    QCOMPARE(mapper.msaPointToViewPoint(QPointF(0, 0)), QPointF());

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    for (qreal x=1; x<= msa->length(); x += .5)
        for (qreal y=1; y<= msa->subseqCount(); y += .5)
            QCOMPARE(mapper.msaPointToViewPoint(QPointF(x, y)),
                     mapper.canvasPointToViewPoint(mapper.msaPointToCanvasPoint(QPointF(x, y))));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaPointToViewRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaPointToViewRect(QPoint(3, 3)), QRectF());
    QCOMPARE(mapper.msaPointToViewRect(QPoint(0, 0)), QRectF());

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    for (int x=1; x<= msa->length(); ++x)
        for (int y=1; y<= msa->subseqCount(); ++y)
            QCOMPARE(mapper.msaPointToViewRect(QPoint(x, y)),
                     mapper.canvasRectToViewRect(mapper.msaPointToCanvasRect(QPoint(x, y))));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::viewRectToMsaRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.viewRectToMsaRect(QRectF(0, 1, 2, 3)), MsaRect());

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    qreal canvasWidthF = msaView.canvasSizeF().width();
    qreal canvasHeightF = msaView.canvasSizeF().height();

    for (int x=-5; x< canvasWidthF+5; x++)
        for (int y=-5; y< canvasHeightF+5; y++)
            QCOMPARE(mapper.viewRectToMsaRect(QRectF(x, y, 10, 4)),
                     mapper.canvasRectFToMsaRect(mapper.viewRectToCanvasRect(QRectF(x, y, 10, 4))));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

void TestPointRectMapperPrivate::msaRectToViewRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaRectToViewRect(MsaRect(1, 1, 3, 3)), QRectF());

    Msa *msa = createMsa(QStringList() << "ABCD" << "AB-S" << "-BCD");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    mapper.setAbstractMsaView(&msaView);

    for (int i=1; i<= msa->subseqCount(); ++i)
        for (int j=1; j<= msa->length(); ++j)
            QCOMPARE(mapper.msaRectToViewRect(MsaRect(j, i, 1, 1)),
                     mapper.canvasRectToViewRect(mapper.msaRectToCanvasRect(MsaRect(j, i, 1, 1))));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

QTEST_MAIN(TestPointRectMapperPrivate)
#include "TestPointRectMapperPrivate.moc"



/*
void TestPointRectMapperPrivate::msaPointFToCanvasRect()
{
    MockMsaView msaView;
    PointRectMapperPrivate mapper(0);

    // ------------------------------------------------------------------------
    // Test: no abstract msa view configured, should return default values
    QCOMPARE(mapper.msaPointToCanvasRect(QPointF(1.7, 0)), QRectF());
    QCOMPARE(mapper.msaPointToCanvasRect(QPointF(34.3, -2.)), QRectF());

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    mapper.setAbstractMsaView(&msaView);

    // ------------------------------------------------------------------------
    // Test: even with no msa defined, should return the correct result
    QCOMPARE(mapper.msaPointToCanvasRect(QPointF(1.4, 1.8)), QRectF(0, 0, cw, ch));
    QCOMPARE(mapper.msaPointToCanvasRect(QPointF(3.1, 4.9)), QRectF(2 * cw, 3 * ch, cw, ch));

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    msaView.setMsa(msa);
    QVERIFY(msaView.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: basic msa
    for (qreal x=1; x<= msa->length(); x += .67)
        for (qreal y=1; y<= msa->subseqCount(); y += .5)
            QCOMPARE(mapper.msaPointToCanvasRect(QPointF(x, y)), QRectF((floor(x)-1)*cw, (floor(y)-1)*ch, cw, ch));

    // ------------------------------------------------------------------------
    // Test: Outside boundaries of Msa should work fine
    QCOMPARE(mapper.msaPointToCanvasRect(QPointF(6., 6.)), QRectF(5*cw, 5*ch, cw, ch));

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}
*/
