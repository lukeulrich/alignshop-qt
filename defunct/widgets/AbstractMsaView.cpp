/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AbstractMsaView.h"

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

#include <QtSvg/QSvgGenerator>

#include <cmath>

#include "Msa.h"
#include "global.h"

// During testing, the PointRectMapperPrivate methods are exposed via AbstractMsaView.h (see bottom of file); however,
// when not testing, we make it completely private by including the class declaration here.
#ifndef TESTING
#include "PointRectMapperPrivate.h"
#endif

#include <QtDebug>

#include "AbstractTextRenderer.h"
#include "ExactTextRenderer.h"          // To be removed!

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QWidget *]
  */
AbstractMsaView::AbstractMsaView(QWidget *parent) :
    QAbstractScrollArea(parent),
    positionalMsaColorProvider_(&defaultColorProvider_),
    msa_(0),
    operatingMode_(PanMode),
    zoom_(1.),
    zoomFactor_(.1),
    zoomMinimum_(.05),
    zoomMaximum_(16.),
    pointRectMapper_(new PointRectMapperPrivate(this)),
    panIsActive_(false),
    selectionIsActive_(false),
    slideIsActive_(false),
    gapInsertionIsActive_(false),
    temporaryPan_(false),
    temporaryPanIsActive_(false)
{
    // Map to monospace font, even on windows, which will not be able find font with the family of monospace. Thus, look
    // up the font with Qt's font matching algorithm (http://stackoverflow.com/questions/1468022/how-to-specify-monospace-fonts-for-cross-platform-qt-applications)
#if QT_VERSION >= 0x040700
    font_.setStyleHint(QFont::Monospace);
#else
    font_.setStyleHint(QFont::TypeWriter);
#endif

    font_.setFamily("Monospace");
    font_.setPointSize(QApplication::font().pointSize());

    // Enable mouse tracking so that we can receive mouse move events even when a mouse button is not pressed
    viewport()->setMouseTracking(true);

    keyAltPressed_ = false;
    keyControlPressed_ = false;
    keyShiftPressed_ = false;
    keyEPressed_ = false;
    keyTPressed_ = false;

    // Set the msa selection timer timeout
    msaSelectionScrollTimer_.setInterval(50);
    connect(&msaSelectionScrollTimer_, SIGNAL(timeout()), this, SLOT(onMsaSelectionScrollTimeout()));
}

/**
  * Imperative to free up any allocated memory. The positionalMsaColorProvider_ pointer may either point to the default
  * stack member, defaultColorProvider_, or a derivative supplied by the user (@see setPositionalMsaColorProvider).
  * If the latter, it is to be deleted. Otherwise, nothing is done.
  */
