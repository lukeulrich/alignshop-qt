/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <cmath>

#include <QtCore/QMargins>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QUndoStack>

#include "../painting/NativeRenderEngine.h"

#include "AbstractMsaView.h"
#include "../MsaTools/IMsaTool.h"
#include "../painting/IRenderEngine.h"
#include "../gui_misc.h"
#include "../widgets/MsaStartStopSideWidget.h"
#include "../widgets/MsaRulerWidget.h"
#include "../widgets/VerticalMsaMarginWidget.h"
#include "../../core/ObservableMsa.h"
#include "../../core/global.h"
#include "../../core/misc.h"
#include "../../graphics/AbstractTextRenderer.h"
#include "../../graphics/ExactTextRenderer.h"          // To be removed!
#include "../../graphics/TextColorStyle.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QWidget *]
  */
AbstractMsaView::AbstractMsaView(QWidget *parent) :
    QAbstractScrollArea(parent),
    msa_(nullptr),
    positionalMsaColorProvider_(&defaultColorProvider_),
    undoStack_(nullptr),
    zoom_(1.),
    pointRectMapper_(this),
    zoomFactor_(.2),
    zoomMinimum_(.05),
    zoomMaximum_(5.),
    keyControlPressed_(false),
    currentMsaTool_(nullptr),
    previousTool_(nullptr),
    currentToolIsTemporary_(false),
    temporaryToolKey_(0),
    temporaryToolKeyPressed_(false),
    renderXShift_(0),
    showMouseCursorPoint_(false),
    msaStartSideWidget_(nullptr),
    msaStopSideWidget_(nullptr),
    msaRulerWidget_(nullptr),
    verticalMsaMarginWidget_(nullptr)
{
    // Map to monospace font, even on windows, which will not be able find font with the family of monospace. Thus, look
    // up the font with Qt's font matching algorithm (http://stackoverflow.com/questions/1468022/how-to-specify-monospace-fonts-for-cross-platform-qt-applications)
#ifdef Q_OS_WIN
#if QT_VERSION >= 0x040700
    font_.setStyleHint(QFont::Monospace);
#else
    font_.setStyleHint(QFont::TypeWriter);
#endif
#else
    font_.setFamily("DejaVuSansMono");
#endif

    font_.setPointSize(QApplication::font().pointSize());

    // Enable mouse tracking so that we can receive mouse move events even when a mouse button is not pressed
    viewport()->setMouseTracking(true);

    // Configure the context menu policy to emit a signal when the user requests the context menu
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Configure the side widgets - will update the viewport margins when the msa is assigned
    msaStartSideWidget_ = new MsaStartStopSideWidget(this, MsaStartStopSideWidget::StartType, this);
    msaStartSideWidget_->setHorizontalPadding(6);
    msaStartSideWidget_->setObjectName("msaStartSideWidget");
    msaStartSideWidget_->setAlignment(Qt::AlignRight);
    msaStopSideWidget_ = new MsaStartStopSideWidget(this, MsaStartStopSideWidget::StopType, this);
    msaStopSideWidget_->setHorizontalPadding(6);
    msaStopSideWidget_->setObjectName("msaStopSideWidget");
    msaRulerWidget_ = new MsaRulerWidget(this, this);
    msaRulerWidget_->setBottomVerticalPadding(4);
}

/**
  * Imperative to free up any allocated memory. The positionalMsaColorProvider_ pointer may either point to the default
  * stack member, defaultColorProvider_, or a derivative supplied by the user (@see setPositionalMsaColorProvider).
  * If the latter, it is to be deleted. Otherwise, nothing is done.
  */
