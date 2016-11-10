/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QFontInfo>
#include <QtGui/QScrollBar>
#include <typeinfo>

#include "AbstractMsaView.h"
#include "MockMsaView.h"
#include "Msa.h"
#include "PositionalMsaColorProvider.h"

class TestAbstractMsaView : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void setPositionalMsaColorProvider();
    void setFont();
    void setMode();
    void zoomFactor();
    void zoomMinMax();
    void setZoom();
    void setZoomBy();
    void setMsa();
    void editCursorPoint();
    void clipRect_data();
    void clipRect();
    void clipRectMsaChanges(); // Clip rect should be modified in response to msa changes (e.g. insertion/removal of sequences).
    void scrollHorz();
    void scrollVert();
    void scrollHorzVert();
    void resizeHorz();
    void resizeVert();
    void resizeHorzVert();

    // Protected draw functionality
    void drawAll();

    // Private utility functions
    void canvasSize();
    void setMsaRegionClip();
    void clipRenderOrigin();

    void scrollPosition();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
  * Mock class for testing the PositionalMsaColorProvider functionality
  */
class MockPositionalMsaColorProvider : public PositionalMsaColorProvider
{
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
void TestAbstractMsaView::constructor()
{
    MockMsaView x;

    x.show();

    QCOMPARE(x.clipRect().topLeft(), QPointF(0, 0));
    QVERIFY(x.clipRect().isNull());
    QVERIFY(x.msa() == 0);
    QVERIFY(x.mode() == AbstractMsaView::PanMode);
    QCOMPARE(x.font().family(), QString("Monospace"));
    QVERIFY(x.positionalMsaColorProvider());
    QVERIFY(QString(typeid(x.positionalMsaColorProvider()).name()).contains("PositionalMsaColorProvider"));
    QVERIFY(x.msaSelectionRect().isNull());
    QCOMPARE(x.zoom(), 1.);
    QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
    QCOMPARE(x.verticalScrollBar()->minimum(), 0);
    QCOMPARE(x.horizontalScrollBar()->maximum(), 0);
    QCOMPARE(x.verticalScrollBar()->maximum(), 0);
    QVERIFY(x.horizontalScrollBar()->isVisible() == false);
    QVERIFY(x.verticalScrollBar()->isVisible() == false);
}

void TestAbstractMsaView::setPositionalMsaColorProvider()
{
    MockMsaView x;

    const PositionalMsaColorProvider *defaultProvider = x.positionalMsaColorProvider();

    // ------------------------------------------------------------------------
    // Test: Custom positional color provider
    MockPositionalMsaColorProvider *pmcp = new MockPositionalMsaColorProvider();
    x.setPositionalMsaColorProvider(pmcp);
    QVERIFY(x.positionalMsaColorProvider() == pmcp);

    // ------------------------------------------------------------------------
    // Test: Set to same positional color provider
    x.setPositionalMsaColorProvider(pmcp);
    QVERIFY(x.positionalMsaColorProvider() == pmcp);

    // ------------------------------------------------------------------------
    // Test: reset to default
    x.setPositionalMsaColorProvider(0);
    QVERIFY(QString(typeid(x.positionalMsaColorProvider()).name()).contains("PositionalMsaColorProvider"));
    QVERIFY(x.positionalMsaColorProvider() == defaultProvider);
}

void TestAbstractMsaView::setFont()
{
    MockMsaView x;

    QSignalSpy spyFontChanged(&x, SIGNAL(fontChanged()));

    // ------------------------------------------------------------------------
    // Test: default font
    QCOMPARE(x.font().family(), QString("Monospace"));

    // ------------------------------------------------------------------------
    // Test: set to non-monospace fonts
    QStringList nonMonoFonts;
    nonMonoFonts << "Arial"
                 << "Times New Roman"
                 << "Comic Sans MS"
                 << "Georgia"
                 << "Tahoma";

    foreach (QString nonMonoFont, nonMonoFonts)
    {
        QFont font(nonMonoFont);
        QFontInfo fi(font);
        QCOMPARE(fi.fixedPitch(), false);

        x.setFont(font);

        // Font should not have changed
        QCOMPARE(x.font().family(), nonMonoFont);

        // No signal should have been emitted
        QCOMPARE(spyFontChanged.count(), 1);
        spyFontChanged.clear();
    }

    // ------------------------------------------------------------------------
    // Test: change to monospace fonts
    QStringList monoFonts;
    monoFonts << "Andale Mono"
              << "Cheq"
              << "Consolas"
              << "Courier"
              << "Courier New"
              << "Inconsolata"
              << "Terminal"
              << "Monospace";

    foreach (QString monoFont, monoFonts)
    {
        QFont font(monoFont);
        QFontInfo fi(font);
        QCOMPARE(fi.fixedPitch(), true);

        QVERIFY(x.font() != font);

        x.setFont(font);

        // Font should not have changed
        QCOMPARE(x.font().family(), monoFont);

        // No signal should have been emitted
        QCOMPARE(spyFontChanged.count(), 1);
        spyFontChanged.clear();
    }

    // ------------------------------------------------------------------------
    // Test: setting the exact same font (valid) will return true, but not emit fontChanged signal
    x.setFont(QFont("Monospace"));
    spyFontChanged.clear();

    x.setFont(QFont("Monospace"));
    QCOMPARE(spyFontChanged.count(), 0);

    // ------------------------------------------------------------------------
    // Test: changing other font parameters both returns true and emits the signal
    QFont font = QFont("Monospace", 6);
    QVERIFY(font != x.font());
    x.setFont(font);
    QCOMPARE(spyFontChanged.count(), 1);
    QCOMPARE(x.font(), font);
    spyFontChanged.clear();

    font = QFont("Monospace", 6, 6);
    x.setFont(font);
    QCOMPARE(spyFontChanged.count(), 1);
    QCOMPARE(x.font(), font);
    spyFontChanged.clear();

    // ------------------------------------------------------------------------
    // Test: changing the font should update the scroll bar steps
    x.setFont(QFont("Monospace", 64));
    QCOMPARE(x.horizontalScrollBar()->singleStep(), qRound(3. * x.charWidth()));
    QCOMPARE(x.verticalScrollBar()->singleStep(), qRound(3. * x.charHeight()));
}

void TestAbstractMsaView::setMode()
{
    QList<AbstractMsaView::OperatingMode> modes;
    modes << AbstractMsaView::PanMode
          << AbstractMsaView::EditMode
          << AbstractMsaView::ZoomMode;
    foreach (AbstractMsaView::OperatingMode mode, modes)
    {
        MockMsaView x;
        QSignalSpy spyModeChanged(&x, SIGNAL(modeChanged()));
        if (x.mode() == mode)
        {
            x.setMode(mode);
            QVERIFY(spyModeChanged.isEmpty());
            QCOMPARE(x.mode(), mode);
        }
        else
        {
            x.setMode(mode);
            QCOMPARE(spyModeChanged.count(), 1);
            QCOMPARE(x.mode(), mode);
            spyModeChanged.clear();
        }
    }
}


void TestAbstractMsaView::zoomFactor()
{
    MockMsaView x;

    x.setZoomFactor(.5);
    QCOMPARE(x.zoomFactor(), .5);

    x.setZoomFactor(3.);
    QCOMPARE(x.zoomFactor(), 3.);
}

void TestAbstractMsaView::zoomMinMax()
{
    MockMsaView x;

    qreal oldZoomMinimum = x.zoomMinimum();
    QVERIFY(oldZoomMinimum > 0.);

    // ------------------------------------------------------------------------
    // Test: zoom minimum at or below 0 is not allowed
    x.setZoomMinimum(0.);
    QCOMPARE(x.zoomMinimum(), oldZoomMinimum);
    x.setZoomMinimum(-5);
    QCOMPARE(x.zoomMinimum(), oldZoomMinimum);

    // ------------------------------------------------------------------------
    // Test: normal zoom minimum
    x.setZoomMinimum(.23);
    QCOMPARE(x.zoomMinimum(), .23);

    // ------------------------------------------------------------------------
    // Test: normal zoom maximum
    x.setZoomMaximum(5.);
    QCOMPARE(x.zoomMaximum(), 5.);

    // ------------------------------------------------------------------------
    // Test: min and max values that push the other extreme
    x.setZoomMinimum(6.);
    QCOMPARE(x.zoomMinimum(), 6.);
    QCOMPARE(x.zoomMaximum(), 6.);

    x.setZoomMaximum(3.);
    QCOMPARE(x.zoomMinimum(), 3.);
    QCOMPARE(x.zoomMaximum(), 3.);
}

void TestAbstractMsaView::setZoom()
{
    MockMsaView x;

    QSignalSpy spyZoomChanged(&x, SIGNAL(zoomChanged(double)));
    QVariantList spyArguments;

    x.setZoomMinimum(.1);
    x.setZoomMaximum(10.);

    // ------------------------------------------------------------------------
    // Test: no msa defined, no zoom should be accepted
    x.setZoom(3.);
    x.setZoom(1.);
    x.setZoom(2.);
    x.setZoom(4.);
    QCOMPARE(x.zoom(), 1.);
    QVERIFY(spyZoomChanged.isEmpty());

    // Setup: add msa
    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);
    QVERIFY(x.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: no change in the zoom
    x.setZoom(1.);
    QCOMPARE(x.zoom(), 1.);
    QVERIFY(spyZoomChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: increase the zoom
    x.setZoom(2.);
    QCOMPARE(x.zoom(), 2.);
    QCOMPARE(spyZoomChanged.count(), 1);
    spyArguments = spyZoomChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toDouble(), 2.);

    // ------------------------------------------------------------------------
    // Test: increase the zoom
    x.setZoom(.5);
    QCOMPARE(x.zoom(), .5);
    QCOMPARE(spyZoomChanged.count(), 1);
    spyArguments = spyZoomChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toDouble(), .5);