AbstractMsaView::~AbstractMsaView()
{
    // The necessary logic is already implemented in the following method, thus, free its memory by setting the
    // provider to zero.
    setPositionalMsaColorProvider(0);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * The returned rectangle is from:
  *    (horizontal scroll bar value,
  *     vertical scroll bar value) ->
  *                                (MIN(viewport.width, Msa length * character width),
  *                                 MIN(viewport.height, Msa sequences * character height))
  *
  * @returns QRectF
  */
QRectF AbstractMsaView::clipRect() const
{
    if (!msa_)
        return QRectF();

    return pointRectMapper_->viewRectToCanvasRect(viewport()->rect());
}

/**
  * @returns QPoint
  */
QPoint AbstractMsaView::editCursorPoint() const
{
    return editCursorPoint_;
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
  * @returns ViewMode
  */
AbstractMsaView::OperatingMode AbstractMsaView::mode() const
{
    return operatingMode_;
}

/**
  * @returns const Msa *
  */
const Msa *AbstractMsaView::msa() const
{
    return msa_;
}

/**
  * @returns const PositionalColorProvider *
  */
const PositionalMsaColorProvider *AbstractMsaView::positionalMsaColorProvider() const
{
    return positionalMsaColorProvider_;
}

/**
  * @returns Rect
  */
Rect AbstractMsaView::msaSelectionRect() const
{
    return Rect();
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
    if (positionalMsaColorProvider)
        positionalMsaColorProvider_ = positionalMsaColorProvider;
    else
        positionalMsaColorProvider_ = &defaultColorProvider_;
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
  * To be moved to external class!
  *
  * @param fileName [const QString &]
  * @param msaRegion [const Rect &]
  *
  * UNTESTED!
  */
void AbstractMsaView::saveSvg(const QString &fileName, const Rect &msaRegion) const
{
    QSvgGenerator svgGenerator;
    svgGenerator.setFileName(fileName);
    svgGenerator.setResolution(physicalDpiX());

    ExactTextRenderer renderer(font());

    Rect normalizedMsaRegion = msaRegion.normalized();

    QSizeF sizeF( (normalizedMsaRegion.width()+1) * renderer.width(),
                  (normalizedMsaRegion.height()+1) * renderer.height());
    svgGenerator.setSize(QSize(ceil(sizeF.width()), ceil(sizeF.height())));

    QPainter painter(&svgGenerator);
    renderMsaRegion(QPointF(0, 0), normalizedMsaRegion, &renderer, painter);
    painter.end();
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

    emit fontChanged();         // Inherited classes should respond to this signal and update their TextRenderer * accordingly

    if (msa_)
    {
        // Ensure that the charWidth and charHeight for the given zoom level is still above 1
        if (abstractTextRenderer()->width() <= abstractTextRenderer()->height())
        {
            if (charWidth() < 1.)
                setZoom(1. / charWidth());
        }
        else    // width > height
        {
            if (charHeight() < 1.)
                setZoom(1. / charHeight());
        }
    }

    // Given that the font has changed, update the scroll bar parameters, but not the position
    updateScrollBarRangesAndSteps();

    // Update the scroll bar positions accordingly
    if (msa_)
        restoreViewFocus();

    // Sometimes changing the font does not trigger a change in the current scroll position and in turn does not
    // update the screen. Thus, we call update to ensure that it gets repainted with every font change
    viewport()->update();
}

/**
  * @param mode [OperatingMode]
  */
void AbstractMsaView::setMode(OperatingMode mode)
{
    if (mode == operatingMode_)
        return;

    operatingMode_ = mode;

    updateMouseCursor();
    viewport()->update();   // To either show/hide the mouse cursor point

    emit modeChanged();
}

/**
  * Configure the view to observe and visualize msa. Watches the msa for changes to update the view appropriately. Does
  * not take ownership of the Msa.
  *
  * Additionally resets to the default selection rectangle, and sets the scroll bars to 0, 0.
  *
  * @param msa [Msa *]
  */
void AbstractMsaView::setMsa(Msa *msa)
{
    // Do nothing if the msa is the same
    if (msa == msa_)
        return;

    // If we already have a valid msa, disconnect any previously configured signals
    if (msa_)
    {
        disconnect(msa_, SIGNAL(gapColumnsInserted(int,int)),               this, SLOT(onMsaGapColumnsInserted(int,int)));
        disconnect(msa_, SIGNAL(gapColumnsRemoved(int)),                    this, SLOT(onMsaGapColumnsRemoved(int)));
        disconnect(msa_, SIGNAL(msaReset()),                                this, SLOT(onMsaReset()));
        disconnect(msa_, SIGNAL(regionSlid(int,int,int,int,int,int,int)),   this, SLOT(onMsaRegionSlid(int,int,int,int,int,int,int)));
        disconnect(msa_, SIGNAL(extendOrTrimFinished(int,int)),             this, SLOT(onMsaExtendOrTrimFinished(int,int)));
        disconnect(msa_, SIGNAL(collapsedLeft(MsaRect,int)),                this, SLOT(onMsaCollapsedLeft(MsaRect,int)));
        disconnect(msa_, SIGNAL(collapsedRight(MsaRect,int)),               this, SLOT(onMsaCollapsedRight(MsaRect,int)));
        disconnect(msa_, SIGNAL(subseqSwapped(int,int)),                    this, SLOT(onMsaSubseqSwapped(int,int)));
        disconnect(msa_, SIGNAL(subseqsInserted(int,int)),                  this, SLOT(onMsaSubseqsInserted(int,int)));
        disconnect(msa_, SIGNAL(subseqsMoved(int,int,int)),                 this, SLOT(onMsaSubseqsMoved(int,int,int)));
        disconnect(msa_, SIGNAL(subseqsRemoved(int,int)),                   this, SLOT(onMsaSubseqsRemoved(int,int)));
        disconnect(msa_, SIGNAL(subseqsSorted()),                           this, SLOT(onMsaSubseqsSorted()));
    }

    msa_ = msa;

    // If the new msa is not NULL, then hook up the appropriate signals
    if (msa_)
    {
        connect(msa_, SIGNAL(gapColumnsInserted(int,int)),               SLOT(onMsaGapColumnsInserted(int,int)));
        connect(msa_, SIGNAL(gapColumnsRemoved(int)),                    SLOT(onMsaGapColumnsRemoved(int)));
        connect(msa_, SIGNAL(msaReset()),                                SLOT(onMsaReset()));
        connect(msa_, SIGNAL(regionSlid(int,int,int,int,int,int,int)),   SLOT(onMsaRegionSlid(int,int,int,int,int,int,int)));
        connect(msa_, SIGNAL(extendOrTrimFinished(int,int)),             SLOT(onMsaExtendOrTrimFinished(int,int)));
        connect(msa_, SIGNAL(collapsedLeft(MsaRect,int)),                SLOT(onMsaCollapsedLeft(MsaRect,int)));
        connect(msa_, SIGNAL(collapsedRight(MsaRect,int)),               SLOT(onMsaCollapsedRight(MsaRect,int)));
        connect(msa_, SIGNAL(subseqSwapped(int,int)),                    SLOT(onMsaSubseqSwapped(int,int)));
        connect(msa_, SIGNAL(subseqsInserted(int,int)),                  SLOT(onMsaSubseqsInserted(int,int)));
        connect(msa_, SIGNAL(subseqsMoved(int,int,int)),                 SLOT(onMsaSubseqsMoved(int,int,int)));
        connect(msa_, SIGNAL(subseqsRemoved(int,int)),                   SLOT(onMsaSubseqsRemoved(int,int)));
        connect(msa_, SIGNAL(subseqsSorted()),                           SLOT(onMsaSubseqsSorted()));
    }

    updateMouseCursor();
    updateScrollBarRangesAndSteps();

    setDefaultSelection();
    setDefaultEditCursorPoint();

    // Reset the msaRegion so that it will be recalculated
    msaRegionClip_ = MsaRect();

    // Reset the scroll position to zero
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    // Reset the zoom - ideally this should be call the setZoom method; however, the setZoom method has lots of other
    // machinery and in particular requires a valid msa to do anything.
    zoom_ = qBound(zoomMinimum_, 1., zoomMaximum_);

    emit msaChanged();
}

/**
  * Wrapper object to zoom relative to the center of the view.
  *
  * @param zoom
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
    emit zoomChanged(zoom_);    // Inherited classes should respond to this signal and update their TextRenderer * accordingly

    // Check that char width and height are valid
    if (charWidth() >= 1. && charHeight() >= 1.)
    {
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
    emit zoomChanged(zoom_);
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

    // Update the msaRegionClip_
    setMsaRegionClip(pointRectMapper_->viewRectToMsaRect(viewport()->rect()));
    ASSERT(msaRegionClip_.isValid());

    drawBackground(painter);
    drawMsa(clipRenderOrigin(), msaRegionClip_, painter);
    MsaRect selRect(visibleSelectionRect());
    if (selRect.isValid())
        drawSelection(pointRectMapper_->msaRectToViewRect(selRect), painter);
    if (msaRegionClip_.contains(editCursorPoint_))
        drawEditCursor(pointRectMapper_->msaPointToViewRect(editCursorPoint_), painter);
    if (operatingMode_ == EditMode)
    {
        if (gapInsertionIsActive_)
        {
            int gapInsertColumn = this->gapInsertionColumn();
            // Important! in the msaPointToViewPoint calculation, we add or subtract .0001 in case we are attempting to
            // add at the boundary of the alignment. This prevents an assertion from being thrown. Also, we really
            // only need the x value; however, the msaPointToViewPoint method takes a Point. Thus we supply a dummy
            // value of 1 for the y position
            qreal x = pointRectMapper_->msaPointToViewPoint(QPointF(qBound(1., static_cast<qreal>(gapInsertColumn), gapInsertColumn - .0001),
                                                                    1.)).x();
            // Move the rendering position inside, if the gap is being inserted at either end of the alignment
            if (gapInsertColumn == 1)
                x += 1.;
            else if (gapInsertColumn == msa_->length()+1)
                x -= 1.;

            drawGapInsertionLine(x, painter);
        }
        else if (!mouseCursorPoint_.isNull() &&
                 !selectionIsActive_ &&
                 !slideIsActive_ &&
                 !isMouseOverSelection())
        {
            // Only draw the mouse cursor point if we are in edit mode, we have a valid mouseCursorPoint, and no selection
            // is currently being made
            drawMouseActivePoint(pointRectMapper_->msaPointToViewRect(mouseCursorPoint_), painter);
        }
    }
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::charHeight() const
{
    ASSERT_X(abstractTextRenderer(), "No abstract text renderer defined");

    return abstractTextRenderer()->height();
}

/**
  * @returns qreal
  */
qreal AbstractMsaView::charWidth() const
{
    ASSERT_X(abstractTextRenderer(), "No abstract text renderer defined");

    return abstractTextRenderer()->width();
}

/**
  * @returns bool
  */
bool AbstractMsaView::isMouseOverSelection() const
{
    return MsaRect(msaSelectionStart_, msaSelectionStop_).normalized().contains(mouseCursorPoint_);
}

/**
  * @returns QRect
  */
Rect AbstractMsaView::msaRegionClip() const
{
    return msaRegionClip_;
}

/**
  * @returns const PointRectMapperPrivate *
  */
const PointRectMapperPrivate *AbstractMsaView::pointRectMapper() const
{
    return pointRectMapper_.data();
}

/**
  * @param origin [const QPointF &]
  * @param msaRegion [const Rect &]
  * @param textRenderer [const TextRenderer *]
  * @param painter [const QPainter &]
  *
  * UNTESTED!
  */
void AbstractMsaView::renderMsaRegion(const QPointF &origin, const MsaRect &msaRegion, const AbstractTextRenderer *abstractTextRenderer, QPainter &painter) const
{
    ASSERT(positionalMsaColorProvider_);
    ASSERT(msa_);
    ASSERT(abstractTextRenderer);

    int top = msaRegion.top();
    int left = msaRegion.left();
    int bottom = msaRegion.bottom();
    int right = msaRegion.right();

    qreal charWidth = abstractTextRenderer->width();
    qreal charHeight = abstractTextRenderer->height();

    for (int i=0, y=top; y<=bottom; ++y, ++i)
    {
        // It is vital that the characters QByteArray object is present while accessing its contents with the const char
        // pointer! The following line does not work and will crash the program:
        //
        // const char *c = msa_->at(y)->bioString().sequence().toAscii().constData()
        //
        // For more information, see log.txt 18 February 2011
        QByteArray characters = msa_->at(y)->bioString().sequence().toAscii();
        const char *c = characters.constData() + left - 1;
        for (int j=0, x=left; x<=right; ++x, ++j)
        {
            abstractTextRenderer->drawChar(QPointF(origin.x() + j * charWidth,
                                                   origin.y() + i * charHeight),
                                           *c,
                                           positionalMsaColorProvider_->color(*msa_, y, x),
                                           &painter);
            ++c;
        }
    }
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
  */
void AbstractMsaView::setMsaRegionClip(const Rect &newMsaRegionClip)
{
    // ASSERT that newMsaRegionClip is within the Msa bounds
    ASSERT(msa_);
    ASSERT_X(newMsaRegionClip.left() > 0 && newMsaRegionClip.top() > 0, "newMsaRegionClip top, left out of range");
    ASSERT_X(newMsaRegionClip.right() <= msa_->length() && newMsaRegionClip.bottom() <= msa_->subseqCount(), "newMsaRegionClip bottom, right out of range");

    // newMsaRegionClip must not be expanded out from msaRegionClip_ on all sides
    ASSERT_X(!(newMsaRegionClip.left() < msaRegionClip_.left() &&
               newMsaRegionClip.right() > msaRegionClip_.right() &&
               newMsaRegionClip.top() < msaRegionClip_.top() &&
               newMsaRegionClip.bottom() < msaRegionClip_.bottom()), "Invalid (or rather unexpected) msa clip rectangle!");

    msaRegionClip_ = newMsaRegionClip;
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
  * @param keyEvent [QKeyEvent *]
  */
void AbstractMsaView::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
    case Qt::Key_Alt:
        keyAltPressed_ = true;
        break;
    case Qt::Key_Control:
        keyControlPressed_ = true;
        break;
    case Qt::Key_Shift:
        keyShiftPressed_ = true;
        break;
    case Qt::Key_Escape:
        if (!selectionIsActive_)
            setDefaultSelection();
        updateMouseCursor();
        viewport()->update();
        break;
    case Qt::Key_Minus:
        setZoomBy(-1);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        setZoomBy(1);
        break;
    case Qt::Key_0:
        if (keyControlPressed_)
            setZoom(1.);
        break;
    case Qt::Key_1:
        setZoom(1.);
        break;
    case Qt::Key_2:
        setZoom(.5);
        break;
    case Qt::Key_H:
        if (!selectionIsActive_ && !gapInsertionIsActive_)
            setMode(PanMode);
        break;
    case Qt::Key_S:
        if (!panIsActive_)
            setMode(EditMode);
        break;
    case Qt::Key_PageDown:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep());
        break;
    case Qt::Key_PageUp:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + -verticalScrollBar()->pageStep());
        break;
    case Qt::Key_Home:
        verticalScrollBar()->setValue(0);
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(0);
        break;
    case Qt::Key_End:
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        if (keyControlPressed_)
            horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
        break;
    case Qt::Key_Left:
        if (!keyShiftPressed_)
        {
            if (keyControlPressed_)
                horizontalScrollBar()->setValue(0);
            else
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep());
        }
        break;
    case Qt::Key_Right:
        if (!keyShiftPressed_)
        {
            if (keyControlPressed_)
                horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
            else
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep());
        }
        break;
    case Qt::Key_Up:
        if (keyControlPressed_)
            verticalScrollBar()->setValue(0);
        else
            verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep());
        break;
    case Qt::Key_Down:
        if (keyControlPressed_)
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        else
            verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep());
        break;
    }

    if (operatingMode_ == EditMode)
    {
        switch(keyEvent->key())
        {
        case Qt::Key_Space:
            if (!selectionIsActive_ && !slideIsActive_ && !keyEvent->isAutoRepeat())
            {
                temporaryPan_ = true;
                setMode(PanMode);
            }
            break;
        case Qt::Key_A:
            if (keyControlPressed_)
                selectAll();
            viewport()->update();
            break;
        case Qt::Key_G:
            // Remove all columns if Ctrl already pressed and this G is the first time it was pressed
            if (!keyEvent->isAutoRepeat())
            {
                if (keyControlPressed_)
                {
                    msa_->removeGapColumns();
                    return;
                }

                gapInsertionIsActive_ = true;   // To show the gap insertion line
                viewport()->update();
            }

            break;
        case Qt::Key_E:
            keyEPressed_ = true;
            if (keyControlPressed_)
            {
                MsaRect selection = MsaRect(msaSelectionStart_, msaSelectionStop_).normalized();
                if (msa_->length() / 2 - selection.left() >= selection.right() - msa_->length() / 2)
                    msa_->extendSubseqsLeft(selection.top(), selection.bottom(), selection.left());
                else
                    msa_->extendSubseqsRight(selection.top(), selection.bottom(), selection.right());
                viewport()->update();
            }
            break;
        case Qt::Key_T:
            keyTPressed_ = true;
            if (keyControlPressed_)
            {
                MsaRect selection = MsaRect(msaSelectionStart_, msaSelectionStop_).normalized();
                // Determine the half containing most of the selection
                if (msa_->length() / 2 - selection.left() >= selection.right() - msa_->length() / 2)
                    msa_->trimSubseqsLeft(selection.top(), selection.bottom(), selection.right());
                else
                    msa_->trimSubseqsRight(selection.top(), selection.bottom(), selection.left());
                viewport()->update();
            }
            break;
        case Qt::Key_R:
            if (keyControlPressed_)
            {
                MsaRect selection = MsaRect(msaSelectionStart_, msaSelectionStop_).normalized();
                msa_->levelSubseqsLeft(selection.top(), selection.bottom(), selection.left());
                viewport()->update();
            }
            break;
        case Qt::Key_Left:
            if (keyControlPressed_ && keyShiftPressed_)
            {
                msa_->collapseLeft(MsaRect(msaSelectionStart_, msaSelectionStop_));
                viewport()->update();
            }
            break;
        case Qt::Key_Right:
            if (keyControlPressed_ && keyShiftPressed_)
            {
                msa_->collapseRight(MsaRect(msaSelectionStart_, msaSelectionStop_));
                viewport()->update();
            }
            break;
        }
    }
}