AbstractMsaView::~AbstractMsaView()
{
    delete verticalMsaMarginWidget_;
    // The necessary logic is already implemented in the following method, thus, free its memory by setting the
    // provider to null.
    setPositionalMsaColorProvider(nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * If any msaRect axis is larger than the corresponding viewable area axis, then ensure that the top/left point is
  * visible.
  *
  * @param msaRect [const PosiRect &]
  */
void AbstractMsaView::centerOn(const PosiRect &msaRect)
{
    if (msa_ == nullptr)
        return;

    ASSERT_X(msa_->isValidRect(msaRect), "msaRect out of range");

    QRectF canvasRect = pointRectMapper_.msaRectToCanvasRect(msaRect);

    // Find horizontal center
    if (canvasRect.width() < viewport()->width())
    {
        int hPos = canvasRect.left() - (viewport()->width() - canvasRect.width()) / 2.;
        horizontalScrollBar()->setValue(hPos);
    }
    else
    {
        // We subtract a bit from the left so that we show a bit of neighboring context
        horizontalScrollBar()->setValue(canvasRect.left() - 10);
    }

    // Find vertical center
    if (canvasRect.height() < viewport()->height())
    {
        int vPos = canvasRect.top() - (viewport()->height() - canvasRect.height()) / 2;
        verticalScrollBar()->setValue(vPos);
    }
    else
    {
        // We subtract a bit from the left so that we show a bit of neighboring context
        verticalScrollBar()->setValue(canvasRect.top() - 10);
    }
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::baseline() const
{
    ASSERT_X(renderEngine()->abstractTextRenderer(), "No abstract text renderer defined");

    return renderEngine()->abstractTextRenderer()->baseline();
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::charHeight() const
{
    ASSERT_X(renderEngine()->abstractTextRenderer(), "No abstract text renderer defined");

    return renderEngine()->abstractTextRenderer()->height();
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::charWidth() const
{
    ASSERT_X(renderEngine()->abstractTextRenderer(), "No abstract text renderer defined");

    return renderEngine()->abstractTextRenderer()->width();
}

/**
  * The returned rectangle is from:
  *    (horizontal scroll bar value,
  *     vertical scroll bar value) ->
  *                                (MIN(viewport.width, ObservableMsa length * character width),
  *                                 MIN(viewport.height, ObservableMsa sequences * character height))
  *
  * @returns QRectF
  */
QRectF AbstractMsaView::clipRect() const
{
    if (!msa_)
        return QRectF();

    return pointRectMapper_.viewRectToCanvasRect(viewport()->rect());
}

/**
  * @returns IMsaTool *
  */
IMsaTool *AbstractMsaView::currentMsaTool() const
{
    return currentMsaTool_;
}

/**
  * The actual font is encapuslated within the FontCharPixmapProvider member.
  *
  * @returns QFont
  */
QFont AbstractMsaView::font() const
{
    return font_;
}

/**
  * @returns bool
  */
bool AbstractMsaView::isMouseOverSelection() const
{
    if (selection().isValid())
    {
        QPoint curMousePos = viewport()->mapFromGlobal(QCursor::pos());
        return pointRectMapper_.msaRectToViewRect(PosiRect(msaSelectionStart_, msaSelectionStop_).normalized()).contains(curMousePos);
    }

    return false;
}

/**
  * @returns bool
  */
bool AbstractMsaView::isMouseCursorPointVisible() const
{
    return showMouseCursorPoint_;
}

/**
  * @param msaRect [const MsaRect &]
  * @returns bool
  */
bool AbstractMsaView::isPartlyVisible(const PosiRect &msaRect) const
{
    ASSERT(msa_ != nullptr);
    ASSERT_X(msa_->isValidRect(msaRect), "msaRect out of range");

    return pointRectMapper_.msaRectToViewRect(msaRect).intersects(viewport()->rect());
}

/**
  * @returns QPoint
  */
QPoint AbstractMsaView::mouseCursorPoint() const
{
    // It is vital to use the viewport()->mapFromGlobal because our calculations will be off if there are any left/top
    // margins defined. In other words, when there are no left or right margins defined, viewport()->mapFromGlobal is
    // equivalent to this->mapFromGlobal(), yet the former is more correct.
    return pointRectMapper_.viewPointToMsaPoint(mouseHotSpot());
}

/**
  * View space.
  *
  * @returns QPoint
  */
QPoint AbstractMsaView::mouseHotSpot() const
{
    return viewport()->mapFromGlobal(QCursor::pos()) + mouseHotSpotOffset_;
}

/**
  * @returns QPoint
  */
QPoint AbstractMsaView::mouseHotSpotOffset() const
{
    return mouseHotSpotOffset_;
}

/**
  * @returns ObservableMsa *
  */
ObservableMsa *AbstractMsaView::msa() const
{
    return msa_;
}

/**
  * @returns const PointRectMapper *
  */
const PointRectMapper *AbstractMsaView::pointRectMapper() const
{
    return &pointRectMapper_;
}

/**
  * @returns const PositionalColorProvider *
  */
const PositionalMsaColorProvider *AbstractMsaView::positionalMsaColorProvider() const
{
    return positionalMsaColorProvider_;
}

/**
  * @param origin [const QPointF &]
  * @param msaRect [const PosiRect &]
  * @param renderEngine [IRenderEngine *renderEngine]
  * @param painter [const QPainter &]
  *
  * UNTESTED!
  */
void AbstractMsaView::renderMsaRegion(const QPointF &origin, const PosiRect &msaRect, IRenderEngine *renderEngine, QPainter *painter)
{
    ASSERT(positionalMsaColorProvider_ != nullptr);
    ASSERT(msa_ != nullptr);
    ASSERT(renderEngine != nullptr);
//    ASSERT(painter != nullptr);

    int top = msaRect.top();
    int left = msaRect.left();
    int bottom = msaRect.bottom();
    int right = msaRect.right();

    AbstractTextRenderer *textRenderer = renderEngine->abstractTextRenderer();
    qreal charWidth = textRenderer->width();
    qreal charHeight = textRenderer->height();

    QPointF pointF = origin;

    for (int i=top; i<=bottom; ++i)
    {
        QVector<TextColorStyle> colors = positionalMsaColorProvider_->colors(*msa_, i, msaRect.horizontalRange());

        pointF.rx() = origin.x();
        const char *c = msa_->at(i)->constData() + left - 1;
        for (int j=0, x=left; x<=right; ++x, ++j, ++c)
        {
            // The exact rendering point is:
            // origin.x() + j * charWidth,
            // origin.y() + i * charHeight
            renderEngine->drawBlockChar(pointF, *c, colors.at(j), painter);
            pointF.rx() += charWidth;
        }
        pointF.ry() += charHeight;
    }
}

/**
  * @returns MsaRulerWidget *
  */
MsaRulerWidget *AbstractMsaView::ruler() const
{
    return msaRulerWidget_;
}

/**
  * @param orientation [Qt::Orientation]
  * @param numSteps [int]
  */
void AbstractMsaView::scroll(Qt::Orientation orientation, int numSteps) const
{
    if (orientation == Qt::Vertical)
        verticalScrollBar()->setValue(verticalScrollBar()->value() + numSteps * verticalScrollBar()->singleStep());
    else
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + numSteps * horizontalScrollBar()->singleStep());
}

/**
  * @returns QPoint
  */
QPoint AbstractMsaView::scrollPosition() const
{
    return QPoint(horizontalScrollBar()->value(),
                  verticalScrollBar()->value());
}

/**
  * @returns PosiRect
  */
PosiRect AbstractMsaView::selection() const
{
    return PosiRect(msaSelectionStart_, msaSelectionStop_);
}

/**
  * @param msaTool [IMsaTool *]
  */
void AbstractMsaView::setCurrentMsaTool(IMsaTool *msaTool, bool isTemporary, int keyboardKey)
{
    if (currentMsaTool_ == msaTool)
        return;

    if (currentMsaTool_ != nullptr)
        currentMsaTool_->deactivate();

    if (isTemporary)
        previousTool_ = currentMsaTool_;
    else
        previousTool_ = nullptr;

    currentMsaTool_ = msaTool;

    if (currentMsaTool_ != nullptr)
        currentMsaTool_->activate();

    currentToolIsTemporary_ = isTemporary;
    temporaryToolKey_ = keyboardKey;
    temporaryToolKeyPressed_ = true;

    emit msaToolChanged();
}

/**
  * @param hotSpotOffset [const QPoint &]
  * @see mouseHotSpot(), mouseHotSpotOffset()
  */
void AbstractMsaView::setMouseHotSpotOffset(const QPoint &hotSpotOffset)
{
    mouseHotSpotOffset_ = hotSpotOffset;
    updateMouseCursorPoint();
}

/**
  * There will always be a valid PositionalMsaColorProvider instance utilized by this class. If
  * positionalMsaColorProvider is zero, then any previously assigned user instance will be freed and the
  * positionalMsaColorProvider() will return a pointer to the default color provider which may never be deleted.
  *
  * Takes ownership of positionalMsaColorProvider.
  *
  * @param positionalMsaColorProvider [PositionalMsaColorProvider *]
  */
void AbstractMsaView::setPositionalMsaColorProvider(PositionalMsaColorProvider *positionalMsaColorProvider)
{
    // Do nothing if same positional msa color provider is supplied
    if (positionalMsaColorProvider_ == positionalMsaColorProvider)
        return;

    // Check if we have already have a custom positional provider
    if (positionalMsaColorProvider_ != &defaultColorProvider_)
    {
        // Yes, free any existing provider
        delete positionalMsaColorProvider_;
        positionalMsaColorProvider_ = 0;
    }

    // Must always have a positional msa color provider, set to the default if the user has passed in a zero
    // pointer
    if (positionalMsaColorProvider != nullptr)
        positionalMsaColorProvider_ = positionalMsaColorProvider;
    else
        positionalMsaColorProvider_ = &defaultColorProvider_;

    emit colorProviderChanged();

    viewport()->update();
}

/**
  * @param msaPoint [const QPoint &]
  */
void AbstractMsaView::setSelectionStart(const QPoint &msaPoint)
{
    setSelection(PosiRect(msaPoint, msaSelectionStop_));
}

/**
  * @param msaPoint [const QPoint &]
  */
void AbstractMsaView::setSelectionStop(const QPoint &msaPoint)
{
    setSelection(PosiRect(msaSelectionStart_, msaPoint));
}

/**
  * @param renderXShift [qreal]
  */
void AbstractMsaView::setRenderXShift(qreal xShift)
{
    if (qFuzzyCompare(renderXShift_, xShift))
        return;

    renderXShift_ = xShift;
    emit renderXShiftChanged(renderXShift_);
}

/**
  * @param undoStack [QUndoStack *undoStack]
  */
void AbstractMsaView::setUndoStack(QUndoStack *undoStack)
{
    undoStack_ = undoStack;
}

void AbstractMsaView::setVerticalMsaMarginWidget(VerticalMsaMarginWidget *verticalMsaMarginWidget)
{
    delete verticalMsaMarginWidget_;
    verticalMsaMarginWidget_ = verticalMsaMarginWidget;
    if (verticalMsaMarginWidget_ != nullptr)
        verticalMsaMarginWidget_->setParent(this);
    updateMarginWidgetGeometries();
    updateScrollBarRangesAndSteps();
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::renderXShift() const
{
    return renderXShift_;
}

/**
  * @returns QUndoStack *
  */
QUndoStack *AbstractMsaView::undoStack() const
{
    return undoStack_;
}

/**
  */
void AbstractMsaView::updateMarginWidgetGeometries()
{
    setUpdatesEnabled(false);
    QMargins margins;

    // The following calculate the minimal amount of space required to display the start/stop coordinates of all
    // sequences in the msa. This must be done first before we can accurately set the viewport margins.
    if (msaStartSideWidget_->isVisible())
    {
        msaStartSideWidget_->updateWidth();
        margins.setLeft(msaStartSideWidget_->width());
    }
    if (msaStopSideWidget_->isVisible())
    {
        msaStopSideWidget_->updateWidth();
        margins.setRight(msaStopSideWidget_->width());
    }
    if (msaRulerWidget_->isVisible())
    {
        msaRulerWidget_->updateHeight();
        margins.setTop(msaRulerWidget_->height());
    }
    if (verticalMsaMarginWidget_ != nullptr && verticalMsaMarginWidget_->isVisible())
    {
        verticalMsaMarginWidget_->updateHeight();
        int canvasHeight = canvasSize().height();
        int x = height() - canvasHeight - margins.top();
        if (horizontalScrollBar()->isVisible())
            x -= horizontalScrollBar()->height();
        margins.setBottom(qMax(x, verticalMsaMarginWidget_->height()));
    }

    // Position the start coordinate side widget. Note - the y value is still 0 because it is derived from an
    // AbstractMsaSideWidget which automatically accounts for the msa ruler height. Also, the height should be the full
    // height of the AbstractScrollArea - not the viewport.
    if (msaStartSideWidget_->isVisible())
        msaStartSideWidget_->setGeometry(0, 0, margins.left(), height() - margins.bottom());

    // Now for the ruler widget
    if (msaRulerWidget_->isVisible())
        msaRulerWidget_->setGeometry(margins.left(), 0, viewport()->width(), margins.top());

    // Finally, the stop coordinate widget. It can be in one of two positions dependending on the viewport width
    // 1) If viewport can fully contain the alignment, then it is on the right hand side of the alignment; note that
    //    this calculation takes into account the x render shift
    // 2) Else, it is just to the left of the vertical scrollbar
    if (msaStopSideWidget_->isVisible())
    {
        int stopWidgetLeft = margins.left() + qMin(canvasSize().width() + static_cast<int>(renderXShift()),
                                                   viewport()->width());
        msaStopSideWidget_->setGeometry(stopWidgetLeft, 0, margins.right(), height() - margins.bottom());
    }
    if (verticalMsaMarginWidget_ != nullptr && verticalMsaMarginWidget_->isVisible())
    {
        // TODO: Determine why this order is critical!
        // Critical order here!
        // For some reason it is vital to call updateScroll;BarRanges *before* setting the geometry. What happens
        // if this is the other way around is that another resize event is triggered for the msaView with a larger
        // viewport size, and that incorrectly places the consensus view to far down on the screen.
        verticalMsaMarginWidget_->updateScrollBarRanges();
        verticalMsaMarginWidget_->setGeometry(margins.left(), margins.top() + viewport()->height(), viewport()->width(), verticalMsaMarginWidget_->height());
    }

    // Only after updating the above geometries, now set the scroll bar geometries
    QRect hRect = horizontalScrollBar()->geometry();
    hRect.setLeft(margins.left());
    hRect.setWidth(viewport()->width());
    horizontalScrollBar()->setGeometry(hRect);

    QRect vRect = verticalScrollBar()->geometry();
    vRect.setTop(margins.top());
    vRect.setHeight(viewport()->height());
    verticalScrollBar()->setGeometry(vRect);

    // Margins:
    // Left: start coordinate side widget
    // Top: ruler
    // Bottom: <configurable vertical widget>
    // Right: stop coordinate side widget
    //
    // Top and bottom (optional) are sandwiched between the left and right similar to a cabinet rail is between two stiles
    if (viewportMargins_ != margins)
    {
        setViewportMargins(margins);
        viewportMargins_ = margins;
//        updateMarginWidgetGeometries();
        emit viewportMarginsChanged(viewportMargins_);
    }

    setUpdatesEnabled(true);
}

/**
  * @returns QMargins
  */
QMargins AbstractMsaView::viewportMargins() const
{
    return viewportMargins_;
}

/**
  * The actual zoom is encapuslated within the FontCharPixmapProvider member.
  *
  * @returns qreal
  */
qreal AbstractMsaView::zoom() const
{
    return zoom_;
}

/**
  * @returns double
  */
double AbstractMsaView::zoomFactor() const
{
    return zoomFactor_;
}

/**
  * @returns double
  */
double AbstractMsaView::zoomMinimum() const
{
    return zoomMinimum_;
}

/**
  * @returns double
  */
double AbstractMsaView::zoomMaximum() const
{
    return zoomMaximum_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void AbstractMsaView::clearSelection()
{
    setSelection(PosiRect());
}

/**
  */
void AbstractMsaView::hideMouseCursorPoint()
{
    if (showMouseCursorPoint_)
    {
        showMouseCursorPoint_ = false;
        viewport()->update();
        emit mouseCursorVisibleChanged(false);
    }
}

/**
  * @param msaRow [int]
  */
void AbstractMsaView::repaintRow(int /* msaRow */)
{
    update();
}

/**
  */
void AbstractMsaView::selectAll()
{
    setSelection(PosiRect(QPoint(1, 1), QPoint(msa_->length(), msa_->rowCount())));
}

/**
  * Should preserve the current center point as best as possible. Should consolidate code with setZoomBy for this!
  *
  * @param font [const QFont &]
  * @see font()
  */
void AbstractMsaView::setFont(const QFont &font)
{
    if (font_ == font)
        return;

    font_ = font;
    if (msa_)
        saveViewFocus();

    AbstractTextRenderer *textRenderer = renderEngine()->abstractTextRenderer();
    textRenderer->setFont(font);
    emit fontChanged();

    if (msa_)
    {
        // Ensure that the charWidth and charHeight for the given zoom level is still above 1
        if (textRenderer->width() <= textRenderer->height())
        {
            if (textRenderer->width() < 1.)
                setZoom(1. / textRenderer->width());
        }
        else    // width > height
        {
            if (textRenderer->height() < 1.)
                setZoom(1. / textRenderer->height());
        }
    }

    // Given that the font has changed, update the scroll bar parameters, but not the position
    updateMarginWidgetGeometries();
    updateScrollBarRangesAndSteps();

    // Update the scroll bar positions accordingly
    if (msa_)
        restoreViewFocus();

    // Sometimes changing the font does not trigger a change in the current scroll position and in turn does not
    // update the screen. Thus, we call update to ensure that it gets repainted with every font change
    viewport()->update();
}

/**
  * Configure the view to observe and visualize msa. Watches the msa for changes to update the view appropriately. Does
  * not take ownership of the ObservableMsa.
  *
  * Additionally resets to the default selection rectangle, and sets the scroll bars to 0, 0.
  *
  * @param msa [ObservableMsa *]
  */
void AbstractMsaView::setMsa(ObservableMsa *msa)
{
    // Do nothing if the msa is the same
    if (msa == msa_)
        return;

    // If we already have a valid msa, disconnect any previously configured signals
    if (msa_)
    {
        disconnect(msa_, SIGNAL(collapsedLeft(PosiRect)),                       this, SLOT(onMsaCollapsedLeft(PosiRect)));
        disconnect(msa_, SIGNAL(collapsedRight(PosiRect)),                      this, SLOT(onMsaCollapsedRight(PosiRect)));
        disconnect(msa_, SIGNAL(gapColumnsInserted(ClosedIntRange)),            this, SLOT(onMsaGapColumnsInserted(ClosedIntRange)));
        disconnect(msa_, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)),    this, SLOT(onMsaGapColumnsRemoved(QVector<ClosedIntRange>)));
        disconnect(msa_, SIGNAL(msaReset()),                                    this, SLOT(onMsaReset()));
        disconnect(msa_, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)),    this, SLOT(onMsaRectangleSlid(PosiRect,int,ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsInserted(ClosedIntRange)),                  this, SLOT(onMsaRowsInserted(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)),                 this, SLOT(onMsaRowsMoved(ClosedIntRange,int)));
        disconnect(msa_, SIGNAL(rowsRemoved(ClosedIntRange)),                   this, SLOT(onMsaRowsRemoved(ClosedIntRange)));
        disconnect(msa_, SIGNAL(rowsSorted()),                                  this, SLOT(clearSelection()));
        disconnect(msa_, SIGNAL(rowsSorted()),                                  this, SLOT(onMsaRowsSorted()));
        disconnect(msa_, SIGNAL(rowsSwapped(int,int)),                          this, SLOT(onMsaRowsSwapped(int,int)));
        disconnect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)),         this, SLOT(onMsaSubseqsChanged(SubseqChangePodVector)));

        disconnect(msa_, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)),    viewport(), SLOT(update()));
        disconnect(msa_, SIGNAL(gapColumnsInserted(ClosedIntRange)),            viewport(), SLOT(update()));
        disconnect(msa_, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)),    viewport(), SLOT(update()));
        disconnect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)),         viewport(), SLOT(update()));
        disconnect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)),                 viewport(), SLOT(update()));
    }

    msa_ = msa;

    // If the new msa is not NULL, then hook up the appropriate signals
    if (msa_)
    {
        connect(msa_, SIGNAL(collapsedLeft(PosiRect)),                       SLOT(onMsaCollapsedLeft(PosiRect)));
        connect(msa_, SIGNAL(collapsedRight(PosiRect)),                      SLOT(onMsaCollapsedRight(PosiRect)));
        connect(msa_, SIGNAL(gapColumnsInserted(ClosedIntRange)),            SLOT(onMsaGapColumnsInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)),    SLOT(onMsaGapColumnsRemoved(QVector<ClosedIntRange>)));
        connect(msa_, SIGNAL(msaReset()),                                    SLOT(onMsaReset()));
        connect(msa_, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)),    SLOT(onMsaRectangleSlid(PosiRect,int,ClosedIntRange)));
        connect(msa_, SIGNAL(rowsInserted(ClosedIntRange)),                  SLOT(onMsaRowsInserted(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)),                 SLOT(onMsaRowsMoved(ClosedIntRange,int)));
        connect(msa_, SIGNAL(rowsRemoved(ClosedIntRange)),                   SLOT(onMsaRowsRemoved(ClosedIntRange)));
        connect(msa_, SIGNAL(rowsSorted()),                                  SLOT(clearSelection()));
        connect(msa_, SIGNAL(rowsSorted()),                                  SLOT(onMsaRowsSorted()));
        connect(msa_, SIGNAL(rowsSwapped(int,int)),                          SLOT(onMsaRowsSwapped(int,int)));
        connect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)),         SLOT(onMsaSubseqsChanged(SubseqChangePodVector)));

        connect(msa_, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)),    viewport(), SLOT(update()));
        connect(msa_, SIGNAL(gapColumnsInserted(ClosedIntRange)),            viewport(), SLOT(update()));
        connect(msa_, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)),    viewport(), SLOT(update()));
        connect(msa_, SIGNAL(subseqsChanged(SubseqChangePodVector)),         viewport(), SLOT(update()));
        connect(msa_, SIGNAL(rowsMoved(ClosedIntRange,int)),                 viewport(), SLOT(update()));

        updateMarginWidgetGeometries();
    }

    updateScrollBarRangesAndSteps();

    clearSelection();

    // Reset the msaRegion so that it will be recalculated
    msaRegionClip_ = PosiRect();

    // Reset the scroll position to zero
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Reset the zoom - ideally this should be call the setZoom method; however, the setZoom method has lots of other
    // machinery and in particular requires a valid msa to do anything.
    zoom_ = qBound(zoomMinimum_, 1., zoomMaximum_);

    emit msaChanged();
}