    // ------------------------------------------------------------------------
    // Test: changing the zoom level should update the scroll bar steps
    x.setZoom(3.);
    QCOMPARE(x.zoom(), 3.);
    QCOMPARE(x.horizontalScrollBar()->singleStep(), qRound(3. * x.charWidth()));
    QCOMPARE(x.verticalScrollBar()->singleStep(), qRound(3. * x.charHeight()));

    // ------------------------------------------------------------------------
    // Test: setting zoom below minimum should clamp it to the minimum zoom value
    x.setZoomMinimum(1.);
    x.setZoomMaximum(5.);
    x.setZoom(2.);
    QCOMPARE(x.zoom(), 2.);
    x.setZoom(.5);
    QCOMPARE(x.zoom(), 1.);
    x.setZoom(6.);
    QCOMPARE(x.zoom(), 5.);
}

void TestAbstractMsaView::setZoomBy()
{
    MockMsaView x;

    x.setZoomMinimum(.01);
    x.setZoomMaximum(1000.);

    qreal oldZoom = x.zoom();
    QCOMPARE(oldZoom, 1.);

    // ------------------------------------------------------------------------
    // Test: setting zoom by should do nothing without valid msa defined
    x.setZoomBy(1);
    x.setZoom(0);
    x.setZoom(-3);
    x.setZoom(8);

    QCOMPARE(x.zoom(), oldZoom);

    // Setup: add msa
    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);
    QVERIFY(x.msa() == msa);

    // ------------------------------------------------------------------------
    // Test: single step with step factor
    x.setZoomFactor(1.);        // Double it each time
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 2.);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 4.);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 8.);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 4.);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 2.);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 1.);

    x.setZoomFactor(.5);        // Step up by half the value
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 1.5);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 2.25);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 3.375);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 2.25);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 1.5);
    x.setZoomBy(-1);
    QCOMPARE(x.zoom(), 1.);

    // ------------------------------------------------------------------------
    // Test: multi step
    x.setZoomFactor(1.);
    x.setZoomBy(2);
    QCOMPARE(x.zoom(), 4.);
    x.setZoomBy(3);
    QCOMPARE(x.zoom(), 32.);
    x.setZoomBy(-5);
    QCOMPARE(x.zoom(), 1.);

    // ------------------------------------------------------------------------
    // Test: 10% of each zoom
    x.setZoomFactor(.1);
    x.setZoom(100.);
    x.setZoomBy(-1);
    if (fabs(x.zoom() - (100. / 1.1)) > .0001)
        QVERIFY(0);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 100.);
    x.setZoomBy(-2);
    if (fabs(x.zoom() - (100. / (1.1 * 1.1))) > .0001)
        QVERIFY(0);
    x.setZoomBy(2);
    QCOMPARE(x.zoom(), 100.);
    x.setZoomBy(1);
    QCOMPARE(x.zoom(), 110.);

    // ------------------------------------------------------------------------
    // Test: different zoom factor
    x.setZoom(1.);
    x.setZoomBy(0);
    QCOMPARE(x.zoom(), 1.);
}