/**
  * @param keyEvent [QKeyEvent *]
  */
void AbstractMsaView::keyReleaseEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key())
    {
    case Qt::Key_Alt:
        if (!keyEvent->isAutoRepeat())
            keyAltPressed_ = false;
        break;
    case Qt::Key_Control:
        if (!keyEvent->isAutoRepeat())
            keyControlPressed_ = false;
        break;
    case Qt::Key_Shift:
        if (!keyEvent->isAutoRepeat())
            keyShiftPressed_ = false;
        break;
    case Qt::Key_E:
        keyEPressed_ = false;
        break;
    case Qt::Key_T:
        keyTPressed_ = false;
        break;
    }

    if (operatingMode_ == EditMode)
    {
        switch(keyEvent->key())
        {
        case Qt::Key_G:
            if (!keyEvent->isAutoRepeat())
            {
                gapInsertionIsActive_ = false;
                viewport()->update();       // To hide the gap insertion line
            }
            break;
        }
    }
    else if (operatingMode_ == PanMode)
    {
        switch(keyEvent->key())
        {
        case Qt::Key_Space:
            if (!keyEvent->isAutoRepeat())
            {
                // The !panIsActive_ check avoids the unpleasant scenario where the user has pressed spacebar to initiate
                // a temporary pan mode, pressed the left mouse button to begin a pan, and then releases the space bar
                // before releasing the mouse button. Beforehand, this would immediately change modes to edit mode and leave
                // the partial pan operation in an unresolved state.
                //
                // If the above situation does occur, it is necessary in the mouseRelease event to set the mode to edit
                // mode if temporaryPan_ is false and mode is
                if (!panIsActive_ && temporaryPan_)
                    setMode(EditMode);

                temporaryPan_ = false;
            }
            break;
        }
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  * @returns bool
  */
bool AbstractMsaView::mouseEnterEvent()
{
    return false;
}

/**
  * @param mouseEvent [QMouseEvent *]
  * @returns bool
  */
bool AbstractMsaView::mouseLeaveEvent()
{
    mouseCursorPoint_ = QPoint();
    viewport()->update();

    return false;
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (operatingMode_ == PanMode)
    {
        if (mouseEvent->button() == Qt::LeftButton)
        {
            panAnchorPoint_ = mouseEvent->pos();
            lastPanScrollPosition_ = scrollPosition();
            panIsActive_ = true;
            if (temporaryPan_)
                temporaryPanIsActive_ = true;
            updateMouseCursor();
        }
    }
    else if (operatingMode_ == EditMode)
    {
        msaSelectionAnchorPoint_ = mouseEvent->pos();

        if (mouseEvent->button() == Qt::LeftButton)
        {
            if (!gapInsertionIsActive_)
            {
                if (isMouseOverSelection())
                {
                    // Initiate a slide operation
                    slideIsActive_ = true;
                    slideMsaAnchorPoint_ = pointRectMapper_->viewPointToMsaPoint(mouseEvent->pos());
                }
                else
                {
                    selectionIsActive_ = true;
                    setSelectionStartFromViewPoint(mouseEvent->pos());
                    viewport()->update();
                }
            }
        }
    }
}

/**
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    // Global updates regardless of mode
    bool mouseCursorPointChanged = false;
    QPoint newMouseCursorPoint = pointRectMapper_->viewPointToMsaPoint(mouseEvent->pos());
    if (newMouseCursorPoint != mouseCursorPoint_)
    {
        mouseCursorPoint_ = newMouseCursorPoint;
        mouseCursorPointChanged = true;
    }

    if (operatingMode_ == PanMode)
    {
        if (panIsActive_)
        {
            int dx = mouseEvent->x() - panAnchorPoint_.x();
            int dy = mouseEvent->y() - panAnchorPoint_.y();

            horizontalScrollBar()->setValue(lastPanScrollPosition_.x() - dx);
            verticalScrollBar()->setValue(lastPanScrollPosition_.y() - dy);

            panAnchorPoint_ = mouseEvent->pos();
            lastPanScrollPosition_ = scrollPosition();
        }
    }
    else if (operatingMode_ == EditMode)
    {
        if (mouseCursorPointChanged)
            viewport()->update();

        if (selectionIsActive_)
        {
            setSelectionStopFromViewPoint(mouseEvent->pos());
            viewport()->update();
        }
        else
        {
            if (slideIsActive_)
            {
                QPoint newSlideMsaPoint = pointRectMapper_->viewPointToMsaPoint(mouseEvent->pos());
                int msa_dx = newSlideMsaPoint.x() - slideMsaAnchorPoint_.x();
                if (msa_dx != 0)
                {
                    MsaRect mrect = MsaRect(msaSelectionStart_, msaSelectionStop_).normalized();
                    int actual_delta = msa_->slideRegion(mrect.x(), mrect.y(), mrect.right(), mrect.bottom(), msa_dx);

                    // Update the msa selection in accordance with how many residues were slid
                    if (actual_delta != 0)
                    {
                        msaSelectionStart_.rx() += actual_delta;
                        msaSelectionStop_.rx() += actual_delta;
                        slideMsaAnchorPoint_ = newSlideMsaPoint;
                    }
                }
            }

            if (gapInsertionIsActive_)
                viewport()->update();

            updateMouseCursor();
        }
    }
}

/**
  *
  *
  * @param mouseEvent [QMouseEvent *]
  */
void AbstractMsaView::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (operatingMode_ == PanMode)
    {
        if (mouseEvent->button() == Qt::LeftButton)
        {
            panIsActive_ = false;
            if (temporaryPanIsActive_)
            {
                temporaryPanIsActive_ = false;

                // User has entered temporary pan mode, released the space key while still holding the mouse button down
                // and finally released the mouse button thus ending the temporary pan operation. Since the space key
                // is no longer pressed, it is essential to switch modes back to editing
                if (!temporaryPan_)
                    setMode(EditMode);
            }
        }
    }
    else if (operatingMode_ == EditMode)
    {
        if (mouseEvent->button() == Qt::LeftButton)
        {
            selectionIsActive_ = false;
            slideIsActive_ = false;
            msaSelectionScrollTimer_.stop();

            if (gapInsertionIsActive_)
            {
                // Add a gap
                msa_->insertGapColumns(gapInsertionColumn(), 1);

                // Update the ranges
                updateScrollBarRangesAndSteps();
                viewport()->update();
            }

            // Attempt to insert a sequence character
            if (keyEPressed_)
            {
                QPoint p = pointRectMapper_->viewPointToMsaPoint(mouseEvent->pos());
                if (p.x() <= msa_->length() / 2.)
                {
                    if (msa_->at(p.y())->start() > 1)
                        msa_->setSubseqStart(p.y(), msa_->at(p.y())->start() - 1);
                }
                else if (msa_->at(p.y())->stop() < msa_->at(p.y())->anonSeq().bioString().length())
                    msa_->setSubseqStop(p.y(), msa_->at(p.y())->stop() + 1);
            }
            else if (keyTPressed_)
            {
                QPoint p = pointRectMapper_->viewPointToMsaPoint(mouseEvent->pos());
                if (p.x() <= msa_->length() / 2.)
                    msa_->setSubseqStart(p.y(), msa_->at(p.y())->start()+1);
                else if (msa_->at(p.y())->stop() > 1)
                    msa_->setSubseqStop(p.y(), msa_->at(p.y())->stop()-1);
            }

            viewport()->update();
        }
    }
    updateMouseCursor();
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
        return mouseEnterEvent();
    case QEvent::Leave:
        return mouseLeaveEvent();
    case QEvent::WindowDeactivate:
        // When the window is deactivated however that may be, disable any potentially active
        // actions
        panIsActive_ = false;
        keyAltPressed_ = false;
        keyControlPressed_ = false;
        keyShiftPressed_ = false;
        keyEPressed_ = false;
        keyTPressed_ = false;
        keyRPressed_ = false;
        selectionIsActive_ = false;
        slideIsActive_ = false;
        temporaryPan_ = false;
        temporaryPanIsActive_ = false;

    default:
        return QAbstractScrollArea::viewportEvent(event);
    }
}