/**
  * @param visible [bool]
  */
void AbstractMsaView::setMsaRulerVisible(bool visible)
{
    msaRulerWidget_->setVisible(visible);
    updateMarginWidgetGeometries();
}

/**
  * @param msaRect [const PosiRect &]
  */
void AbstractMsaView::setSelection(const PosiRect &msaRect)
{
#ifdef QT_DEBUG
    if (msaRect.isNull() == false)
    {
        ASSERT(msa_ != nullptr);
        ASSERT_X(msa_->isValidColumn(msaRect.left()), "msaRect.left out of range");
        ASSERT_X(msa_->isValidColumn(msaRect.right()), "msaRect.right out of range");
        ASSERT_X(msa_->isValidRow(msaRect.top()), "msaRect.top out of range");
        ASSERT_X(msa_->isValidRow(msaRect.bottom()), "msaRect.bottom out of range");
    }
#endif

    PosiRect oldSelection = selection().normalized();
    PosiRect normalizedMsaRect = msaRect.normalized();
    if (oldSelection == normalizedMsaRect)
        return;

    msaSelectionStart_ = msaRect.topLeft();
    msaSelectionStop_ = msaRect.bottomRight();

    emit selectionChanged(selection(), oldSelection);
    viewport()->update();
}

/**
  * @param visible [bool]
  */
