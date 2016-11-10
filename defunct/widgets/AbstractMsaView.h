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
#include <QtCore/QTimer>

#include <QtGui/QAbstractScrollArea>
#include <QtGui/QPixmap>

#include "PositionalMsaColorProvider.h"
#include "TextPixmapRenderer.h"

#include "util/Rect.h"
#include "util/MsaRect.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;

// Given these specific type names for clarity sake when working with the code
// typedef QPoint MsaLocus;        // 1-based coordinate corresponding to a specific row/column within a Msa
// typedef QRect MsaRegion;        // Two MsaLoci that define the bounds of a specific rectangular region within a Msa

class PointRectMapperPrivate;
class AbstractTextRenderer;

/**
  * MsaView is a robust graphical control for viewing and manipulating Msas.
  *
  * Depending on the font and zoom, most Msa will not usually fit within the typical user's screen resolution. Thus, a
  * primary goal is to make it possible to easily pan and zoom a subsection of the Msa in a similar style as Adobe
  * Acrobat does with their Reader program. Similarly, it is desirable to
  *
  * On a similar note, it is vital to be able to edit a Msa using the mouse for selection and navigation. Because the
  * natural method for pan and making a selection (mouse down, mouse drag, mouse up) are identical, it is necessary to
  * provide a mechanism for switching between pan and edit modes. This is accomplished by setting the view to a valid
  * operating modes (@see OperatingMode) with the method, setMode(). Oftentimes, a selection will need to be made that
  * extends beyond the currently viewable area. To account for this, when the user drags their mouse beyond the view's
  * edges, the view automatically scrolls (until they release their mouse). Zooming may be done in any mode via the
  * mouse wheel or alternatively, user's may enter the zoom mode which has additional methods for handling zoom
  * requests.
  *
  * The actual colors used for display are abstracted out via a PositionalColorProvider, which by default is a basic
  * PositionColorProvider which simply returns black text on a white background. If a null PositionalColorProvider is
  * supplied, the default one is used.
  *
  * EditCursor is edit action anchor point - displayed as selection of single character; inverted within selection;
  *            always visible; current keyboard location
  * MouseCursor simply shows character the mouse is currently over; displayed as outlined box; inverted when within
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
  * handler defined for this widget. This occurs because QAbstractScrolLArea takes ownership of the widget and installs
  * an event handler to intercept several of the most common events including the paintEvent.
  *
  * The goal with MsaView is to have this base class provide both an interface for drawing all the components of a Msa
  * and additionally provide a software rendering implementation. Moreover, other classes should be able to derive from
  * MsaView and provide accelerated renderer implementation of the interface methods (e.g. using OpenGL). To achieve
  * this goal, MsaView::setViewport() is protected and thus will always use the default QWidget. Derived classes may
  * call this method with a QWidget derivative (e.g. QGLWidget) of their choice.
  *
  * The current selection is dynamically updated in response to user events. For example, if the user inserts a gap
  * column in the middle of a selection, the selection should increase horizontally by the number of gaps inserted.
  * Conversely, if the selection contains gap columns that are removed, the selection should be reduced by that amount.
  * If the selection is completely contained within the gap region, it should be reduced to the edit cursor and the
  * edit cursor moved to the top left most character beside the selection before it was removed. The same concepts apply
  * when sequences are inserted/removed from the Msa. If the sequences are sorted, then the selection should remain
  * unchanged - although it will no longer be relevant.
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

    Q_PROPERTY(double zoomFactor
               READ zoomFactor
               WRITE setZoomFactor);
    Q_PROPERTY(double zoomMinimum
               READ zoomMinimum
               WRITE setZoomMinimum);
    Q_PROPERTY(double zoomMaximum
               READ zoomMaximum
               WRITE setZoomMaximum);

public:
    // ------------------------------------------------------------------------------------------------
    // Enumerated types
    //! All supported viewing modes
    enum OperatingMode
    {
        PanMode,            //!< Pan the clip/view window
        ZoomMode,           //!< Scale the windows contents
        EditMode            //!< All editing is done from the edit mode screen
    };

    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit AbstractMsaView(QWidget *parent = 0);                  //!< Construct and initialize instance
    virtual ~AbstractMsaView();                                     //!< Virtual destructor for supporting inheritance

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QRectF clipRect() const;                                        //!< Returns the currently visible region of the alignment
    QPoint editCursorPoint() const;                                 //!< Returns the current edit cursor point in Msa coordinates or a null QPoint if msa is not defined or is empty
    QFont font() const;                                             //!< Returns the rendering font
    OperatingMode mode() const;                                     //!< Returns the operating mode
    const Msa *msa() const;                                         //!< Returns the Msa being viewed
    //!< Returns the active color provider
    const PositionalMsaColorProvider *positionalMsaColorProvider() const;
    Rect msaSelectionRect() const;                                  //!< Returns the Msa coordinates for the current selection
    QPoint scrollPosition() const;                                  //!< Convenience method that returns the horizontal and vertical scroll bar positions as a QPoint
    //! Sets the PositionalMsaColorProvider to positionalMsaColorProvider or the default PositionalMsaColorProvider if positionalMsaColorProvider is zero
    void setPositionalMsaColorProvider(PositionalMsaColorProvider *positionalMsaColorProvider);
    qreal zoom() const;                                             //!< Returns the zoom
    double zoomFactor() const;                                      //!< Returns the zoom factor
    double zoomMinimum() const;                                     //!< Returns the minimum zoom value
    double zoomMaximum() const;                                     //!< Returns the maximum zoom value
    MsaRect selection() const;

public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    //! Renders the current visualization of msaRegion to a SVG image file name fileName; TEMPORARY!! Will eventually move to external class!
    void saveSvg(const QString &fileName, const Rect &msaRegion) const;
    void setFont(const QFont &font);                                //!< Sets the rendering font to font (must be a monospace compatible) and returns true on success or false otherwise
    void setMode(OperatingMode mode);                               //!< Sets the operating mode to mode
    void setMsa(Msa *msa);                                          //!< Sets the msa being visualized to msa
    void setZoom(double zoom);                                      //!< Sets the zoom to zoom
    void setZoom(double zoom, const QPointF &focus);
    //!< Zoom by zoomFactor_ amount times relative to focus; if either the focus x or y is negative or the focus is outside the viewport range, then relative to center of view
    void setZoomBy(int amount, const QPointF focus = QPointF(-1, -1));
    void setZoomFactor(double zoomFactor);                          //!< Sets the zoom factor to zoomFactor; does nothing if zoomFactor is less than or equal to zero
    void setZoomMinimum(double zoomMinimum);                        //!< Sets the miminum zoom value to zoomMinimum; zoomMinimum must be greater than zero. If zoomMinimum exceeds zoomMaximum, zooMaximum is set to zoomMinimum
    void setZoomMaximum(double zoomMaximum);                        //!< Sets the maximum zoom value to zoomMaximum; zoomMaximum must be greater than zoomMinimum

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void editCursorMoved(QPoint point, QPoint previousPoint);       //!< Emitted whenever the edit cursor has moved positions from previousPoint to point (1-based, Msa coordinates)
    void fontChanged();                                             //!< Emitted whenever the font has changed
    void modeChanged();                                             //!< Emitted when the OperatingMode has changed
    void mouseCursorMoved(QPoint point, QPoint previousPoint);      //!< Emitted whenever the mouse cursor has moved positions from previousPoint to point (1-based, Msa coordinates)
    void msaChanged();                                              //!< Emitted when the Msa instance has changed
    void selectionChanged(QRect selection, QRect previousSelection);//!< Emitted whenever the selected Msa region changes from previousSelection to selection (1-based, Msa coordinates)
    void zoomChanged(double zoom);                                  //!< Emitted when the zoom has changed to zoom

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual AbstractTextRenderer *abstractTextRenderer() const = 0; //!< Return the current text renderer
    virtual void drawAll(QPainter *painter);                        //!< Master method for executing all drawing operations in the appropriate order and context using painter
    virtual void drawBackground(QPainter *painter) const = 0;       //!< Draw the background using painter; empty stub because QWidget auto fills the background
    //! Draw the relevant Msa characters contained in msaRect at origin using painter
    virtual void drawMsa(const QPointF &origin, const MsaRect &msaRect, QPainter *painter) = 0;
    //! Draw the area specified by rect as the selection using painter
    virtual void drawSelection(const QRectF &rect, QPainter *painter) const = 0;
    //! Draw the edit cursor at rect using painter
    virtual void drawEditCursor(const QRectF &rect, QPainter *painter) const = 0;
    //! Draw the mouse cursor at rect using painter
    virtual void drawMouseActivePoint(const QRectF &rect, QPainter *painter) const = 0;
    //! Draw the gap insertion line at x using painter
    virtual void drawGapInsertionLine(qreal x, QPainter *painter) const = 0;

    qreal charHeight() const;                                       //!< Convenience function that returns the height of an individual character block
    qreal charWidth() const;                                        //!< Convenience function that returns the width of an individual character block
    bool isMouseOverSelection() const;                              //!< Convenience function that returns true if the mouse is over the selection; false otherwise
    Rect msaRegionClip() const;                                     //!< Returns the current Msa region clip
    const PointRectMapperPrivate *pointRectMapper() const;          //!< Returns the point rect mapper instance
    //! Convenience method for rendering msaRegion at origin using textRenderer and painter
    void renderMsaRegion(const QPointF &origin, const MsaRect &msaRegion, const AbstractTextRenderer *abstractTextRenderer, QPainter &painter) const;
    virtual void scrollContentsBy(int dx, int dy);                  //!< Virtual handler called whenever the scroll bars are moved by dx and dy
    virtual void setMsaRegionClip(const Rect &newMsaRegionClip);    //!< Sets the currently rendered msa region to newMsaRegionClip
    virtual void setViewport(QWidget *widget);                      //!< Trivial stub (calls QAbstractScrollArea method) to prevent external classes from changing the drawing widget

    // Viewport events
    virtual void keyPressEvent(QKeyEvent *keyEvent);                //!< Reimplemented from QAbstractScrollArea::keyPressEvent()
    virtual void keyReleaseEvent(QKeyEvent *keyEvent);              //!< Reimplemented from QAbstractScrollArea::keyReleaseEvent()
    virtual bool mouseEnterEvent();                                 //!< Custom mouse enter event
    virtual bool mouseLeaveEvent();                                 //!< Custom mouse leave event
    virtual void mousePressEvent(QMouseEvent *mouseEvent);          //!< Reimplemented from QAbstractScrollArea::mousePressEvent()
    virtual void mouseMoveEvent(QMouseEvent *mouseEvent);           //!< Reimplemented from QAbstractScrollArea::mouseMoveEvent()
    virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);        //!< Reimplemented from QAbstractScrollArea::mouseReleaseEvent()
    virtual void paintEvent(QPaintEvent *paintEvent);               //!< Reimplemented from QAbstractScrollArea::paintEvent()
    virtual void resizeEvent(QResizeEvent *resizeEvent);            //!< Reimplemented from QAbstractScrollArea::resizeEvent()
    virtual bool viewportEvent(QEvent *event);                      //!< Reimplemented from QAbstractScrollArea::viewportEvent()
    virtual void wheelEvent(QWheelEvent *wheelEvent);               //!< Reimplemented from QAbstractScrollArea::wheelEvent()

    // ------------------------------------------------------------------------------------------------
    // Protected members
    PositionalMsaColorProvider *positionalMsaColorProvider_;        //!< Color provider for providing both foreground and background colors for all msa characters; points to either defaultColorProvider_ or a user-supplied color provider
    Msa *msa_;                                                      //!< The underlying Msa object being rendered
    MsaRect msaRegionClip_;                                         //!< Clip msa region to be rendered

protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots

    // Handlers for updating the display as necessary in response to Msa changes
    void onMsaCollapsedLeft(const MsaRect &msaRect, int rightMostModifiedColumn);
    void onMsaCollapsedRight(const MsaRect &msaRect, int leftMostModifiedColumn);
    void onMsaExtendOrTrimFinished(int start, int end);
    void onMsaGapColumnsInserted(int column, int count);
    void onMsaGapColumnsRemoved(int count);
    void onMsaRegionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight);
    void onMsaReset();
    void onMsaSubseqSwapped(int first, int second);
    void onMsaSubseqsInserted(int start, int end);
    void onMsaSubseqsMoved(int start, int end, int target);
    void onMsaSubseqsRemoved(int start, int end);
    void onMsaSubseqsSorted();

    void onMsaSelectionScrollTimeout();

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
    MsaRect visibleSelectionRect() const;                           //!< Returns the msa rectangle that should be drawn on the screen to represent the visible selection or null if none of the selection is visible

    void setDefaultEditCursorPoint();                               //!< [Internal] Sets the default edit cursor position (1, 1)
    void setDefaultSelection();                                     //!< [Internal] Sets the default msa selection rectangle
    void updateScrollBarRangesAndSteps();                           //!< [Internal] Updates the scroll bar ranges and steps
    void updateMouseCursor();                                       //!< [Internal] Sets the mouse cursor to match the current state
    int gapInsertionColumn() const;                                 //!< [Internal] Returns the position in msa space where a gap will be inserted based on the current mouse position
    void selectAll();                                               //!< [Internal] Selects the entire alignment

    void setSelectionStartFromViewPoint(const QPointF &viewPoint);  //!< [Internal] Sets the start selection point relative to viewPoint
    void setSelectionStopFromViewPoint(const QPointF &viewPoint);   //!< [Internal] Sets the stop selection point relative to viewPoint; starts the scroll timer if necessary

    // ------------------------------------------------------------------------------------------------
    // Private members
    OperatingMode operatingMode_;                                   //!< Current operating mode
    PositionalMsaColorProvider defaultColorProvider_;               //!< Default color provider that is used whenever user has not supplied a valid color provider
    QFont font_;
    qreal zoom_;
    double zoomFactor_;                                             //!< Amount to zoom per unit change
    double zoomMinimum_;                                            //!< Minimum zoom value
    double zoomMaximum_;                                            //!< Maximum zoom value
    QScopedPointer<PointRectMapperPrivate> pointRectMapper_;

    Rect msaSelectionRect_;                                         //!< Msa coordinates of those characters in the current selection
    QPoint editCursorPoint_;                                        //!< Msa coordinate of the edit cursor
    QPoint mouseCursorPoint_;                                       //!< Msa coordinate of the mouse cursor; if mouse button is pressed, this is the msa character that would begin the selection
    ViewFocusData viewFocusData_;                                   //!< View focus


    QPoint lastPanScrollPosition_;
    QPoint panAnchorPoint_;
    bool panIsActive_;

    bool keyAltPressed_;
    bool keyControlPressed_;
    bool keyShiftPressed_;
    bool keyEPressed_;
    bool keyTPressed_;
    bool keyRPressed_;

    // Selection related data
    bool selectionIsActive_;
    QPoint msaSelectionAnchorPoint_;
    QPoint msaSelectionStart_;
    QPoint msaSelectionStop_;
    QTimer msaSelectionScrollTimer_;

    // Slide related data
    bool slideIsActive_;
    QPoint slideMsaAnchorPoint_;

    // Gap insertion
    bool gapInsertionIsActive_;

    // Temporarily switch to pan mode
    bool temporaryPan_;
    bool temporaryPanIsActive_;

    friend class PointRectMapperPrivate;

#ifdef TESTING
    friend class TestAbstractMsaView;
    friend class TestPointRectMapperPrivate;
#endif
};

// Special conditions for testing the PointRectMapperPrivate methods
#ifdef TESTING
#ifndef POINTRECTMAPPERPRIVATE_H
#define POINTRECTMAPPERPRIVATE_H
#include "PointRectMapperPrivate.h"
#endif
#endif

#endif // ABSTRACTMSAVIEW_H





/**
  * Unused artifactual junk
  */
/**
  * MsaPixmapCache simply associates a list of pixmaps with a particular msa region (1-based) and is strictly
  * intended for optimizing the software rendering of a Msa.
  */
//    struct MsaPixmapCache
//    {
//        QList<QPixmap> pixmaps_;                                    //!< List of rendered pixmaps corresponding to the Msa region_
//        QRect region_;                                              //!< Msa region (1-based) for which a pixmap has been rendered and contained in pixmaps_
//    };

//    MsaPixmapCache msaPixmapCache_;                                 //!< Local cache of rendered sequences

//    QPointF mapMsaLocusToPoint(const MsaLocus &msaLocus);
//    MsaLocus mapPointToMsaLocus(const QPointF &point);
//    QRectF mapMsaRegionToRect(const MsaRegion &msaRegion);
//    MsaRegion mapRectToMsaRegion(const QRectF &rect);

// //!< Sets the clip rectangle to position relative to boxPoint
//    void setClipPosition(const QPoint &position, BoxPoint boxPoint);
// //!< Emitted when the clip position has changed to position from previousPosition; always in pixel units
//    void clipPositionChanged(QPoint position, QPoint previousPosition);