void TestAbstractMsaView::setMsa()
{
    MockMsaView x;

    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    QSignalSpy spyMsaChanged(&x, SIGNAL(msaChanged()));

    // ------------------------------------------------------------------------
    // Test: default should be empty msa
    QVERIFY(x.msa() == 0);

    // ------------------------------------------------------------------------
    // Test: setMsa to new Msa
    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);
    QVERIFY(x.msa() == msa);
    QCOMPARE(spyMsaChanged.count(), 1);
    spyMsaChanged.clear();

    // Check that the signals are hooked up properly
    QVERIFY(disconnect(msa, SIGNAL(gapColumnsInserted(int,int)), &x, SLOT(onMsaGapColumnsInserted(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(gapColumnsRemoved(int)), &x, SLOT(onMsaGapColumnsRemoved(int))));
    QVERIFY(disconnect(msa, SIGNAL(msaReset()), &x, SLOT(onMsaReset())));
    QVERIFY(disconnect(msa, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int,int,int,int))));
    QVERIFY(disconnect(msa, SIGNAL(extendOrTrimFinished(int,int)), &x, SLOT(onMsaExtendOrTrimFinished(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(collapsedLeft(MsaRect,int)), &x, SLOT(onMsaCollapsedLeft(MsaRect,int))));
    QVERIFY(disconnect(msa, SIGNAL(collapsedRight(MsaRect,int)), &x, SLOT(onMsaCollapsedRight(MsaRect,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsSorted()), &x, SLOT(onMsaSubseqsSorted())));

    // Check that the scrollbar values are set appropriately
    QVERIFY(x.clipRect().width() < x.viewport()->width());
    QVERIFY(x.clipRect().height() < x.viewport()->height());
    QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
    QCOMPARE(x.verticalScrollBar()->minimum(), 0);
    QCOMPARE(x.horizontalScrollBar()->maximum(), 0);
    QCOMPARE(x.verticalScrollBar()->maximum(), 0);

    // ------------------------------------------------------------------------
    // Test: setting msa to zero should automatically disconnect signals
    x.setMsa(0);                // First clear the msa
    QVERIFY(x.msa() == 0);      // Check that it is no longer associated
    QCOMPARE(spyMsaChanged.count(), 1);
    spyMsaChanged.clear();
    x.setMsa(msa);              // Again call setMsa to reconnect the signals
    QVERIFY(x.msa() == msa);    // Check that the new one is hooked up
    x.setMsa(0);                // disconnect it again
    QVERIFY(x.msa() == 0);      // Check that it is no longer associated
    QCOMPARE(spyMsaChanged.count(), 2);
    spyMsaChanged.clear();

    // Now, all the disconnect signals should fail
    QCOMPARE(disconnect(msa, SIGNAL(gapColumnsInserted(int,int)), &x, SLOT(onMsaGapColumnsInserted(int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(gapColumnsRemoved(int)), &x, SLOT(onMsaGapColumnsRemoved(int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(msaReset()), &x, SLOT(onMsaReset())), false);
    QCOMPARE(disconnect(msa, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int,int,int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(extendOrTrimFinished(int,int)), &x, SLOT(onMsaExtendOrTrimFinished(int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(collapsedLeft(MsaRect,int)), &x, SLOT(onMsaCollapsedLeft(MsaRect,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(collapsedRight(MsaRect,int)), &x, SLOT(onMsaCollapsedRight(MsaRect,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))), false);
    QCOMPARE(disconnect(msa, SIGNAL(subseqsSorted()), &x, SLOT(onMsaSubseqsSorted())), false);

    // ------------------------------------------------------------------------
    // Test: setting the msa, should set the default selection rect to the very first character
    QVERIFY(x.msaSelectionRect().isNull());
    x.setMsa(msa);
    QCOMPARE(x.msaSelectionRect(), Rect(QPoint(1, 1), QPoint(1, 1)));
    x.setMsa(0);

    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: creating msa larger than can be rendered on the screen should configure the scrollbars values appropriately
    msa = createMsaLargerThan(cw, ch, x.viewport()->size(), 10, 10);
    QVERIFY(msa);
    x.setMsa(msa);

    QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
    QCOMPARE(x.verticalScrollBar()->minimum(), 0);
    QCOMPARE(x.horizontalScrollBar()->maximum(), x.canvasSize().width() - x.viewport()->width());
    QCOMPARE(x.verticalScrollBar()->maximum(), x.canvasSize().height() - x.viewport()->height());
    QCOMPARE(x.horizontalScrollBar()->singleStep(), qRound(3. * cw));
    QCOMPARE(x.verticalScrollBar()->singleStep(), qRound(3. * ch));
    QCOMPARE(x.horizontalScrollBar()->pageStep(), x.viewport()->width());
    QCOMPARE(x.verticalScrollBar()->pageStep(), x.viewport()->height());

    // ------------------------------------------------------------------------
    // Test: when changing the msa, it should reset the scroll position to 0, 0 and the zoom should be reset to 1.
    x.horizontalScrollBar()->setValue(5);
    x.verticalScrollBar()->setValue(5);
    QCOMPARE(x.horizontalScrollBar()->value(), 5);
    QCOMPARE(x.verticalScrollBar()->value(), 5);
    QCOMPARE(x.clipRect().topLeft(), QPointF(5, 5));
    x.setZoom(2.);
    QCOMPARE(x.zoom(), 2.);

    x.setMsa(0);
    QCOMPARE(x.horizontalScrollBar()->value(), 0);
    QCOMPARE(x.verticalScrollBar()->value(), 0);
    QCOMPARE(x.horizontalScrollBar()->maximum(), 0);
    QCOMPARE(x.verticalScrollBar()->maximum(), 0);
    QCOMPARE(x.zoom(), 1.);

    x.setMsa(msa);
    x.horizontalScrollBar()->setValue(5);
    x.verticalScrollBar()->setValue(5);
    QCOMPARE(x.horizontalScrollBar()->value(), 5);
    QCOMPARE(x.verticalScrollBar()->value(), 5);
    QCOMPARE(x.clipRect().topLeft(), QPointF(5, 5));

    Msa *msa2 = createMsaLargerThan(cw, ch, x.viewport()->size(), 5, 8);
    QVERIFY(msa2);
    x.setMsa(msa2);
    QCOMPARE(x.horizontalScrollBar()->value(), 0);
    QCOMPARE(x.verticalScrollBar()->value(), 0);
    QCOMPARE(x.clipRect().size().toSize(), x.viewport()->size());

    x.setMsa(0);

    delete msa;
    msa = 0;
    delete msa2;
    msa2 = 0;
}

void TestAbstractMsaView::editCursorPoint()
{
    MockMsaView x;

    // ------------------------------------------------------------------------
    // Test: edit cursor point without msa
    QVERIFY(x.editCursorPoint().isNull());

    // ------------------------------------------------------------------------
    // Test: edit cursor point with empty msa
    {
        Msa *msa = new Msa();
        x.setMsa(msa);
        QVERIFY(x.editCursorPoint().isNull());

        x.setMsa(0);
        delete msa;
        msa = 0;
    }

    // ------------------------------------------------------------------------
    // Test: edit cursor point with non-empty msa - default state = 1,1
    Msa *msa = createMsa(QStringList() << "ABC");
    QVERIFY(msa);
    x.setMsa(msa);
    QCOMPARE(x.editCursorPoint(), QPoint(1, 1));

    delete msa;
    msa = 0;
}

void TestAbstractMsaView::clipRect_data()
{
    QTest::addColumn<Msa *>("msa");
    QTest::addColumn<bool>("isNull");
    QTest::addColumn<QRectF>("clipRect");

    MockMsaView x;
    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    Msa *msa = 0;
    QTest::newRow("no msa defined") << msa << true << QRectF();
    QTest::newRow("empty msa") << new Msa() << true << QRectF();
    msa = createMsa(QStringList() << "ABC");
    QSize expectedSize(qMin(x.viewport()->width(), static_cast<int>(ceil(msa->length() * cw))),
                       qMin(x.viewport()->height(), static_cast<int>(ceil(msa->subseqCount() * ch))));
    QTest::newRow("1 sequence msa") << msa << false << QRectF(QPoint(0, 0), expectedSize);
    msa = createMsa(QStringList() << "ABC" << "DEF");
    expectedSize = QSize(qMin(x.viewport()->width(), static_cast<int>(ceil(msa->length() * cw))),
                         qMin(x.viewport()->height(), static_cast<int>(ceil(msa->subseqCount() * ch))));
    QTest::newRow("2 sequence msa") << msa << false << QRectF(QPoint(0, 0), expectedSize);


    int horzThresh = static_cast<int>(x.viewport()->width() / cw);
    expectedSize = QSize(x.viewport()->width(), static_cast<int>(ceil(ch)));
    QTest::newRow("longer subseq than can fit in horizontal space")
            << createMsa(QStringList() << QString("A").repeated(horzThresh + 5))
            << false << QRectF(QPoint(0, 0), expectedSize);

    int vertThresh = static_cast<int>(x.viewport()->height() / ch);
    QStringList subseqs;
    for (int i=0; i< vertThresh + 10; ++i)
        subseqs << "A";

    expectedSize = QSize(static_cast<int>(ceil(cw)), x.viewport()->height());
    QTest::newRow("more subseqs than can fit in vertical space")
            << createMsa(subseqs) << false << QRectF(QPoint(0, 0), expectedSize);

    QTest::newRow("completely filled viewport")
            << createMsaLargerThan(cw, ch, x.viewport()->size(), 5, 10) << false << QRectF(QPoint(0, 0), x.viewport()->size());
}

void TestAbstractMsaView::clipRect()
{
    MockMsaView x;

    QFETCH(Msa *, msa);
    QFETCH(bool, isNull);
    QFETCH(QRectF, clipRect);

    x.setMsa(msa);

    QCOMPARE(x.clipRect().isNull(), isNull);
    QCOMPARE(x.clipRect(), clipRect);

    x.setMsa(0);
    if (msa)
    {
        delete msa;
        msa = 0;
    }
}

void TestAbstractMsaView::clipRectMsaChanges()
{
    /*
    MockMsaView x;
    QSignalSpy spyClipSizeChanged(&x, SIGNAL(clipSizeChanged(QSize,QSize)));
    QVariantList spyArguments;

    qreal cw = x.fontCharPixmapProvider_->width('M');
    qreal ch = x.fontCharPixmapProvider_->height();

    Msa *msa = createMsa(QStringList() << "ABC");
    QVERIFY(msa);
    x.setMsa(msa);

    // ------------------------------------------------------------------------
    // Test: remove only sequence, should update the cliprect to invalid
    QSize oldSize(qMin(x.viewport()->width(), static_cast<int>(ceil(msa->length() * cw))),
                  qMin(x.viewport()->height(), static_cast<int>(ceil(msa->subseqCount() * ch))));
    QSize expectedSize(0, 0);
    msa->clear();
    QCOMPARE(msa->subseqCount(), 0);

    QCOMPARE(x.clipRect(), QRect(QPoint(0, 0), expectedSize));
    QCOMPARE(spyClipSizeChanged.count(), 1);
    spyArguments = spyClipSizeChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toSize(), oldSize);
    QCOMPARE(spyArguments.at(1).toSize(), expectedSize);

    // ------------------------------------------------------------------------
    // Test: add 1-2 sequences, should update the cliprect as necessary
    AnonSeq anonSeq(9, "ABCABC");
    Subseq *subseq1 = new Subseq(anonSeq);
    subseq1->setStop(3);
    Subseq *subseq2 = new Subseq(anonSeq);
    subseq2->setStart(4);
    subseq2->setStop(6);

    QVERIFY(msa->append(subseq1));
    oldSize = QSize(0, 0);
    expectedSize = QSize(qMin(x.viewport()->width(), static_cast<int>(ceil(msa->length() * cw))),
                         qMin(x.viewport()->height(), static_cast<int>(ceil(msa->subseqCount() * ch))));

    QCOMPARE(x.clipRect(), QRect(QPoint(0, 0), expectedSize));

    QCOMPARE(spyClipSizeChanged.count(), 1);
    spyArguments = spyClipSizeChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toSize(), oldSize);
    QCOMPARE(spyArguments.at(1).toSize(), expectedSize);

    // Now add another subseq
    oldSize = expectedSize;
    QVERIFY(msa->append(subseq2));
    expectedSize = QSize(qMin(x.viewport()->width(), static_cast<int>(ceil(msa->length() * cw))),
                         qMin(x.viewport()->height(), static_cast<int>(ceil(msa->subseqCount() * ch))));

    QCOMPARE(x.clipRect(), QRect(QPoint(0, 0), expectedSize));

    QCOMPARE(spyClipSizeChanged.count(), 1);
    spyArguments = spyClipSizeChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toSize(), oldSize);
    QCOMPARE(spyArguments.at(1).toSize(), expectedSize);
    */
}

void TestAbstractMsaView::scrollHorz()
{
    MockMsaView x;
    qreal cw = x.charWidth();

    int nHorzVisChars = static_cast<int>(ceil(x.viewport()->width() / cw));

    Msa *msa = createMsa(QStringList() << QString("A").repeated(nHorzVisChars + 10));
    QVERIFY(msa);
    x.setMsa(msa);

    // ------------------------------------------------------------------------
    // Test: Should not be able to setValue below minimum
    QRectF oldClipRect = x.clipRect();
    x.horizontalScrollBar()->setValue(-1);
    QCOMPARE(x.clipRect(), oldClipRect);
    QCOMPARE(x.horizontalScrollBar()->value(), 0);

    // ------------------------------------------------------------------------
    // Test: scroll in the positive direction
    QVERIFY(x.horizontalScrollBar()->maximum() > 0);
    for (int i=1, z=x.horizontalScrollBar()->maximum(); i<=z; ++i)
    {
        x.horizontalScrollBar()->setValue(i);
        QCOMPARE(x.clipRect(), QRectF(i, 0, x.viewport()->width(), x.canvasSize().height()));
    }

    // ------------------------------------------------------------------------
    // Test: Should not be able to setValue above maximum
    oldClipRect = x.clipRect();
    x.horizontalScrollBar()->setValue(x.horizontalScrollBar()->maximum() + 5);
    QCOMPARE(x.clipRect(), oldClipRect);

    // ------------------------------------------------------------------------
    // Test: scroll in the negative direction
    for (int i=x.horizontalScrollBar()->maximum(); i>=0; --i)
    {
        x.horizontalScrollBar()->setValue(i);
        QCOMPARE(x.clipRect(), QRectF(i, 0, x.viewport()->width(), x.canvasSize().height()));
    }

    x.setMsa(0);
    delete msa;
    msa = 0;
}

void TestAbstractMsaView::scrollVert()
{
    MockMsaView x;
    qreal ch = x.charHeight();

    int nVertVisChars = static_cast<int>(ceil(x.viewport()->height() / ch));

    QStringList subseqs;
    for (int i=0; i< nVertVisChars + 10; ++i)
        subseqs << "B";
    Msa *msa = createMsa(subseqs);
    QVERIFY(msa);
    x.setMsa(msa);

    // ------------------------------------------------------------------------
    // Test: Should not be able to setValue below minimum
    QRectF oldClipRect = x.clipRect();
    x.verticalScrollBar()->setValue(-1);
    QCOMPARE(x.clipRect(), oldClipRect);
    QCOMPARE(x.verticalScrollBar()->value(), 0);

    // ------------------------------------------------------------------------
    // Test: scroll in the positive direction
    QVERIFY(x.verticalScrollBar()->maximum() > 0);
    for (int i=1, z=x.verticalScrollBar()->maximum(); i<=z; ++i)
    {
        x.verticalScrollBar()->setValue(i);
        QCOMPARE(x.clipRect(), QRectF(0, i, x.canvasSize().width(), x.viewport()->height()));
    }

    // ------------------------------------------------------------------------
    // Test: Should not be able to setValue above maximum
    oldClipRect = x.clipRect();
    x.verticalScrollBar()->setValue(x.verticalScrollBar()->maximum() + 5);
    QCOMPARE(x.clipRect(), oldClipRect);

    // ------------------------------------------------------------------------
    // Test: scroll in the negative direction
    for (int i=x.verticalScrollBar()->maximum(); i>=0; --i)
    {
        x.verticalScrollBar()->setValue(i);
        QCOMPARE(x.clipRect(), QRectF(0, i, x.canvasSize().width(), x.viewport()->height()));
    }

    x.setMsa(0);
    delete msa;
    msa = 0;
}

void TestAbstractMsaView::scrollHorzVert()
{
    MockMsaView msaView;
    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 15, 15);
    QVERIFY(msa);
    msaView.setMsa(msa);

    // ------------------------------------------------------------------------
    // Test: scroll in all possible directions
    for (int y=1, yz=msaView.verticalScrollBar()->maximum(); y<yz; ++y)
    {
        msaView.verticalScrollBar()->setValue(y);
        for (int x=1, xz=msaView.horizontalScrollBar()->maximum(); x<xz; ++x)
        {
            msaView.horizontalScrollBar()->setValue(x);
            QCOMPARE(msaView.clipRect(), QRectF(x, y, msaView.viewport()->width(), msaView.viewport()->height()));
        }
    }

    msaView.setMsa(0);
    delete msa;
    msa = 0;
}

// Testing of the resize event is a bit tricky given that the viewport widget is contained within the abstractscrollarea
// parent widget. Normally, the abstractscrollarea would be resized, which would then cascade a resizeEvent to the
// viewport widget, and finally call the resizeEvent for the viewport area. Because our widget is not actually
// visually displayed and taking events, we have to simulate them manually. This is accomplished by directly calling
// resizeEvent of AbstractMsaView with the sizes we want to test.
//
// !!! Decided to simply show the actual window so we have event processing. The above no longer applies
void TestAbstractMsaView::resizeHorz()
{
    MockMsaView x;

    x.show();

    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    int nHorzVisChars = static_cast<int>(ceil(x.viewport()->width() / cw));

    Msa *msa = createMsa(QStringList() << QString("A").repeated(nHorzVisChars + 10));
    QVERIFY(msa);
    x.setMsa(msa);

    QSize originalSize = x.viewport()->size();

    // ------------------------------------------------------------------------
    // Test: ClipRect at position 0, viewport width from 50 .. canvasSize().width() + 50
    QVERIFY(nHorzVisChars * cw > 50);   // Make sure that 50 is less than the visible width of entire canvas
    QVERIFY(ch < 100);                  // Make sure that the single sequence is completely visible vertically
    for (int i=50, z=x.canvasSize().width() + 50; i<z; ++i)
    {
        x.resize(QSize(i, 100));
        QCOMPARE(x.clipRect(), QRectF(0, 0, qMin(x.canvasSize().width(), x.viewport()->width()), x.canvasSize().height()));

//        qDebug() << i << x.viewport()->width() << x.canvasSize() << spyClipSizeChanged;
        if (x.viewport()->width() <= x.canvasSize().width())
        {
            if (x.viewport()->width() < x.canvasSize().width())
                QVERIFY(x.horizontalScrollBar()->isVisible());

            QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(x.horizontalScrollBar()->maximum(), x.canvasSize().width() - x.viewport()->width());
            QCOMPARE(x.horizontalScrollBar()->pageStep(), x.viewport()->width());
        }
        else
        {
            QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(x.horizontalScrollBar()->maximum(), 0);
        }
    }

    // ------------------------------------------------------------------------
    // Test: ClipRect at position 50, viewport width from 100 .. canvasSize().width() + 50
    x.resize(originalSize);
    QVERIFY(nHorzVisChars * cw > 100);  // Make sure that 100 is less than the visible width of entire canvas
    x.horizontalScrollBar()->setValue(50);
    QCOMPARE(x.clipRect().left(), 50.);
    for (int i=100, z=x.canvasSize().width() + 50; i<z; ++i)
    {
        x.resize(QSize(i, 100));
        QCOMPARE(x.clipRect(), QRectF(x.horizontalScrollBar()->value(), 0, qMin(x.canvasSize().width(), x.viewport()->width()), x.canvasSize().height()));

        if (x.viewport()->width() <= x.canvasSize().width() - 50)
        {
            // Only the clipSize should be updated
            QCOMPARE(x.clipRect().left(), 50.);

            QVERIFY(x.horizontalScrollBar()->isVisible());
            QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(x.horizontalScrollBar()->maximum(), x.canvasSize().width() - x.viewport()->width());
            QCOMPARE(x.horizontalScrollBar()->pageStep(), x.viewport()->width());
        }
        else if (x.viewport()->width() <= x.canvasSize().width())
        {
            // Both the clipRect position and clipSize should be updated
            QCOMPARE(x.clipRect().left(), static_cast<double>(x.canvasSize().width() - x.viewport()->width()));

            if (x.viewport()->width() < x.canvasSize().width())
                QVERIFY(x.horizontalScrollBar()->isVisible());
            QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(x.horizontalScrollBar()->maximum(), x.canvasSize().width() - x.viewport()->width());
            QCOMPARE(x.horizontalScrollBar()->pageStep(), x.viewport()->width());
        }
        else
        {
            // viewport is wider than canvas size
            QCOMPARE(x.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(x.horizontalScrollBar()->maximum(), 0);
            QVERIFY(x.horizontalScrollBar()->isVisible() == false);
        }
    }
}

void TestAbstractMsaView::resizeVert()
{
    MockMsaView x;

    x.show();

    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    int nVertVisChars = static_cast<int>(ceil(x.viewport()->height() / ch));

    QStringList subseqs;
    for (int i=0; i< nVertVisChars + 10; ++i)
        subseqs << "B";
    Msa *msa = createMsa(subseqs);
    QVERIFY(msa);
    x.setMsa(msa);

    QSize originalSize = x.viewport()->size();

    // ------------------------------------------------------------------------
    // Test: ClipRect at position 0, viewport height from 50 .. canvasSize().height() + 50
    QVERIFY(nVertVisChars * ch > 50);   // Make sure that 50 is less than the visible height of entire canvas
    QVERIFY(cw < 100);                  // Make sure that the single sequence is completely visible horizontally
    for (int i=50, z=x.canvasSize().height() + 50; i<z; ++i)
    {
        x.resize(QSize(100, i));
        QCOMPARE(x.clipRect(), QRectF(0, 0, x.canvasSize().width(), qMin(x.canvasSize().height(), x.viewport()->height())));

        if (x.viewport()->height() <= x.canvasSize().height())
        {
            if (x.viewport()->height() < x.canvasSize().height())
                QVERIFY(x.verticalScrollBar()->isVisible());

            QCOMPARE(x.verticalScrollBar()->minimum(), 0);
            QCOMPARE(x.verticalScrollBar()->maximum(), x.canvasSize().height() - x.viewport()->height());
            QCOMPARE(x.verticalScrollBar()->pageStep(), x.viewport()->height());
        }
        else
        {
            QCOMPARE(x.verticalScrollBar()->minimum(), 0);
            QCOMPARE(x.verticalScrollBar()->maximum(), 0);
        }
    }

    // ------------------------------------------------------------------------
    // Test: ClipRect at position 50, viewport width from 100 .. canvasSize().width() + 50
    x.resize(originalSize);
    QVERIFY(nVertVisChars * ch > 100);  // Make sure that 100 is less than the visible width of entire canvas
    x.verticalScrollBar()->setValue(50);
    QCOMPARE(x.clipRect().top(), 50.);
    for (int i=100, z=x.canvasSize().height() + 50; i<z; ++i)
    {
        x.resize(QSize(100, i));
        QCOMPARE(x.clipRect(), QRectF(0, x.verticalScrollBar()->value(), x.canvasSize().width(), qMin(x.canvasSize().height(), x.viewport()->height())));

        if (x.viewport()->height() <= x.canvasSize().height() - 50)
        {
            // Only the clipSize should be updated
            QCOMPARE(x.clipRect().top(), 50.);

            QVERIFY(x.verticalScrollBar()->isVisible());
            QCOMPARE(x.verticalScrollBar()->minimum(), 0);
            QCOMPARE(x.verticalScrollBar()->maximum(), x.canvasSize().height() - x.viewport()->height());
            QCOMPARE(x.verticalScrollBar()->pageStep(), x.viewport()->height());
        }
        else if (x.viewport()->height() <= x.canvasSize().height())
        {
            // Both the clipRect position and clipSize should be updated
            QCOMPARE(x.clipRect().top(), static_cast<double>(x.canvasSize().height() - x.viewport()->height()));

            if (x.viewport()->height() < x.canvasSize().height())
                QVERIFY(x.verticalScrollBar()->isVisible());
            QCOMPARE(x.verticalScrollBar()->minimum(), 0);
            QCOMPARE(x.verticalScrollBar()->maximum(), x.canvasSize().height() - x.viewport()->height());
            QCOMPARE(x.verticalScrollBar()->pageStep(), x.viewport()->height());
        }
        else
        {
            // viewport is wider than canvas size
            QCOMPARE(x.verticalScrollBar()->minimum(), 0);
            QCOMPARE(x.verticalScrollBar()->maximum(), 0);
            QVERIFY(x.verticalScrollBar()->isVisible() == false);
        }
    }
}

void TestAbstractMsaView::resizeHorzVert()
{
    MockMsaView msaView;

    msaView.show();

    qreal cw = msaView.charWidth();
    qreal ch = msaView.charHeight();

    Msa *msa = createMsaLargerThan(cw, ch, msaView.viewport()->size(), 10, 10);
    QVERIFY(msa);
    msaView.setMsa(msa);

    QSize originalSize = msaView.viewport()->size();

    // ------------------------------------------------------------------------
    // Test: cliprect at (0, 0), increasing diagonally
    for (int i=50, z=msaView.canvasSize().height() + 50; i<z; ++i)
    {
        msaView.resize(QSize(i, i));
        QCOMPARE(msaView.clipRect(), QRectF(0,
                                            0,
                                            qMin(msaView.canvasSize().width(), msaView.viewport()->width()),
                                            qMin(msaView.canvasSize().height(), msaView.viewport()->height())));

        // Check horizontally
        if (msaView.viewport()->width() <= msaView.canvasSize().width())
        {
            if (msaView.viewport()->width() < msaView.canvasSize().width())
                QVERIFY(msaView.horizontalScrollBar()->isVisible());

            QCOMPARE(msaView.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.horizontalScrollBar()->maximum(), msaView.canvasSize().width() - msaView.viewport()->width());
            QCOMPARE(msaView.horizontalScrollBar()->pageStep(), msaView.viewport()->width());
        }
        else
        {
            QCOMPARE(msaView.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.horizontalScrollBar()->maximum(), 0);
        }

        // Check vertical
        if (msaView.viewport()->height() <= msaView.canvasSize().height())
        {
            if (msaView.viewport()->height() < msaView.canvasSize().height())
                QVERIFY(msaView.verticalScrollBar()->isVisible());

            QCOMPARE(msaView.verticalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.verticalScrollBar()->maximum(), msaView.canvasSize().height() - msaView.viewport()->height());
            QCOMPARE(msaView.verticalScrollBar()->pageStep(), msaView.viewport()->height());
        }
        else
        {
            QCOMPARE(msaView.verticalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.verticalScrollBar()->maximum(), 0);
        }
    }

    // ------------------------------------------------------------------------
    // Test: cliprect at (50, 50), increasing diagonally
    msaView.resize(originalSize);
    msaView.verticalScrollBar()->setValue(50);
    msaView.horizontalScrollBar()->setValue(50);
    QCOMPARE(msaView.clipRect().left(), 50.);
    QCOMPARE(msaView.clipRect().top(), 50.);

    for (int i=50, z=msaView.canvasSize().height() + 50; i<z; ++i)
    {
        msaView.resize(QSize(i, i));
        QCOMPARE(msaView.clipRect(), QRectF(msaView.horizontalScrollBar()->value(),
                                            msaView.verticalScrollBar()->value(),
                                            qMin(msaView.canvasSize().width(), msaView.viewport()->width()),
                                            qMin(msaView.canvasSize().height(), msaView.viewport()->height())));

        // ----------------
        // Check horizontal
        if (msaView.viewport()->width() <= msaView.canvasSize().width() - 50)
        {
            // Only the clipSize should be updated
            QCOMPARE(msaView.clipRect().left(), 50.);

            QVERIFY(msaView.horizontalScrollBar()->isVisible());
            QCOMPARE(msaView.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.horizontalScrollBar()->maximum(), msaView.canvasSize().width() - msaView.viewport()->width());
            QCOMPARE(msaView.horizontalScrollBar()->pageStep(), msaView.viewport()->width());
        }
        else if (msaView.viewport()->width() <= msaView.canvasSize().width())
        {
            // Both the clipRect position and clipSize should be updated
            QCOMPARE(msaView.clipRect().left(), static_cast<double>(msaView.canvasSize().width() - msaView.viewport()->width()));

            if (msaView.viewport()->width() < msaView.canvasSize().width())
                QVERIFY(msaView.horizontalScrollBar()->isVisible());
            QCOMPARE(msaView.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.horizontalScrollBar()->maximum(), msaView.canvasSize().width() - msaView.viewport()->width());
            QCOMPARE(msaView.horizontalScrollBar()->pageStep(), msaView.viewport()->width());
        }
        else
        {
            // viewport is wider than canvas size
            QCOMPARE(msaView.horizontalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.horizontalScrollBar()->maximum(), 0);
            QVERIFY(msaView.horizontalScrollBar()->isVisible() == false);
        }

        // ----------------
        // Check vertical
        if (msaView.viewport()->height() <= msaView.canvasSize().height() - 50)
        {
            // Only the clipSize should be updated
            QCOMPARE(msaView.clipRect().top(), 50.);

            QVERIFY(msaView.verticalScrollBar()->isVisible());
            QCOMPARE(msaView.verticalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.verticalScrollBar()->maximum(), msaView.canvasSize().height() - msaView.viewport()->height());
            QCOMPARE(msaView.verticalScrollBar()->pageStep(), msaView.viewport()->height());
        }
        else if (msaView.viewport()->height() <= msaView.canvasSize().height())
        {
            // Both the clipRect position and clipSize should be updated
            QCOMPARE(msaView.clipRect().top(), static_cast<double>(msaView.canvasSize().height() - msaView.viewport()->height()));

            if (msaView.viewport()->height() < msaView.canvasSize().height())
                QVERIFY(msaView.verticalScrollBar()->isVisible());
            QCOMPARE(msaView.verticalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.verticalScrollBar()->maximum(), msaView.canvasSize().height() - msaView.viewport()->height());
            QCOMPARE(msaView.verticalScrollBar()->pageStep(), msaView.viewport()->height());
        }
        else
        {
            // viewport is wider than canvas size
            QCOMPARE(msaView.verticalScrollBar()->minimum(), 0);
            QCOMPARE(msaView.verticalScrollBar()->maximum(), 0);

            // The following should actual be testable; however, without processing events it does not work :|
//            QVERIFY(msaView.verticalScrollBar()->isVisible() == false);
        }
    }
}

// Protected draw functionality
void TestAbstractMsaView::drawAll()
{
    MockMsaView x;

    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    x.show();

    // ------------------------------------------------------------------------
    // Test: paintEvent should call the drawAll function and only the drawAll function because the msa is not yet
    //       defined.
    QVERIFY(x.drawCalls_.isEmpty());
    x.paintEvent(0);
    QCOMPARE(x.drawCalls_.count(), 1);
    QCOMPARE(x.drawCalls_.at(0).funcName_, QString("drawAll"));
    x.drawCalls_.clear();

    // ------------------------------------------------------------------------
    // Test: with empty msa should only call drawAll as well
    Msa *msa = new Msa();
    x.setMsa(msa);
    QVERIFY(x.drawCalls_.isEmpty());
    x.paintEvent(0);
    QCOMPARE(x.drawCalls_.count(), 1);
    QCOMPARE(x.drawCalls_.at(0).funcName_, QString("drawAll"));
    x.drawCalls_.clear();
    x.setMsa(0);
    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: msa with scrollable area; selection is by default the edit cursor, which by default is the top left
    //       character of the msa.
    msa = createMsaLargerThan(cw, ch, x.viewport()->size(), 15, 15);
    QVERIFY(msa);
    x.setMsa(msa);
    QVERIFY(x.drawCalls_.isEmpty());
    x.paintEvent(0);
    QVERIFY2(x.drawCalls_.count() >= 6, QString("Received: %1").arg(x.drawCalls_.count()).toAscii());
    QCOMPARE(x.drawCalls_.at(0).funcName_, QString("drawAll"));
    QCOMPARE(x.drawCalls_.at(1).funcName_, QString("setMsaRegionClip"));
    QCOMPARE(x.drawCalls_.at(1).arguments_.at(0).value<MsaRect>(), x.pointRectMapper()->viewRectToMsaRect(x.viewport()->rect()));
    QCOMPARE(x.drawCalls_.at(2).funcName_, QString("drawBackground"));
    QCOMPARE(x.drawCalls_.at(3).funcName_, QString("drawMsa"));
    QCOMPARE(x.drawCalls_.at(3).arguments_.at(0).toPointF(), x.clipRenderOrigin());
    QCOMPARE(x.drawCalls_.at(3).arguments_.at(1).value<MsaRect>(), x.pointRectMapper()->viewRectToMsaRect(x.viewport()->rect()));

    // This is the problem to be fixed!
    QCOMPARE(x.drawCalls_.at(4).funcName_, QString("drawSelection"));
    QCOMPARE(x.drawCalls_.at(4).arguments_.at(0).toRectF(), QRectF(0, 0, cw, ch));
    QCOMPARE(x.drawCalls_.at(5).funcName_, QString("drawEditCursor"));
    QCOMPARE(x.drawCalls_.at(5).arguments_.at(0).toRectF(), QRectF(0, 0, cw, ch));
    x.drawCalls_.clear();

    // ------------------------------------------------------------------------
    // Test: scroll so that the first sequence is not visible and redraw
    QVERIFY(x.verticalScrollBar()->isVisible());
    QVERIFY(x.verticalScrollBar()->maximum() > static_cast<int>(ch) + 10);
    x.verticalScrollBar()->setValue(static_cast<int>(ch) + 10);
    QVERIFY(x.drawCalls_.isEmpty());
    x.paintEvent(0);
    QVERIFY(x.drawCalls_.count() >= 4);
    QCOMPARE(x.drawCalls_.at(0).funcName_, QString("drawAll"));
    QCOMPARE(x.drawCalls_.at(1).funcName_, QString("setMsaRegionClip"));
    QCOMPARE(x.drawCalls_.at(1).arguments_.at(0).value<MsaRect>(), x.pointRectMapper()->viewRectToMsaRect(x.viewport()->rect()));
    QCOMPARE(x.drawCalls_.at(2).funcName_, QString("drawBackground"));
    QCOMPARE(x.drawCalls_.at(3).funcName_, QString("drawMsa"));
    QCOMPARE(x.drawCalls_.at(3).arguments_.at(0).toPointF(), x.clipRenderOrigin());
    QCOMPARE(x.drawCalls_.at(3).arguments_.at(1).value<MsaRect>(), x.pointRectMapper()->viewRectToMsaRect(x.viewport()->rect()));
    x.drawCalls_.clear();
}

// Private utility functions
void TestAbstractMsaView::canvasSize()
{
    MockMsaView x;

    // ------------------------------------------------------------------------
    // Test: default canvas size with no msa set should be zero
    QCOMPARE(x.canvasSizeF(), QSizeF(0, 0));
    QCOMPARE(x.canvasSize(), QSize(0, 0));

    // ------------------------------------------------------------------------
    // Test: basic msa
    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);
    QVERIFY(x.msa() == msa);

    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    QCOMPARE(x.canvasSizeF(), QSizeF(cw * 3, ch * 3));
    QCOMPARE(x.canvasSize(), QSize(ceil(cw * 3), ceil(ch * 3)));

    // ------------------------------------------------------------------------
    // Test: change of font
    x.setFont(QFont("monospace", 64));
    cw = x.charWidth();
    ch = x.charHeight();

    QCOMPARE(x.canvasSizeF(), QSizeF(cw * 3, ch * 3));
    QCOMPARE(x.canvasSize(), QSize(ceil(cw * 3), ceil(ch * 3)));

    // ------------------------------------------------------------------------
    // Test: change of zoom
    x.setZoom(.5);
    cw = x.charWidth();
    ch = x.charHeight();

    QCOMPARE(x.canvasSizeF(), QSizeF(cw * 3., ch * 3.));
    QCOMPARE(x.canvasSize(), QSize(ceil(cw * 3), ceil(ch * 3)));

    // ------------------------------------------------------------------------
    // Test: increased width
    msa->insertGapColumns(1, 2);
    QCOMPARE(x.canvasSizeF(), QSizeF(cw * 5, ch * 3));
    QCOMPARE(x.canvasSize(), QSize(ceil(cw * 5), ceil(ch * 3)));

    // ------------------------------------------------------------------------
    // Test: increased number of subseqs
    AnonSeq aseq(1, "C");
    Subseq *subseq = new Subseq(aseq);
    QVERIFY(subseq->setBioString("----C"));
    QVERIFY(msa->append(subseq));
    QCOMPARE(x.canvasSizeF(), QSizeF(cw * 5, ch * 4));
    QCOMPARE(x.canvasSize(), QSize(ceil(cw * 5), ceil(ch * 4)));

    delete msa;
    msa = 0;
}

void TestAbstractMsaView::setMsaRegionClip()
{
    MockMsaView x;

    // ------------------------------------------------------------------------
    // Test: default region clip should be null - because there is no msa defined
    QVERIFY(x.msaRegionClip().isNull());

    // ------------------------------------------------------------------------
    // Test: set the region clip to a value and then attempt to retrieve it
    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);

    Rect expected(1, 2, 2, 1);
    x.setMsaRegionClip(expected);
    QCOMPARE(x.msaRegionClip(), expected);
}

void TestAbstractMsaView::clipRenderOrigin()
{
    MockMsaView x;
    qreal cw = x.charWidth();
    qreal ch = x.charHeight();

    x.show();

    // ------------------------------------------------------------------------
    // Test: Should always be zero without no msa defined
    QVERIFY(x.clipRenderOrigin().isNull());

    // ------------------------------------------------------------------------
    // Test: msa that is able to fit within default borders, should be zero and no scrolling permitted
    {
        Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
        QVERIFY(msa);
        x.setMsa(msa);

        QVERIFY(x.horizontalScrollBar()->isVisible() == false);
        QVERIFY(x.verticalScrollBar()->isVisible() == false);

        // The msaRegionClip_ (upon which clipRenderOrigin depends) is only updated in the drawAll function
        x.drawAll(0);
        QCOMPARE(x.clipRenderOrigin(), QPointF(0, 0));

        x.setMsa(0);
        delete msa;
        msa = 0;
    }

    // ------------------------------------------------------------------------
    // Test: horizontal clip origin
    Msa *msa = createMsaLargerThan(cw, ch, x.viewport()->size(), 15, 15);
    QVERIFY(msa);
    x.setMsa(msa);

    for (int i=1; i< x.horizontalScrollBar()->maximum(); ++i)
    {
        x.horizontalScrollBar()->setValue(i);
        // The msaRegionClip_ (upon which clipRenderOrigin depends) is only updated in the drawAll function
        x.drawAll(0);
        QCOMPARE(x.clipRenderOrigin(), QPointF(-(i - static_cast<int>(i / cw) * cw), 0));
    }
    x.horizontalScrollBar()->setValue(0);

    // ------------------------------------------------------------------------
    // Test: vertical clip origin
    for (int i=1; i< x.verticalScrollBar()->maximum(); ++i)
    {
        x.verticalScrollBar()->setValue(i);
        // The msaRegionClip_ (upon which clipRenderOrigin depends) is only updated in the drawAll function
        x.drawAll(0);
        QCOMPARE(x.clipRenderOrigin(), QPointF(0, -(i - static_cast<int>(i / ch) * ch)));
    }

    delete msa;
    msa = 0;
}

void TestAbstractMsaView::scrollPosition()
{
    MockMsaView x;
    qreal cw = x.charWidth();
    qreal ch = x.charHeight();
    x.show();

    // ------------------------------------------------------------------------
    // Test: Default state
    QCOMPARE(x.scrollPosition(), QPoint(x.horizontalScrollBar()->value(), x.verticalScrollBar()->value()));

    Msa *msa = createMsa(QStringList() << "ABC" << "AB-" << "-BC");
    QVERIFY(msa);
    x.setMsa(msa);

    // ------------------------------------------------------------------------
    // Test: valid msa
    QCOMPARE(x.scrollPosition(), QPoint(x.horizontalScrollBar()->value(), x.verticalScrollBar()->value()));

    // ------------------------------------------------------------------------
    // Test: msa with scrolling
    x.setMsa(0);
    delete msa;
    msa = createMsaLargerThan(cw, ch, x.viewport()->size(), 15, 15);
    QVERIFY(msa);
    x.setMsa(msa);

    x.horizontalScrollBar()->setValue(10);
    x.verticalScrollBar()->setValue(20);

    // Make sure that we actually scrolled somewhere
    QCOMPARE(x.horizontalScrollBar()->value(), 10);
    QCOMPARE(x.verticalScrollBar()->value(), 20);

    QCOMPARE(x.scrollPosition(), QPoint(x.horizontalScrollBar()->value(), x.verticalScrollBar()->value()));
}

QTEST_MAIN(TestAbstractMsaView)
#include "TestAbstractMsaView.moc"