void AbstractMsaView::setStartSideWidgetVisible(bool visible)
{
    msaStartSideWidget_->setVisible(visible);
    updateMarginWidgetGeometries();
}

/**
  * @param visible [bool]
  */
void AbstractMsaView::setStopSideWidgetVisible(bool visible)
{
    msaStopSideWidget_->setVisible(visible);
    updateMarginWidgetGeometries();
}

void AbstractMsaView::setBottomMarginWidgetVisible(bool visible)
{
    if (verticalMsaMarginWidget_ != nullptr)
    {
        verticalMsaMarginWidget_->setVisible(visible);
        updateMarginWidgetGeometries();
    }
}

/**
  * Wrapper object to zoom relative to the center of the view.
  *
  * @param zoom [double]
  */
void AbstractMsaView::setZoom(double zoom)
{
    setZoom(zoom, QPoint(-1, -1));
}

/**
  * If zoom is different than the current zoom, updates the zoom value, emits zoomChanged() via the scaleChanged signal
  * of textRenderer_, and updates the viewport.
  *
  * @param zoom [double]
  */
void AbstractMsaView::setZoom(double zoom, const QPointF &focus)
{
    if (!msa_)
        return;

    qreal clampedZoom = qBound(zoomMinimum_, zoom, zoomMaximum_);

    // Since we only check to the fourth decimal place, it is important that connected classes do not require more than
    // this!
    if (fabs(zoom_ - clampedZoom) < .0001)
        return;

    // Important to call this before the zoomChanged signal so that we have the view focus data before the charWidth
    // and charHeight values are changed within the inherited classes
    saveViewFocus(focus);

    // Minimum of 1 character in x and y directions for the char width and height
    qreal oldZoom = zoom_;
    zoom_ = clampedZoom;
    AbstractTextRenderer *textRenderer = renderEngine()->abstractTextRenderer();
    textRenderer->setScale(zoom_);

    // Check that char width and height are valid
    if (textRenderer->width() >= 1. && textRenderer->height() >= 1.)
    {
        emit zoomChanged(zoom_);

        updateMarginWidgetGeometries();
        // Update the scroll bar positions accordingly
        updateScrollBarRangesAndSteps();

        // Restore the view focus and push and update request in the rare event that the new scroll bars position
        // did not change
        restoreViewFocus();
        viewport()->update();

        return;
    }

    // Else, while zoom is above the absoluteMinimumZoom, it would cause the charHeight and charWidth to be < 1, which
    // is not permissible
    zoom_ = oldZoom;
    textRenderer->setScale(zoom_);
}

