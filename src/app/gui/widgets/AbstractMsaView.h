/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSAVIEW_H
#define ABSTRACTMSAVIEW_H

#include <QtCore/QScopedPointer>

#include <QtGui/QAbstractScrollArea>
#include <QtGui/QFont>
#include <QtGui/QPixmap>

#include "../util/PointRectMapper.h"

#include "../../graphics/PositionalMsaColorProvider.h"
#include "../../graphics/TextPixmapRenderer.h"

#include "../../core/util/Rect.h"
#include "../../core/util/PosiRect.h"
#include "../../core/PODs/SubseqChangePod.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QUndoStack;

class ObservableMsa;

// Given these specific type names for clarity sake when working with the code
// typedef QPoint MsaLocus;        // 1-based coordinate corresponding to a specific row/column within a Msa
// typedef QRect MsaRegion;        // Two MsaLoci that define the bounds of a specific rectangular region within a Msa

class AbstractTextRenderer;
class IMsaTool;
class IRenderEngine;
class MsaStartStopSideWidget;
class MsaRulerWidget;
class VerticalMsaMarginWidget;

/**
  * MsaView is a robust graphical control for viewing and manipulating Msas.
  *
  * Depending on the font and zoom, most Msa's will not usually fit within the typical user's screen resolution. Thus, a
  * primary goal is to make it possible to easily pan and zoom a subsection of the Msa in a similar style as Adobe
  * Acrobat does with their Reader program.
  *
  * On a similar note, it is vital to be able to edit a Msa using the mouse for selection and navigation. Because the
  * natural method for pan and making a selection (mouse down, mouse drag, mouse up) are identical, it is necessary to
  * provide a mechanism for switching between pan and edit modes. This is accomplished by setting the view to a valid
  * operating mode (@see OperatingMode) with the method, setMode(). Oftentimes, a selection will need to be made that
  * extends beyond the currently viewable area. To account for this, when the user drags their mouse beyond the view's
  * edges, the view automatically scrolls (until they release their mouse). Zooming may be done in any mode via the
  * mouse wheel or alternatively, user's may enter the zoom mode which has additional methods for handling zoom
  * requests.
  *
  * The actual colors used for display are abstracted out via a PositionalColorProvider, which by default is a basic
  * PositionColorProvider which simply returns black text on a white background. If a null PositionalColorProvider is
  * supplied, the default one is used.
  *
  * EditCursor is the edit action anchor point - displayed as selection of single character; inverted within selection;
  *            always visible; current keyboard location
  * MouseCursor simply shows character the mouse is currently over; displayed as an outlined box; inverted when within
  *             selection. Hidden when user controls EditCursor with keyboard or not in EditMode
  *
  * Operating modes and controls:
  * o PanMode (current selection and cursor location remain unchanged and visible)
  *   - Pan: LMouseDown, LMouseDrag, LMouseUp
  *   - Pan: Arrow keys; holding arrow key down should continuously scroll
  *   - Pan: MWheel; including left/right on supported mice; ignored if inside mouse move
  *
  * o ZoomMode
  *   - LMouseDown
  *
  * o EditMode
  *   - Cursor: LMouseDown places cursor at character under mouse or character nearest mouse position (e.g. user clicks
  *             on area outside the rendered alignment region)
  *   - Cursor: Arrow keys move cursor; clear selection if exit outside of selection
  *   - Selection: LMouseDown place cursor at character under mouse or character nearest mouse position ; clears any
  *                previous selection unless click is within selection (that should being slide move)
  *   - Selection: LMouseDrag extends selection from initial cursor position
  *   - Selection: LMouseUp selection ends; ignore if not LMouseDown begin within canvas (e.g. user clicks left mouse
  *                drags into canvas, and then releases)
  *   - Selection: Arrow keys + ShiftDown to extend selection
  *   - Slide: LMouseDown within selection; LMouseDrag slides characters horizontally as much as is possible; LMouseUp
  *            complete slide operation
  *   - Slide: Left/Right + CtrlDown
  *   - Intra-slide (sliding characters within selection, but not moving the selection): Shift + LMouseDrag when sliding
  *   - Show gap column insertion line: Space key
  *   - Insert gap column: Space key + (LMouseDown OR Enter Key)
  *
  * o All modes:
  *   - Zoom In: Ctrl + MWheel-Up
  *   - Zoom Out: Ctrl + MWheel-Down
  *
  * In a typical QAbstractScrollArea derivative, the user may call setViewport(QWidget *) to configure the actual canvas
  * to utilize for all painting operations. For software rendering, this method never needs to be called because a
  * default QWidget is used automatically; however, to facilitate painting with other engines (using the QPainter API)
  * it is exposed. The only other mechanism I know of at this point is passing a QGLWidget instance to this method,
  * which in turn will utilize the OpenGL paint engine. QGraphicsView utilizes this approach. A downside though is that
  * all rendering to this widget must be done in QAbstractScrollArea's paint event - even if there is a paintEvent
  * handler defined for this widget. This occurs because QAbstractScrollArea takes ownership of the widget and installs
  * an event handler to intercept several of the most common events including the paintEvent.
  *
  * The goal with MsaView is to have this base class provide both an interface for drawing all the components of a Msa
  * and additionally provide a software rendering implementation. Moreover, other classes should be able to derive from
  * MsaView and provide an accelerated renderer implementation of the interface methods (e.g. using OpenGL). To achieve
  * this goal, MsaView::setViewport() is protected and thus will always use the default QWidget. Derived classes may
  * call this method with a QWidget derivative (e.g. QGLWidget) of their choice.
  *
  * The current selection is dynamically updated in response to user events. For example, if the user inserts a gap
  * column in the middle of a selection, the selection should increase horizontally by the number of gaps inserted.
  * Conversely, if the selection contains gap columns that are removed, the selection should be reduced by that amount.
  * If the selection is completely contained within the gap region, it should be reduced to the edit cursor and the
  * edit cursor moved to the top left most character beside the selection before it was removed. The same concepts apply
  * when sequences are inserted/removed from the Msa. If the sequences are sorted, then the selection should be cleared.
  *
  * Model applications:
  * o Google Sketchup
  * o Inkscape
  *
  * General notes:
  * o Canvas: virtual space occuped by entire msa rendering
  * o Clip: visible window of canvas
  * o Only monospace fonts are supported
  * o Scroll bars
  *   - single step = rounded width/height of one character
  *   - page step = width/height of widget geometry
  * o All scrolling (programmatically or user-based) must be done via the scroll bar commands (setValue)
  *
  * >> TODO:
  * - ReadOnly operating mode
  * - Cursor when in edit mode - basically box around residue under mouse
  * - Guide lines: created by clicking on edge ruler? maybe not; Hide/show via the pipe symbol
  * - Context menu with options
  * - keyboard controls
  * - The selection should be able to be extended by dragging the selection borders with the mouse.
  * - Removal of gap columns should properly preserve/tweak the selection relative to the gap columns removed. Currently
  *   it simply shifts the selection.
  */