/**
  *
  *
  * @param wheelEvent [QWheelEvent *]
  */
void AbstractMsaView::wheelEvent(QWheelEvent *wheelEvent)
{
    int numDegrees = wheelEvent->delta() / 8;
    int numSteps = -numDegrees / 15;

//    qDebug() << "Wheel Delta" << wheelEvent->delta();
//    qDebug() << "Wheel degrees" << numDegrees;
//    qDebug() << "Wheel steps" << numSteps;
//    qDebug() << keyControlPressed_;

    if (keyControlPressed_ && wheelEvent->orientation() == Qt::Vertical)
    {
        setZoomBy(-numSteps, wheelEvent->pos());
    }
    else
    {
        if (wheelEvent->orientation() == Qt::Horizontal)
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + numSteps * horizontalScrollBar()->singleStep());
        else
            verticalScrollBar()->setValue(verticalScrollBar()->value() + numSteps * verticalScrollBar()->singleStep());

        wheelEvent->accept();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param msaRect [const MsaRect &]
  * @param rightMostModifiedColumn [int]
  */
void AbstractMsaView::onMsaCollapsedLeft(const MsaRect &msaRect, int rightMostModifiedColumn)
{

}

/**
  * @param msaRect [const MsaRect &]
  * @param leftMostModifiedColumn [int]
  */
void AbstractMsaView::onMsaCollapsedRight(const MsaRect &msaRect, int leftMostModifiedColumn)
{

}

/**
  * @param start [int]
  * @param end [int]
  */
void AbstractMsaView::onMsaExtendOrTrimFinished(int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/**
  * @param column [int]
  * @param count [int]
  */
void AbstractMsaView::onMsaGapColumnsInserted(int column, int count)
{
    Q_UNUSED(column);
    Q_UNUSED(count);

    // Update the selection if necessary
    if (column > msaSelectionStop_.x())
        return;

    if (column > msaSelectionStart_.x())
        msaSelectionStop_.rx() += count;
    else // Column <= msaSelectionStart.x
    {
        msaSelectionStart_.rx() += count;
        msaSelectionStop_.rx() += count;
    }
}

/**
  *
  *
  * @param count [int]
  */
void AbstractMsaView::onMsaGapColumnsRemoved(int /* count */)
{
    updateScrollBarRangesAndSteps();

    // Check that the selection is still valid and constrain to msa boundaries as necessary
    if (msaSelectionStart_.x() > msa_->length())
        setDefaultSelection();
    else if (msaSelectionStop_.x() > msa_->length())
        msaSelectionStop_.rx() = msa_->length();

    // Make sure that the current mouse point is within range
    if (mouseCursorPoint_.x() > msa_->length())
        mouseCursorPoint_.rx() = msa_->length();

    viewport()->update();
}

/**
  * @param left [int]
  * @param top [int]
  * @param right [int]
  * @param bottom [int]
  * @param delta [int]
  * @param finalLeft [int]
  * @param finalRight [int]
  */
void AbstractMsaView::onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight)
{
    ASSERT_X(delta != 0, "Delta must be non zero");

    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
    Q_UNUSED(finalLeft);
    Q_UNUSED(finalRight);

    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  *
  *
  */
void AbstractMsaView::onMsaReset()
{
    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  * @param first [int]
  * @param second [int]
  */
void AbstractMsaView::onMsaSubseqSwapped(int first, int second)
{
    Q_UNUSED(first);
    Q_UNUSED(second);

    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void AbstractMsaView::onMsaSubseqsInserted(int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);

    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  * @param target [int]
  */
void AbstractMsaView::onMsaSubseqsMoved(int start, int end, int target)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(target);

    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  *
  *
  * @param start [int]
  * @param end [int]
  */
void AbstractMsaView::onMsaSubseqsRemoved(int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);

    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  *
  *
  */
void AbstractMsaView::onMsaSubseqsSorted()
{
    qDebug() << "Please implement:" << __FUNCTION__;
}

/**
  */
void AbstractMsaView::onMsaSelectionScrollTimeout()
{
    ASSERT(selectionIsActive_);

    int dx = 0;
    int dy = 0;

    QPoint curMousePos = viewport()->mapFromGlobal(QCursor::pos());

    // Determine horizontal scroll
    bool mouseXOutsideViewport = curMousePos.x() < 0 || curMousePos.x() >= viewport()->width();
    bool mouseYOutsideViewport = curMousePos.y() < 0 || curMousePos.y() >= viewport()->height();
    if (mouseXOutsideViewport)
    {
        if (curMousePos.x() > msaSelectionAnchorPoint_.x())
            // Went off the right side
            dx = curMousePos.x() - viewport()->width();
        else
            dx = curMousePos.x();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + dx);
    }

    if (mouseYOutsideViewport)
    {
        if (curMousePos.y() > msaSelectionAnchorPoint_.y())
            // Went off the bottom
            dy = curMousePos.y() - viewport()->height();
        else
            dy = curMousePos.y();

        verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
    }

    // Update the selection - even if the mouse has not moved
    setSelectionStopFromViewPoint(curMousePos);
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
  * AbstractMsaView provides a visual window into a part or all of the Msa. When rendering the Msa, it is only necessary
  * to render the portion of the Msa that is currently visible. For simplicity and convenience purposes, if any part of
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

    return QPointF(-(horizontalScrollBar()->value() - charWidth() * (msaRegionClip_.left() - 1)),
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

    QPointF canvasFocusPoint = pointRectMapper_->viewPointToCanvasPoint(viewPoint);
    viewFocusData_.msaPoint_ = pointRectMapper_->canvasPointToMsaPointF(canvasFocusPoint);
    QRectF clipRect = this->clipRect();
    viewFocusData_.leftFraction_ = (canvasFocusPoint.x() - clipRect.left()) / clipRect.width();
    viewFocusData_.topFraction_ = (canvasFocusPoint.y() - clipRect.top()) / clipRect.height();
}

/**
  *
  *
  * @returns Rect
  */
MsaRect AbstractMsaView::visibleSelectionRect() const
{
    return msaRegionClip_.intersection(selection());
}

/**
  * If msa_ is defined and has at least one sequence, then make the default edit cursor position (in Msa coordinates)
  * the very first character. Otherwise, set the editCursorPoint to an invalid/null QPoint.
  */
void AbstractMsaView::setDefaultEditCursorPoint()
{
    if (msa_ && msa_->subseqCount() > 0)
        editCursorPoint_ = QPoint(1, 1);
    else
        editCursorPoint_ = QPoint();
}

/**
  * If msa_ is defined, then make the default selection (in Msa coordinates) the very first character. Otherwise, set
  * the msaSelectionRect_ to an invalid/null QRect.
  */
void AbstractMsaView::setDefaultSelection()
{
    if (msa_)
        msaSelectionStart_ = msaSelectionStop_ = QPoint(1, 1);
    else
        msaSelectionStart_ = msaSelectionStop_ = QPoint();
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
    horizontalScrollBar()->setSingleStep(qRound(3. * charWidth()));
    verticalScrollBar()->setSingleStep(qRound(3. * charHeight()));

    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setPageStep(viewport()->height());
}

/**
  * To facilitate setting the appropriate cursor, all cursor changes should be placed in this method.
  */
void AbstractMsaView::updateMouseCursor()
{
    if (!msa_)
    {
        viewport()->setCursor(Qt::ArrowCursor);
        return;
    }

    switch (operatingMode_)
    {
    case PanMode:
        if (panIsActive_)
            viewport()->setCursor(Qt::ClosedHandCursor);
        else
            viewport()->setCursor(Qt::OpenHandCursor);
        break;
    case EditMode:
        if (selectionIsActive_)
            viewport()->setCursor(Qt::ArrowCursor);
        else    // Selection is not active
        {
            if (slideIsActive_ || isMouseOverSelection())
                viewport()->setCursor(Qt::SizeHorCursor);
            else
                viewport()->setCursor(Qt::ArrowCursor);
        }
    }
}

/**
  *
  */
int AbstractMsaView::gapInsertionColumn() const
{
    return pointRectMapper_->viewPointToMsaPointF(mapFromGlobal(QCursor::pos())).x() + .5;
}

/**
  * @returns MsaRect
  */
MsaRect AbstractMsaView::selection() const
{
    return MsaRect(msaSelectionStart_, msaSelectionStop_);
}

/**
  */
void AbstractMsaView::selectAll()
{
    msaSelectionStart_ = QPoint(1, 1);
    msaSelectionStop_ = QPoint(msa_->length(), msa_->subseqCount());
}

void AbstractMsaView::setSelectionStartFromViewPoint(const QPointF &viewPoint)
{
    // If shift is not pressed, we are initiating a new selection start point vs extending the existing selection
    if (!keyShiftPressed_)
    {
        msaSelectionStart_ = pointRectMapper_->viewPointToMsaPoint(viewPoint);

        // If alt is pressed, then we are in column selection mode; move the selection start top to 1
        if (keyAltPressed_)
            msaSelectionStart_.ry() = 1;
    }
    msaSelectionStop_ = pointRectMapper_->viewPointToMsaPoint(viewPoint);

    // If Alt is pressed = column selection mode; move selection stop bottom to last sequence
    if (keyAltPressed_)
        msaSelectionStop_.ry() = msa_->subseqCount();
}

void AbstractMsaView::setSelectionStopFromViewPoint(const QPointF &viewPoint)
{
    ASSERT_X(selectionIsActive_, "Selection must be active");

    msaSelectionStop_ = pointRectMapper_->viewPointToMsaPoint(viewPoint);

    // If Alt is pressed = column selection mode; move selection stop bottom to last sequence
    if (keyAltPressed_)
    {
        msaSelectionStop_.ry() = msa_->subseqCount();

        // Additionally, we set the selection start top to 1 to handle the case where the user starts a selection,
        // and then presses the Alt key
        msaSelectionStart_.ry() = 1;
    }

    // Automatically scroll if viewPoint is outside the viewport() rect bounds
    if (viewport()->rect().contains(viewPoint.toPoint()))
        msaSelectionScrollTimer_.stop();
    else if (!msaSelectionScrollTimer_.isActive())
        // Mouse has left the viewport area while selecting, initiate automatic scrolling
        msaSelectionScrollTimer_.start();
}




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param abstractMsaView [AbstractMsaView *]
  * @param resolution [int]
  */
PointRectMapperPrivate::PointRectMapperPrivate(AbstractMsaView *abstractMsaView)
    : abstractMsaView_(abstractMsaView)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractMsaView *
  */
AbstractMsaView *PointRectMapperPrivate::abstractMsaView() const
{
    return abstractMsaView_;
}

/**
  * @param abstractMsaView [AbstractMsaView *]
  */
void PointRectMapperPrivate::setAbstractMsaView(AbstractMsaView *abstractMsaView)
{
    abstractMsaView_ = abstractMsaView;
}

/**
  * @param canvasPoint [const QPoint &]
  * @returns QPoint
  */
QPoint PointRectMapperPrivate::canvasPointToMsaPoint(const QPointF &canvasPointF) const
{
    return floorPoint(canvasPointToMsaPointF(canvasPointF));
}

/**
  * @param canvasPoint [const QPoint &]
  * @returns QPoint
  */
QPointF PointRectMapperPrivate::canvasPointToMsaPointF(const QPointF &canvasPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    ASSERT_X(abstractMsaView_->msa_, "msa has not been defined");
    ASSERT_X(canvasPointF.x() >= 0 && canvasPointF.x() < abstractMsaView_->canvasSizeF().width(), "canvasPointF.x() out of range");
    ASSERT_X(canvasPointF.y() >= 0 && canvasPointF.y() < abstractMsaView_->canvasSizeF().height(), "canvasPointF.y() out of range");

    // The value is within the proper bounds, now perform the calculation
    return unboundedCanvasPointToMsaPointF(canvasPointF);
}

/**
  * @param canvasRect [const Rect &]
  * @returns MsaRect
  */
MsaRect PointRectMapperPrivate::canvasRectToMsaRect(const Rect &canvasRect) const
{
    return canvasRectFToMsaRect(canvasRect.toQRectF());

    if (!abstractMsaView_)
        return MsaRect();

    Rect normalizedRect = canvasRect.normalized();
    ASSERT_X(normalizedRect.width() > 0 && normalizedRect.height() > 0, "normalized rect must have a non-zero width and height");
    ASSERT_X(Rect(QPoint(0, 0), abstractMsaView_->canvasSize()).contains(canvasRect.normalized()), "rect outside of canvas boundaries");



    return MsaRect(canvasPointToMsaPoint(normalizedRect.topLeft()),
                   canvasPointToMsaPoint(QPointF(normalizedRect.right() - 1,
                                                 normalizedRect.bottom() - 1)));
}

/**
  * If the normalized canvasRectF has a zero width or height a default Rect is returned.
  *
  * @param canvasRect [const QRectF &]
  * @returns MsaRect
  */
MsaRect PointRectMapperPrivate::canvasRectFToMsaRect(const QRectF &canvasRectF) const
{
    if (!abstractMsaView_)
        return Rect();

    QRectF normalizedRect = canvasRectF.normalized();
    if (normalizedRect.width() == 0 || normalizedRect.height() == 0)
        return Rect();

    ASSERT_X(QRectF(QPointF(0, 0), abstractMsaView_->canvasSizeF()).contains(normalizedRect), "rect outside of canvas boundaries");

    // The above statements ensure that we are within the proper canvas boundaries
    return MsaRect(unboundedCanvasPointToMsaPoint(normalizedRect.topLeft()),
                   QPoint(ceil(normalizedRect.right() / abstractMsaView_->charWidth()),
                          ceil(normalizedRect.bottom() / abstractMsaView_->charHeight())));
}

/**
  * msaPoint may either be fractional or integral.
  *
  * @param msaPoint  [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapperPrivate::msaPointToCanvasPoint(const QPointF &msaPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    ASSERT_X(!msaPointF.isNull(), "msaPoint must not be null");
    ASSERT_X(msaPointF.x() >= 1. && msaPointF.y() >= 1., "msaPoint out of range");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible Msa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msa not defined");
    else if (msaPointF.x() >= abstractMsaView_->msa_->length()+1 || msaPointF.y() >= abstractMsaView_->msa_->subseqCount()+1)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msaPointF (%.1f, %.1f) outside msa boundaries (%d, %d)", msaPointF.x(), msaPointF.y(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
#endif
#endif

    return QPointF((msaPointF.x()-1.) * abstractMsaView_->charWidth(),
                   (msaPointF.y()-1.) * abstractMsaView_->charHeight());
}


/**
  * @param msaPoint [const QPoint &]
  * @returns QRectF
  */
QRectF PointRectMapperPrivate::msaPointToCanvasRect(const QPoint &msaPoint) const
{
    if (!abstractMsaView_)
        return QRectF();

    ASSERT_X(!msaPoint.isNull(), "msaPoint must not be null");
    ASSERT_X(msaPoint.x() > 0 && msaPoint.y() > 0, "msaPoint out of range");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible Msa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msa not defined");
    else if (msaPoint.x() > abstractMsaView_->msa_->length() || msaPoint.y() > abstractMsaView_->msa_->subseqCount())
        qWarning("AbstractMsaView::msaPointToCanvasPoint - msaPoint (%d, %d) outside msa boundaries (%d, %d)", msaPoint.x(), msaPoint.y(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
#endif
#endif

    return QRectF(msaPointToCanvasPoint(msaPoint),
                  QSizeF(abstractMsaView_->charWidth(), abstractMsaView_->charHeight()));
}

/**
  * MsaRect must have x and y greater than 0 and x + width > 0 and y + height > 0. No other checking is performed to
  * ensure that MsaRect is a valid rectangle within the alignment.
  *
  * All rectangular coordinates in any quadrant are converted into a normalized representation and a valid rectangle is
  * returned.
  *
  * @param msaRect [const MsaRect &]
  * @returns QRectF
  */
QRectF PointRectMapperPrivate::msaRectToCanvasRect(const Rect &msaRect) const
{
    if (!abstractMsaView_)
        return QRectF();

    // Custom normalization with respect to 1-based Msa coordinates
    Rect normalizedRect = msaRect.normalized();
    ASSERT_X(normalizedRect.left() > 0 && normalizedRect.top() > 0, "rect left and top must both be greater than zero");
    ASSERT_X(normalizedRect.right() > 0 && normalizedRect.bottom() > 0, "rect right and bottom must be greater than zero");

#ifdef QT_DEBUG
#ifndef TESTING
    // Print to console if point is outside the permissible Msa boundaries
    if (!abstractMsaView_->msa_)
        qWarning("AbstractMsaView::msaRectToCanvasRect - msa not defined");
    else if (normalizedRect.left() > abstractMsaView_->msa_->length()
             || normalizedRect.right() > abstractMsaView_->msa_->length()
             || normalizedRect.top() > abstractMsaView_->msa_->subseqCount()
             || normalizedRect.bottom() > abstractMsaView_->msa_->subseqCount())
    {
        qWarning("AbstractMsaView::msaRectToCanvasRect - rect [(%d, %d) -> (%d, %d)] outside msa boundaries (%d, %d)",
                 normalizedRect.left(), normalizedRect.top(), normalizedRect.right(), normalizedRect.bottom(), abstractMsaView_->msa_->length(), abstractMsaView_->msa_->subseqCount());
    }
#endif
#endif

    return QRectF(msaPointToCanvasPoint(normalizedRect.topLeft()),
                  QSizeF((normalizedRect.width()+1) * abstractMsaView_->charWidth(),
                         (normalizedRect.height()+1) * abstractMsaView_->charHeight()));
}

/**
  * The view space is simply a translated representation of the canvas space. Therefore, conversion simply requires
  * a translation operation.
  *
  * @param canvasPoint [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapperPrivate::canvasPointToViewPoint(const QPointF &canvasPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    ASSERT_X(abstractMsaView_->msa_, "msa has not been defined");
    ASSERT_X(canvasPointF.x() >= 0 && canvasPointF.x() < abstractMsaView_->canvasSizeF().width(), "canvasPointF.x() out of range");
    ASSERT_X(canvasPointF.y() >= 0 && canvasPointF.y() < abstractMsaView_->canvasSizeF().height(), "canvasPointF.y() out of range");

    QPointF viewOrigin(abstractMsaView_->horizontalScrollBar()->value(),
                       abstractMsaView_->verticalScrollBar()->value());

    return canvasPointF - viewOrigin;
}

/**
  * The argument, canvasRectF, must be contained in the actual canvas; however, its resulting point in view space does
  * not necessarily need to be within the visible viewport boundaries.
  *
  * @param canvasRect [const QRectF &]
  * @returns QRectF
  */
QRectF PointRectMapperPrivate::canvasRectToViewRect(const QRectF &canvasRectF) const
{
    if (!abstractMsaView_)
        return QRectF();

    QRectF normalizedRect = canvasRectF.normalized();
    ASSERT_X(normalizedRect.width() > 0 && normalizedRect.height() > 0, "normalized rect must have a non-zero width and height");
    ASSERT_X(QRectF(QPointF(0, 0), abstractMsaView_->canvasSizeF()).contains(normalizedRect), "canvasRectF outside of canvas boundaries");

    return QRectF(canvasPointToViewPoint(normalizedRect.topLeft()),
                  canvasRectF.size());
}

/**
  * Because the view space is completely unbounded, it is possible for a viewpoint to not have a directly corresponding
  * value in the canvas space. In these cases, the closest canvas point is returned. Because no individual canvas point
  * can exist on the bottom/right-most edges of the canvas space, the actual canvas value returned for viewpoints
  * greater than the canvas size will be the canvassize - .0001.
  *
  * @param viewPointF [const QPointF &]
  * @returns QPointF
  * @see viewRectToCanvasRect()
  */
QPointF PointRectMapperPrivate::viewPointToCanvasPoint(const QPointF &viewPointF) const
{
    if (!abstractMsaView_)
        return QPointF();

    QSizeF canvasSizeF = abstractMsaView_->canvasSizeF();

    return QPointF(qBound(0., viewPointF.x() + abstractMsaView_->horizontalScrollBar()->value(), canvasSizeF.width()-.0001),
                   qBound(0., viewPointF.y() + abstractMsaView_->verticalScrollBar()->value(), canvasSizeF.height()-.0001));
}

/**
  * Similar logic applies here regarding the viewpoint -> canvas mapping as described in viewPointToCanvasPoint.
  *
  * @param viewRectF [const QRectF &]
  * @returns QRectF
  * @see viewPointToCanvasPoint()
  */
QRectF PointRectMapperPrivate::viewRectToCanvasRect(const QRectF &viewRectF) const
{
    if (!abstractMsaView_)
        return QRectF();

    QRectF normalizedRect = viewRectF.normalized();
    return QRectF(viewPointToCanvasPoint(normalizedRect.topLeft()),
                  viewPointToCanvasPoint(normalizedRect.bottomRight()));
}

/**
  * @param viewPointF [const QPointF &]
  * @returns QPoint
  */
QPoint PointRectMapperPrivate::viewPointToMsaPoint(const QPointF &viewPointF) const
{
    return floorPoint(viewPointToMsaPointF(viewPointF));
}

/**
  * @param viewPointF [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapperPrivate::viewPointToMsaPointF(const QPointF &viewPointF) const
{
    return canvasPointToMsaPointF(viewPointToCanvasPoint(viewPointF));
}

/**
  * @param msaPointF [const QPointF &]
  * @returns QPointF
  */
QPointF PointRectMapperPrivate::msaPointToViewPoint(const QPointF &msaPointF) const
{
    return canvasPointToViewPoint(msaPointToCanvasPoint(msaPointF));
}

/**
  * @param msaPoint [const QPoint &]
  * @returns QPointF
  */
QRectF PointRectMapperPrivate::msaPointToViewRect(const QPoint &msaPoint) const
{
    return canvasRectToViewRect(msaPointToCanvasRect(msaPoint));
}

/**
  * @param viewRectF [const QRectF &]
  * @returns Rect
  */
MsaRect PointRectMapperPrivate::viewRectToMsaRect(const QRectF &viewRectF) const
{
    return canvasRectFToMsaRect(viewRectToCanvasRect(viewRectF));
}

/**
  * @param msaRect [const MsaRect &]
  * @returns QRectF
  */
QRectF PointRectMapperPrivate::msaRectToViewRect(const MsaRect &msaRect) const
{
    return canvasRectToViewRect(msaRectToCanvasRect(msaRect));
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param canvasPointF
  * @returns QPointF
  */
QPointF PointRectMapperPrivate::unboundedCanvasPointToMsaPointF(const QPointF &canvasPointF) const
{
    ASSERT(abstractMsaView_);

    // Must add 1 to both the x and y derived values to make it 1-based
    return QPointF(1. + canvasPointF.x() / abstractMsaView_->charWidth(),
                   1. + canvasPointF.y() / abstractMsaView_->charHeight());
}

/**
  * @param canvasPointF
  * @returns QPoint
  */
QPoint PointRectMapperPrivate::unboundedCanvasPointToMsaPoint(const QPointF &canvasPointF) const
{
    return floorPoint(unboundedCanvasPointToMsaPointF(canvasPointF));
}