/**
  * @param amount [int]
  * @param focus [const QPointF]
  */
void AbstractMsaView::setZoomBy(int amount, const QPointF focus)
{
    if (!msa_ || amount == 0)
        return;

    setZoom(zoom() * pow(1 + zoomFactor_, amount), focus);
}

/**
  * The zoom factor is the amount to increase/decrease the zoom relative to the current zoom. For example, a .1 zoom
  * factor indicates that the currently value should
  *
  * @param zoomFactor [double]
  */
void AbstractMsaView::setZoomFactor(double zoomFactor)
{
    if (zoomFactor <= 0.)
        return;

    zoomFactor_ = zoomFactor;
}

/**
  * @param zoomMinimum [double]
  */
void AbstractMsaView::setZoomMinimum(double zoomMinimum)
{
    if (zoomMinimum <= 0.)
        return;

    zoomMinimum_ = zoomMinimum;
    if (zoomMinimum_ > zoomMaximum_)
        zoomMaximum_ = zoomMinimum_;
}

/**
  * @param zoomMaximum [double]
  */
void AbstractMsaView::setZoomMaximum(double zoomMaximum)
{
    if (zoomMaximum <= 0.)
        return;

    zoomMaximum_ = zoomMaximum;
    if (zoomMaximum_ < zoomMinimum_)
        zoomMinimum_ = zoomMaximum_;
}

/**
  */