class AbstractMsaView : public QAbstractScrollArea
{
    Q_OBJECT

    Q_PROPERTY(double zoomFactor    READ zoomFactor     WRITE setZoomFactor);
    Q_PROPERTY(double zoomMinimum   READ zoomMinimum    WRITE setZoomMinimum);
    Q_PROPERTY(double zoomMaximum   READ zoomMaximum    WRITE setZoomMaximum);

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit AbstractMsaView(QWidget *parent = 0);                  //!< Construct and initialize instance
    virtual ~AbstractMsaView();                                     //!< Virtual destructor for supporting inheritance

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void centerOn(const PosiRect &msaRect);                         //!< Scrolls the view such that msaRect is optimally centered
    qreal baseline() const;                                         //!< Returns the text rendering baseline for all alignment characters
    qreal charHeight() const;                                       //!< Convenience function that returns the height of an individual character block
    qreal charWidth() const;                                        //!< Convenience function that returns the width of an individual character block
    QRectF clipRect() const;                                        //!< Returns the currently visible region of the alignment
    IMsaTool *currentMsaTool() const;                               //!< Returns the current msa tool
    QFont font() const;                                             //!< Returns the rendering font
    bool isMouseOverSelection() const;                              //!< Convenience function that returns true if the mouse is over the selection; false otherwise
    bool isMouseCursorPointVisible() const;                         //!< Returns true if the mouse cursor point is visible
    bool isPartlyVisible(const PosiRect &msaRect) const;            //!< Returns true if msaRect is partly within the viewable area; false otherwise
    QPoint mouseCursorPoint() const;                                //!< Returns the constrained point in Msa space that corresponds to the current mouse position
    QPoint mouseHotSpot() const;                                    //!< Returns the actionable point of the mouse cursor point which takes into account any hot spot offset
    QPoint mouseHotSpotOffset() const;                              //!< Returns the current mouse hot spot offset
    ObservableMsa *msa() const;                                     //!< Returns the Msa being viewed
    const PointRectMapper *pointRectMapper() const;                 //!< Returns the point rect mapper for this msa view
    //!< Returns the active color provider
    const PositionalMsaColorProvider *positionalMsaColorProvider() const;
    //! Convenience method for rendering msaRect at origin using renderEngine and painter
    void renderMsaRegion(const QPointF &origin, const PosiRect &msaRect, IRenderEngine *renderEngine, QPainter *painter);
    virtual IRenderEngine *renderEngine() const = 0;                //!< Return the current rendering engine
    MsaRulerWidget *ruler() const;                                  //!< Returns a pointer to the msaRulerWidget
    void scroll(Qt::Orientation orientation, int numSteps) const;   //!< Scroll the view in orientation by numSteps
    QPoint scrollPosition() const;                                  //!< Convenience method that returns the horizontal and vertical scroll bar positions as a QPoint
    PosiRect selection() const;                                     //!< Returns the Msa coordinates for the current selection
    //! Sets the active msa tool to msaTool; if isTemporary then resets to current tool when keyboardKey is released
    void setCurrentMsaTool(IMsaTool *msaTool, bool isTemporary = false, int keyboardKey = 0);
    void setMouseHotSpotOffset(const QPoint &hotSpotOffset);        //!< Sets the mouse hot spot offset to hotSpotOffset
    //! Sets the PositionalMsaColorProvider to positionalMsaColorProvider or the default PositionalMsaColorProvider if positionalMsaColorProvider is zero
    void setPositionalMsaColorProvider(PositionalMsaColorProvider *positionalMsaColorProvider);
    //! Sets the selection to range from startMsaPoint to stopMsaPoint
    void setSelectionStart(const QPoint &msaPoint);                 //!< Sets the selection start to msaPoint
    void setSelectionStop(const QPoint &msaPoint);                  //!< Sets the selection stop to msaPoint
    void setRenderXShift(qreal xShift);                             //!< Sets the amount to translate the x-axis render origin
    void setUndoStack(QUndoStack *undoStack);                       //!< Sets the undo stack to undoStack
    void setVerticalMsaMarginWidget(VerticalMsaMarginWidget * verticalMsaMarginWidget);
    qreal renderXShift() const;                                     //!< Returns the current translation amount for the x-axis
    QUndoStack *undoStack() const;                                  //!< Returns the current undo stack
    void updateMarginWidgetGeometries();                            //!< Updates the viewport width and properly positions the side widgets; normally does not need to be called externally
    QMargins viewportMargins() const;                               //!< Returns the current viewport margins
    qreal zoom() const;                                             //!< Returns the zoom
    double zoomFactor() const;                                      //!< Returns the zoom factor
    double zoomMinimum() const;                                     //!< Returns the minimum zoom value
    double zoomMaximum() const;                                     //!< Returns the maximum zoom value


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void clearSelection();                                          //!< Clears the selection completely
    void hideMouseCursorPoint();                                    //!< Hides the mouse cursor point
    virtual void repaintRow(int msaRow);                            //!< Forces a repaint of at least the row at msaRow (1-based); this base class implementation simply submits an update request
    void selectAll();                                               //!< Selects the entire alignment
    void setFont(const QFont &font);                                //!< Sets the rendering font to font (must be a monospace compatible) and returns true on success or false otherwise
    void setMsa(ObservableMsa *msa);                                //!< Sets the msa being visualized to msa
    void setMsaRulerVisible(bool visible);                          //!< Sets the msa ruler visibility to visible
    void setSelection(const PosiRect &msaRect);                     //!< Sets the current selection to msaRect
    void setStartSideWidgetVisible(bool visible);                   //!< Sets the vertical start position side widget's visibility to visible
    void setStopSideWidgetVisible(bool visible);                    //!< Sets the vertical stop position side widget's visibility to visible
    void setBottomMarginWidgetVisible(bool visible);
    void setZoom(double zoom);                                      //!< Sets the zoom to zoom
    void setZoom(double zoom, const QPointF &focus);
    //!< Zoom by zoomFactor_ amount times relative to focus; if either the focus x or y is negative or the focus is outside the viewport range, then relative to center of view
    void setZoomBy(int amount, const QPointF focus = QPointF(-1, -1));
    void setZoomFactor(double zoomFactor);                          //!< Sets the zoom factor to zoomFactor; does nothing if zoomFactor is less than or equal to zero
    void setZoomMinimum(double zoomMinimum);                        //!< Sets the miminum zoom value to zoomMinimum; zoomMinimum must be greater than zero. If zoomMinimum exceeds zoomMaximum, zooMaximum is set to zoomMinimum
    void setZoomMaximum(double zoomMaximum);                        //!< Sets the maximum zoom value to zoomMaximum; zoomMaximum must be greater than zoomMinimum
    void showMouseCursorPoint();                                    //!< Shows the mouse cursor point


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void msaToolChanged();                                          //!< Emitted when the current msa tool has changed
    void editCursorMoved(QPoint point, QPoint previousPoint);       //!< Emitted when the edit cursor has moved positions from previousPoint to point (1-based, Msa coordinates)
    void fontChanged();                                             //!< Emitted when the font has changed
    void mouseCursorMoved(QPoint point, QPoint previousPoint);      //!< Emitted when the mouse cursor has moved positions from previousPoint to point (1-based, Msa coordinates)
    void mouseCursorVisibleChanged(bool visible);                   //!< Emitted when the mouse cursor visibility changes to visible
    void msaChanged();                                              //!< Emitted when the Msa instance has changed
    void colorProviderChanged();                                    //!< Emitted when the position msa color provider has changed
    void renderXShiftChanged(qreal xShift);                         //!< Emitted whenever the renderXShift has changed
    //! Emitted whenever the selected Msa region changes from previousSelection to currentSelection (1-based, Msa coordinates)
    void selectionChanged(const PosiRect &currentSelection, const PosiRect &previousSelection);
    void viewportMarginsChanged(const QMargins &margins);
    void viewportMouseLeft();                                       //!< Emitted when the mouse leaves the viewport
    void viewportMouseMoved(QPoint point);                          //!< Emitted when the mouse moves within the viewport
    void zoomChanged(double zoom);                                  //!< Emitted when the zoom has changed to zoom


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void drawAll(QPainter *painter);                        //!< Master method for executing all drawing operations in the appropriate order and context using painter
    virtual void drawBackground(QPainter *painter) const;           //!< Draw the background using painter; empty stub because QWidget auto fills the background
    //! Draw the relevant Msa characters contained in msaRect at origin using painter
    virtual void drawMsa(const QPointF &origin, const PosiRect &msaRect, QPainter *painter);
    //! Draw the area specified by rect as the selection using painter
    virtual void drawSelection(const QRectF &rect, QPainter *painter) const;
    //! Draw the mouse cursor at rect using painter
    virtual void drawMouseCursorPoint(const QRectF &rect, QPainter *painter) const;