void AbstractMsaView::showMouseCursorPoint()
{
    if (!showMouseCursorPoint_)
    {
        showMouseCursorPoint_ = true;
        viewport()->update();
        emit mouseCursorVisibleChanged(true);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Master method for performing all draw operations as needed and in the proper order.
  *
  * @param painter [QPainter *]
  */
void AbstractMsaView::drawAll(QPainter *painter)
{
    // Do not draw anything if no msa has been defined
    if (!msa_ || msa_->subseqCount() == 0)
        return;

    QRect viewRect = viewport()->rect();

    // Update the msaRegionClip_
    setMsaRegionClip(pointRectMapper_.viewRectToMsaRect(viewRect));
    ASSERT(msaRegionClip_.isValid());

    // Limit all drawing to the area occupied by the MSA. If scrollbars are visible, this is naturally done already.
    if (!horizontalScrollBar()->isVisible())
        viewRect.setWidth(charWidth() * msa_->length());
    if (!verticalScrollBar()->isVisible())
        viewRect.setHeight(charHeight() * msa_->rowCount());
    if (!horizontalScrollBar()->isVisible() || !verticalScrollBar()->isVisible())
        painter->setClipRect(viewRect);

    drawBackground(painter);
    drawMsa(clipRenderOrigin(), msaRegionClip_.normalized(), painter);
    PosiRect selRect(visibleSelectionRect());
    if (selRect.isValid())
    {
        QRectF viewRect = pointRectMapper_.msaRectToViewRect(selRect);

        viewRect.setLeft(viewRect.left() + renderXShift_);
        viewRect.setRight(viewRect.right() + renderXShift_);
        drawSelection(viewRect, painter);
    }

    if (showMouseCursorPoint_)
    {
        // Note: important to call the mouseCursorPoint() method because it takes into account any defined mouse hot
        // spot offset.
        QRectF viewRect = pointRectMapper_.msaPointToViewRect(mouseCursorPoint());
        viewRect.moveLeft(viewRect.left() + renderXShift_);
        drawMouseCursorPoint(viewRect, painter);
    }

    // Provide for tool specific painting :)
    if (currentMsaTool_ != nullptr)
        currentMsaTool_->viewportPaint(renderEngine(), painter);
}

/**
  * Draw the background using painter; empty stub because QWidget auto fills the background.
  *
  * @param painter [QPainter *]
  */
void AbstractMsaView::drawBackground(QPainter * /* painter */) const {}

/**
  * @param origin [const QPointF &]
  * @param msaRect [const PosiRect &]
  * @param painter [QPainter *]
  */
void AbstractMsaView::drawMsa(const QPointF &origin, const PosiRect &msaRect, QPainter *painter)
{
    ASSERT(painter != nullptr);
    renderMsaRegion(origin, msaRect, renderEngine(), painter);
}

/**
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void AbstractMsaView::drawSelection(const QRectF &rect, QPainter *painter) const
{
    // Draw the transparent rectangle
    renderEngine()->fillRect(rect.toRect(), QColor(0, 0, 0, 64), painter);

    // Outline the rectangle
    renderEngine()->setLineWidth(qMax(1, qMin(2, qRound(zoom()))));
    renderEngine()->outlineRectInside(rect.toRect(), Qt::black, painter);
}

/**
  * @param rect [const QRectF &]
  * @param painter [QPainter *]
  */
void AbstractMsaView::drawMouseCursorPoint(const QRectF &rect, QPainter *painter) const
{
    // Draw an outlined rectangle
    renderEngine()->setLineWidth(qMax(1, qMin(2, qRound(zoom()))));
    renderEngine()->outlineRectInside(rect.toRect(), Qt::black, painter);
}

/**
  * @returns PosiRect
  */
PosiRect AbstractMsaView::msaRegionClip() const
{
    return msaRegionClip_;
}

/**
  * Scrolling simply entails updating the viewport, which will refresh the screen with the latest position information.
  *
  * @param dx [int]
  * @param dy [int]
  */
void AbstractMsaView::scrollContentsBy(int /* dx */, int /* dy */)
{
    viewport()->update();
}

/**
  * @param newMsaRegionlCip [const Rect &]
  * @see setRenderXShift()
  */
void AbstractMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    // ASSERT that newMsaRegionClip is within the ObservableMsa bounds
    ASSERT(msa_ != nullptr);
    ASSERT_X(newMsaRegionClip.left() > 0 && newMsaRegionClip.top() > 0, "newMsaRegionClip top, left out of range");
    ASSERT_X(newMsaRegionClip.right() <= msa_->length() && newMsaRegionClip.bottom() <= msa_->subseqCount(), "newMsaRegionClip bottom, right out of range");

    // newMsaRegionClip must not be expanded out from msaRegionClip_ on all sides
    ASSERT_X(!(newMsaRegionClip.left() < msaRegionClip_.left() &&
               newMsaRegionClip.right() > msaRegionClip_.right() &&
               newMsaRegionClip.top() < msaRegionClip_.top() &&
               newMsaRegionClip.bottom() < msaRegionClip_.bottom()), "Invalid (or rather unexpected) msa clip rectangle!");

    msaRegionClip_ = newMsaRegionClip;

    // Account for any view shifting :) We presume that any view shifting should preserve the requested newMsaRegionClip
    int msaXShift = -renderXShift_ / charWidth();
    if (msaXShift)
    {
        msaRegionClip_.setLeft(qMax(1, msaRegionClip_.left() + msaXShift));
        msaRegionClip_.setRight(qMin(msa_->columnCount(), newMsaRegionClip.right() + msaXShift));
    }
}

/**
  * Does nothing in this class except call the parent class method; however, because it is protected, derived classes
  * may override this method as needed. For instance, to enforce that a QGLWidget is used (see GlAbstractMsaView)
  *
  * @param widget [QWidget *]
  */
void AbstractMsaView::setViewport(QWidget *widget)
{
    QAbstractScrollArea::setViewport(widget);
}

/**
  * @param event [QEvent *]
  * @returns bool
  */
bool AbstractMsaView::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        if (currentMsaTool_->isActive())
        {
            event->accept();
            return true;
        }
    }

    return QAbstractScrollArea::event(event);
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void AbstractMsaView::keyPressEvent(QKeyEvent *keyEvent)
{
    if (currentToolIsTemporary_ && !keyEvent->isAutoRepeat())
        temporaryToolKeyPressed_ = true;

    switch(keyEvent->key())
    {
    case Qt::Key_Control:
        keyControlPressed_ = true;
        break;
    case Qt::Key_PageDown:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep());
        return;
    case Qt::Key_PageUp:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + -verticalScrollBar()->pageStep());
        return;
    case Qt::Key_Home:
        verticalScrollBar()->setValue(0);
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(0);
        return;
    case Qt::Key_End:
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
        return;
    case Qt::Key_Left:
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(0);
        else
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep());
        return;
    case Qt::Key_Right:
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
        else
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep());
        return;
    case Qt::Key_Up:
        if (keyControlPressed_)
            verticalScrollBar()->setValue(0);
        else
            verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep());
        return;
    case Qt::Key_Down:
        if (keyControlPressed_)
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        else
            verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep());
        return;

    default:
        break;
    }

    if (currentMsaTool_ != nullptr)
        currentMsaTool_->viewportKeyPressEvent(keyEvent);

    keyEvent->ignore();
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void AbstractMsaView::keyReleaseEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Control:
        if (!keyEvent->isAutoRepeat())
            keyControlPressed_ = false;
        break;

    default:
        break;
    }

    if (currentMsaTool_ != nullptr)
    {
        currentMsaTool_->viewportKeyReleaseEvent(keyEvent);

        // Check if the currently active tool is temporary and reset it if is
        if (currentToolIsTemporary_ && !keyEvent->isAutoRepeat())
        {
            temporaryToolKeyPressed_ = false;

            // The !isActive_ check avoids the unpleasant scenario where the user has pressed the relevant key to
            // initiate a temporary mode, then somehow exited that mode while an activity is underway. This would leave the
            // operation in an unresolved state.
            if (!currentMsaTool_->isActive() && keyEvent->key() == temporaryToolKey_)
                setCurrentMsaTool(previousTool_);
        }
    }

    keyEvent->ignore();
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (currentMsaTool_ != nullptr)
        currentMsaTool_->viewportMousePressEvent(mouseEvent);
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (currentMsaTool_ != nullptr)
        currentMsaTool_->viewportMouseMoveEvent(mouseEvent);

    updateMouseCursorPoint();

    emit viewportMouseMoved(mouseEvent->pos());
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (currentMsaTool_ != nullptr)
    {
        currentMsaTool_->viewportMouseReleaseEvent(mouseEvent);

        // Check if the currently active tool is temporary and reset it if is
        if (currentToolIsTemporary_ && !temporaryToolKeyPressed_)
        {
            ASSERT(currentMsaTool_->isActive() == false);

            // The !isActive_ check avoids the unpleasant scenario where the user has pressed the relevant key to
            // initiate a temporary mode, then somehow exited that mode while an activity is underway. This would leave the
            // operation in an unresolved state.
            setCurrentMsaTool(previousTool_);
        }
    }
}

/**
  * Delegate all painting to the virtual drawAll event, which fires off the necessary virtualized draw methods as needed
  * and in the proper order.
  *
  * @param paintEvent [QPaintEvent *]
  */
void AbstractMsaView::paintEvent(QPaintEvent * /* paintEvent */)
{
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    drawAll(&painter);
}

/**
  * The viewport widget has been resized. Depending on the viewport and canvas sizes, must update the scroll bar values,
  * clip size, and clip rect position.
  *
  * @param resizeEvent [QResizeEvent *]
  */
void AbstractMsaView::resizeEvent(QResizeEvent * /* resizeEvent */)
{
    updateMarginWidgetGeometries();
    updateScrollBarRangesAndSteps();
}

/**
  * @param event [QEvent *]
  * @returns bool
  */
bool AbstractMsaView::viewportEvent(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Enter:
        if (currentMsaTool_ != nullptr)
            currentMsaTool_->viewportMouseEnterEvent();
        return false;
    case QEvent::Leave:
        if (currentMsaTool_ != nullptr)
            currentMsaTool_->viewportMouseLeaveEvent();
        emit viewportMouseLeft();
        return false;
    case QEvent::WindowDeactivate:
        if (currentMsaTool_ != nullptr)
            currentMsaTool_->viewportWindowDeactivate();

        // When the window is deactivated however that may be, disable any potentially active
        // actions
        keyControlPressed_ = false;

    default:
        return QAbstractScrollArea::viewportEvent(event);
    }
}

/**
  * @param wheelEvent [QWheelEvent *]
  */
void AbstractMsaView::wheelEvent(QWheelEvent *wheelEvent)
{
    int numSteps = ::stepsInWheelEvent(wheelEvent);

//    qDebug() << Q_FUNC_INFO;
//    qDebug() << "Wheel Delta" << wheelEvent->delta();
//    qDebug() << "Wheel degrees" << numDegrees;
//    qDebug() << "Wheel steps" << numSteps;
//    qDebug() << keyControlPressed_;

    // Important to use the modifiers method here for when the user uses the mouse wheel and control but the viewport
    // does not have the focus
    if ((wheelEvent->modifiers() & Qt::ControlModifier) && wheelEvent->orientation() == Qt::Vertical)
        setZoomBy(-numSteps, wheelEvent->pos());
    else
        scroll(wheelEvent->orientation(), numSteps);

    wheelEvent->accept();

    if (currentMsaTool_ != nullptr)
        currentMsaTool_->viewportWheelEvent(wheelEvent);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param msaRect [const PosiRect &]
  */
void AbstractMsaView::onMsaCollapsedLeft(const PosiRect & /* msaRect */) {}

/**
  * @param msaRect [const PosiRect &]
  */
void AbstractMsaView::onMsaCollapsedRight(const PosiRect & /* msaRect */) {}

/**
  * @param columns [const ClosedIntRange &]
  */
void AbstractMsaView::onMsaGapColumnsInserted(const ClosedIntRange &columns)
{
    updateMarginWidgetGeometries();

    // Given the gap columns, the scroll bars should update accordingly
    updateScrollBarRangesAndSteps();

    PosiRect curSel = selection().normalized();

    // Update the selection if necessary
    if (columns.begin_ > curSel.right())
        return;

    PosiRect newSel = curSel;

    if (columns.begin_ > curSel.left())
        newSel.setRight(curSel.right() + columns.length());
    else // begin <= msaSelectionStart.x
    {
        newSel.setLeft(curSel.left() + columns.length());
        newSel.setRight(curSel.right() + columns.length());
    }

    setSelection(newSel);
}

/**
  * 5 cases to deal with (x => selection, | = gap column)
  * 1) xxxx |||
  *    Action: nothing
  *
  * 2) xxxx
  *      ||||
  *    Action: decrease selection stop to gap range beginning - 1
  *
  * 3) xxxxxxxxxxx
  *        ||||
  *    Action: decrease selection stop by gap range size
  *
  * 4)   xxxx
  *    ||||
  *    Action: set selection start to gap range beginning and decrease selection stop by gap range size
  *
  * 5)   xxxx
  *    ||||||||
  *    Action: entire selection region has been removed, set the default selection
  *
  * 6)     xxxx
  *    ||||
  *    Action: decrease selection start and stop by removed range size
  *
  * @param columnRanges [const QVector<ClosedIntRange> &]]
  */
void AbstractMsaView::onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &columnRanges)
{
    updateMarginWidgetGeometries();

    bool clearSel = false;
    PosiRect oldNormalizedSelection = selection().normalized();
    PosiRect newSel = oldNormalizedSelection;
    for (int i=columnRanges.size() - 1; i>= 0; --i)
    {
        // Case 1: gap range is beyond selection
        if (columnRanges.at(i).begin_ > newSel.right())
            continue;

        // Cases 2 and 3: Gap range overlaps right portion of selection
        if (columnRanges.at(i).begin_ > newSel.left())
        {
            newSel.setRight(newSel.right() - qMin(newSel.right() - columnRanges.at(i).begin_ + 1,
                                                  columnRanges.at(i).length()));
        }
        // Cases 4 and 5: gap range end overlaps left portion (or all) of selection
        else if (columnRanges.at(i).end_ >= newSel.left())
        {
            newSel.setLeft(columnRanges.at(i).begin_);
            newSel.setRight(qMax(0, newSel.right() - columnRanges.at(i).length()));
        }
        // Case 6: gap range is completely to the left of the selection start
        else
        {
            newSel.setLeft(newSel.left() - columnRanges.at(i).length());
            newSel.setRight(newSel.right() - columnRanges.at(i).length());
        }

        if (newSel.right() < newSel.left())
        {
            clearSel = true;
            // Since the selection is to be cleared, no need to continue modifying the selection relative to gap removal
            break;
        }
    }

    if (clearSel)
        clearSelection();
    else if (newSel != oldNormalizedSelection)
        setSelection(newSel);

    updateScrollBarRangesAndSteps();
    viewport()->update();
}

/**
  */
void AbstractMsaView::onMsaReset() {}

/**
  * @param msaRect [const PosiRect &]
  * @param delta [int]
  * @param finalRange [const ClosedIntRange &]
  */
void AbstractMsaView::onMsaRectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange)
{
    ASSERT_X(delta != 0, "Delta must be non zero");

    Q_UNUSED(msaRect);
    Q_UNUSED(delta);
    Q_UNUSED(finalRange);
}

/**
  * @param rows [const ClosedIntRange &]
  */
void AbstractMsaView::onMsaRowsInserted(const ClosedIntRange &rows)
{
    updateMarginWidgetGeometries();
    updateScrollBarRangesAndSteps();

    PosiRect curSel = selection().normalized();

    // Update the selection if necessary
    if (rows.begin_ > curSel.bottom())
        return;

    PosiRect newSel = curSel;

    if (rows.begin_ > curSel.top())
        newSel.setBottom(curSel.bottom() + rows.length());
    else // begin <= msaSelectionStart.x
    {
        newSel.setTop(curSel.top() + rows.length());
        newSel.setBottom(curSel.bottom() + rows.length());
    }

    setSelection(newSel);
}

/**
  * @param rows [const ClosedIntRange &]
  * @param finalRow [int]
  */
void AbstractMsaView::onMsaRowsMoved(const ClosedIntRange & /* rows */, int /* finalRow */)
{
    msaStartSideWidget_->update();
    msaStopSideWidget_->update();
}

/**
  * Similar logic as with the gap column removal, but applied to the vertical selection range.
  *
  * @param rows [const ClosedIntRange &]
  * @see onMsaGapGapColumnsRemoved
  */
void AbstractMsaView::onMsaRowsRemoved(const ClosedIntRange &rows)
{
    {
        PosiRect oldNormalizedSelection = selection().normalized();
        PosiRect newSel = oldNormalizedSelection;

        if (rows.begin_ > newSel.bottom())
            ;   // <-- This is intentional! Used to be a return and once refactored may be a return again
        // Cases 2 and 3: row range overlaps bottom portion of selection
        else if (rows.begin_ > newSel.top())
        {
            newSel.setBottom(newSel.bottom() - qMin(newSel.bottom() - rows.begin_ + 1,
                                                    rows.length()));
        }
        // Cases 4 and 5: row range end overlaps top portion (or all) of selection
        else if (rows.end_ >= newSel.top())
        {
            newSel.setTop(rows.begin_);
            newSel.setBottom(qMax(0, newSel.bottom() - rows.length()));
        }
        // Case 6: row range is completely to the top of the selection start
        else
        {
            newSel.setTop(newSel.top() - rows.length());
            newSel.setBottom(newSel.bottom() - rows.length());
        }

        if (newSel.bottom() < newSel.top())
            clearSelection();
        else if (newSel != oldNormalizedSelection)
            setSelection(newSel);
    }

    // OPTIMIZATION: check that the removed rows would affect the visible alignment and only then update
    updateMarginWidgetGeometries();
    updateScrollBarRangesAndSteps();
}