    PosiRect msaRegionClip() const;                                 //!< Returns the current Msa region clip
    virtual void scrollContentsBy(int dx, int dy);                  //!< Virtual handler called whenever the scroll bars are moved by dx and dy
    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);    //!< Sets the currently rendered msa region to newMsaRegionClip
    virtual void setViewport(QWidget *widget);                      //!< Trivial stub (calls QAbstractScrollArea method) to prevent external classes from changing the drawing widget

    // Viewport events
    virtual bool event(QEvent *event);                              //!< Top-level event handler for entire QAbstractScrollArea; specifically for responding to ShortcutOverride events
    virtual void keyPressEvent(QKeyEvent *keyEvent);                //!< Reimplemented from QAbstractScrollArea::keyPressEvent()
    virtual void keyReleaseEvent(QKeyEvent *keyEvent);              //!< Reimplemented from QAbstractScrollArea::keyReleaseEvent()
    virtual void mousePressEvent(QMouseEvent *mouseEvent);          //!< Reimplemented from QAbstractScrollArea::mousePressEvent()
    virtual void mouseMoveEvent(QMouseEvent *mouseEvent);           //!< Reimplemented from QAbstractScrollArea::mouseMoveEvent()
    virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);        //!< Reimplemented from QAbstractScrollArea::mouseReleaseEvent()
    virtual void paintEvent(QPaintEvent *paintEvent);               //!< Reimplemented from QAbstractScrollArea::paintEvent()
    virtual void resizeEvent(QResizeEvent *resizeEvent);            //!< Reimplemented from QAbstractScrollArea::resizeEvent()
    virtual bool viewportEvent(QEvent *event);                      //!< Reimplemented from QAbstractScrollArea::viewportEvent()
    virtual void wheelEvent(QWheelEvent *wheelEvent);               //!< Reimplemented from QAbstractScrollArea::wheelEvent()



protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Protected slots

    // Handlers for updating the display as necessary in response to Msa changes
    virtual void onMsaCollapsedLeft(const PosiRect &msaRect);
    virtual void onMsaCollapsedRight(const PosiRect &msaRect);
    virtual void onMsaGapColumnsInserted(const ClosedIntRange &columns);
    virtual void onMsaGapColumnsRemoved(const QVector<ClosedIntRange> &columnRanges);
    virtual void onMsaReset();
    virtual void onMsaRectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange);
    virtual void onMsaRowsInserted(const ClosedIntRange &rows);
    virtual void onMsaRowsMoved(const ClosedIntRange &rows, int finalRow);
    virtual void onMsaRowsRemoved(const ClosedIntRange &rows);
    virtual void onMsaRowsSorted();
    virtual void onMsaRowsSwapped(int first, int second);
    virtual void onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods);

private:
    // ------------------------------------------------------------------------------------------------
    // Private structures
    struct ViewFocusData
    {
        QPointF msaPoint_;
        qreal leftFraction_;
        qreal topFraction_;

        ViewFocusData() : leftFraction_(0), topFraction_(0)
        {}
    };

    // ------------------------------------------------------------------------------------------------
    // Private methods
    QSize canvasSize() const;                                       //!< Returns the width of the drawing canvas; msa length multiplied by the monospace character width and then rounded down to the nearest integer
    QSizeF canvasSizeF() const;                                     //!< Returns the width (floating precision) of the drawing canvas; msa length multiplied by the monospace character width
    QPointF clipRenderOrigin() const;                               //!< Returns the origin for all painting operations with respect to the scroll position
    void restoreViewFocus() const;                                  //!< Sets the view position (by altering the horizontal and vertical scrollbar values) to the previously saved focus point
    //! Saves the current viewport focus of focusPoint relative to the current viewport dimensions; if focusPoint is not within the viewport, the center of the viewport will be used
    void saveViewFocus(const QPointF &focusPoint = QPointF(-1, -1));
    PosiRect visibleSelectionRect() const;                          //!< Returns the msa rectangle that should be drawn on the screen to represent the visible selection or null if none of the selection is visible
    void updateMouseCursorPoint();                                  //!< [Internal] Refreshes the mouse cursor point and emits the mouseCursorMoved if it has changed
    void updateScrollBarRangesAndSteps();                           //!< [Internal] Updates the scroll bar ranges and steps

    // ------------------------------------------------------------------------------------------------
    // Private members
    ObservableMsa *msa_;                                            //!< The underlying Msa object being rendered
    PositionalMsaColorProvider *positionalMsaColorProvider_;        //!< Color provider for providing both foreground and background colors for all msa characters; points to either defaultColorProvider_ or a user-supplied color provider
    QUndoStack *undoStack_;
    PositionalMsaColorProvider defaultColorProvider_;               //!< Default color provider that is used whenever user has not supplied a valid color provider
    PosiRect msaRegionClip_;                                        //!< Clip msa region to be rendered
    QFont font_;
    qreal zoom_;
    PointRectMapper pointRectMapper_;
    ViewFocusData viewFocusData_;                                   //!< View focus

    // Properties
    double zoomFactor_;                                             //!< Amount to zoom per unit change
    double zoomMinimum_;                                            //!< Minimum zoom value
    double zoomMaximum_;                                            //!< Maximum zoom value

    // Selection related data
    QPoint msaSelectionStart_;
    QPoint msaSelectionStop_;

    // Key watchers
    bool keyControlPressed_;

    // Active and temporary tools
    IMsaTool *currentMsaTool_;
    IMsaTool *previousTool_;
    bool currentToolIsTemporary_;
    int temporaryToolKey_;
    bool temporaryToolKeyPressed_;

    // Miscellaneous variables
    QPoint mouseHotSpotOffset_;                 // Relative amount to add to current mouse position to obtain its hotspot
    qreal renderXShift_;
    bool showMouseCursorPoint_;
    QPoint mouseCursorPoint_;


    // Margin variables
    QMargins viewportMargins_;
    MsaStartStopSideWidget *msaStartSideWidget_;
    MsaStartStopSideWidget *msaStopSideWidget_;
    MsaRulerWidget *msaRulerWidget_;
    VerticalMsaMarginWidget *verticalMsaMarginWidget_;

    // --------------------------
    // Friend classes
    friend class PointRectMapper;

#ifdef TESTING
    friend class TestAbstractMsaView;
#endif
};

#endif // ABSTRACTMSAVIEW_H


/**
  *
void AbstractMsaView::clampMouseActivePointToMsaBounds()
{
    ASSERT(msa_ != nullptr);
    ASSERT(msa_->columnCount() > 0 && msa_->rowCount() > 0);

    if (mouseCursorPoint_.x() < 0)
        mouseCursorPoint_.setX(1);
    else if (mouseCursorPoint_.x() > msa_->columnCount())
        mouseCursorPoint_.setX(msa_->columnCount());

    if (mouseCursorPoint_.y() < 0)
        mouseCursorPoint_.setY(1);
    else if (mouseCursorPoint_.y() > msa_->rowCount())
        mouseCursorPoint_.setY(msa_->rowCount());
}

  */