/**
  */
void AbstractMsaView::onMsaRowsSorted()
{
    msaStartSideWidget_->update();
    msaStopSideWidget_->update();
}

/**
  * @param first [int]
  * @param second [int]
  */
void AbstractMsaView::onMsaRowsSwapped(int /* first */, int /* second */)
{
    qWarning("Please implement!! %s\n", Q_FUNC_INFO);
}

/**
  * @param subseqChangePods [const SubseqChangePodVector &]
  */
void AbstractMsaView::onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods)
{
    foreach (const SubseqChangePod &changePod, subseqChangePods)
    {
        if (changePod.operation_ != SubseqChangePod::eInternal)
        {
            updateMarginWidgetGeometries();
            return;
        }
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Because the font may be arbitrarily zoomed, the actual canvas size is best represented by real numbers; however
  * when requested in integer units, the returned size is the rounded up version of the corresponding floating point
  * numbers.
  *
  * @returns QSize
  */
QSize AbstractMsaView::canvasSize() const
{
    if (!msa_)
        return QSize(0, 0);

    QSizeF sizeF = canvasSizeF();
    return QSize(ceil(sizeF.width()), ceil(sizeF.height()));
}

/**
  * @returns QSizeF
  */
QSizeF AbstractMsaView::canvasSizeF() const
{
    if (!msa_)
        return QSizeF(0., 0.);

    return QSizeF(msa_->length() * charWidth(), msa_->subseqCount() * charHeight());
}

/**
  * AbstractMsaView provides a visual window into a part or all of the ObservableMsa. When rendering the ObservableMsa, it is only necessary
  * to render the portion of the ObservableMsa that is currently visible. For simplicity and convenience purposes, if any part of
  * a glyph is visible, then we render the entire rectangular block for that glyph. This occurs whenever a glyph rests
  * on the edge of the viewable window. Because scrolling is done at the pixel level and each character typically will
  * cover a rectangular array of pixels, the rendering origin varies from 0 .. negative character width. This method
  * returns this offset value based on the current scroll bar positions and the currently displayed msaRegion.
  *
  * Only valid if the msaRegion has been properly updated (which at least occurs every paintEvent).
  *
  * @returns QPointF
  */
QPointF AbstractMsaView::clipRenderOrigin() const
{
    if (!msa_)
        return QPointF();

    return QPointF(-(horizontalScrollBar()->value() - charWidth() * (msaRegionClip_.left() - 1)) + renderXShift_,
                   -(verticalScrollBar()->value() - charHeight() * (msaRegionClip_.top() - 1)));
}

/**
  */
void AbstractMsaView::restoreViewFocus() const
{
    QRectF clipRect = this->clipRect();
    horizontalScrollBar()->setValue((viewFocusData_.msaPoint_.x()-1.) * charWidth() - viewFocusData_.leftFraction_ * clipRect.width());
    verticalScrollBar()->setValue((viewFocusData_.msaPoint_.y()-1.) * charHeight() - viewFocusData_.topFraction_ * clipRect.height());
}

/**
  * FocusPoint is in viewport space!
  *
  * @param focusPoint [const QPointF &]
  */
void AbstractMsaView::saveViewFocus(const QPointF &focusPoint)
{
    bool viewportContainsFocusPoint = focusPoint.x() >= 0
            && focusPoint.y() >= 0
            && focusPoint.x() < viewport()->width()
            && focusPoint.y() < viewport()->height();
    QPointF viewPoint;
    if (viewportContainsFocusPoint)
    {
        viewPoint = focusPoint;
    }
    else
    {
        // If the canvas is completetly contained within the viewport, then use the center of the canvas as the focus
        // point rather than the center of the viewport.
        QSizeF canvasSize = this->canvasSize();
        viewPoint.rx() = (viewport()->width() < canvasSize.width()) ? viewport()->width() / 2. : canvasSize.width() / 2.;
        viewPoint.ry() = (viewport()->height() < canvasSize.height()) ? viewport()->height() / 2. : canvasSize.height() / 2.;
    }

    QPointF canvasFocusPoint = pointRectMapper_.viewPointToCanvasPoint(viewPoint);
    viewFocusData_.msaPoint_ = pointRectMapper_.canvasPointToMsaPointF(canvasFocusPoint);
    QRectF clipRect = this->clipRect();
    viewFocusData_.leftFraction_ = (canvasFocusPoint.x() - clipRect.left()) / clipRect.width();
    viewFocusData_.topFraction_ = (canvasFocusPoint.y() - clipRect.top()) / clipRect.height();
}

/**
  * @returns PosiRect
  */
PosiRect AbstractMsaView::visibleSelectionRect() const
{
    return msaRegionClip_.intersection(selection());
}

/**
  * @see mouseMoveEvent(), setMouseHotSpotOffset()
  */
void AbstractMsaView::updateMouseCursorPoint()
{
    static QPoint lastMouseCursorPoint_;

    mouseCursorPoint_ = mouseCursorPoint();
    if (mouseCursorPoint_ != lastMouseCursorPoint_)
    {
        emit mouseCursorMoved(mouseCursorPoint_, lastMouseCursorPoint_);
        lastMouseCursorPoint_ = mouseCursorPoint_;
    }
}

/**
  * If a msa has not been defined or it is empty, the range for both scrollbars is clamped to zero. Otherwise, the range
  * is clamped to canvas dimensions minus the viewport dimensions.
  *
  * Currently, each single scroll bar step is equivalent to the 3 times the width and height of an individual character.
  * The page step is set to the height/width of the viewport.
  */
void AbstractMsaView::updateScrollBarRangesAndSteps()
{
    if (!msa_ || msa_->subseqCount() == 0)
    {
        horizontalScrollBar()->setRange(0, 0);
        verticalScrollBar()->setRange(0, 0);

        // Have the widget automatically paint it's background
        viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }
    else
    {
        QSize size = canvasSize();
        horizontalScrollBar()->setRange(0, size.width() - viewport()->width());
        verticalScrollBar()->setRange(0, size.height() - viewport()->height());

        // Small optimization :)
        // Only paint the background if it is partially visible
        viewport()->setAttribute(Qt::WA_OpaquePaintEvent, horizontalScrollBar()->isVisible() && verticalScrollBar()->isVisible());
    }

    // -----------------
    // Now for the steps
    // Make the step scale inversely to the zoom that way scrolling is more natural even at zoomed in/out levels
    horizontalScrollBar()->setSingleStep(qRound(3. * charWidth() / zoom_));
    verticalScrollBar()->setSingleStep(qRound(3. * charHeight() / zoom_));

    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setPageStep(viewport()->height());
}
