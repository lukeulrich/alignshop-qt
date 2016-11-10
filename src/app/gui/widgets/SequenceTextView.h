/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef SEQUENCETEXTVIEW_H
#define SEQUENCETEXTVIEW_H

#include <QtCore/QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QString>

#include <QtGui/QAbstractScrollArea>

class ClosedIntRange;
/// The class responsible for painting the sequence data.
class SequenceViewPainter;

/// The class representing a text selection.
class Selection
{
public:
    /// The selection start.
    int start_;

    /// The selection stop.
    int stop_;

    /// The selection color.
    QColor color_;

    /// The selection background color.
    QColor backColor_;

    /// The constructor.
    Selection();

    Selection(const ClosedIntRange &range);

    /// Determines whether or not the specified selection intersects with the
    /// this selection.
    /// @param selection const Selection &
    /// @return bool
    bool intersects(const Selection &selection) const;
};

/// Enumerates the directions a selection can take.
enum SelectionDirection
{
    /// The selection direction is unknown.
    SelectionDirection_Unknown,

    /// The selection is left of the starting position.
    SelectionDirection_Left,

    /// The selection is right of the starting position.
    SelectionDirection_Right
};

/// The sequence text view class represents a sequence visually.
class SequenceTextView : public QAbstractScrollArea
{
    Q_OBJECT

    friend class SequenceViewPainter;

public:
    /// The size of a column.
    static const int COL_SIZE;

    /// The size of the padding at the top of the control.
    static const int PADDING_TOP;

    /// The general padding size, used to compute the space beteween visual
    /// elements.
    static const int PADDING;

    /// The speed (in ms) of a tripple click.
    static const int TRIPLE_CLICK_SPEED;

    /// The constructor.
    /// @param parent QWidget * (Defaults to 0.)
    explicit SequenceTextView(QWidget *parent = 0);

    /// The destructor.
    ~SequenceTextView();

    /// Adds a selection.
    /// @param selection const Selection &
    /// @return void
    void addSelection(const Selection &selection);

    /// Adds a selection.
    /// @param start int
    /// @param stop int
    /// @return void
    void addSelection(int start, int stop);
    void addSelection(const ClosedIntRange &range);

    /// Gets whether user selection is allowed.
    /// @return bool
    bool allowSelect() const;

    /// Clears all selections.
    /// @return void
    void clearSelections();

    /// Gets the current selection (or null if none exists).
    /// @return Selection *
    Selection *currentSelection();

    /// Gets the sequence index at the specified point.
    /// @param point const QPoint &
    /// @return int
    int indexAt(const QPoint &point) const;

    /// Gets whether or not the scrollbars are visible.
    /// @return bool
    bool isScrollable() const;

    /// Selects all the text.
    /// @return void
    void selectAll();

    /// Selects the block (column in a row) at the specified index.
    /// @param index int
    /// @return void
    void selectBlockContainingIndex(int index);

    /// Sets whether or not selection is allowed.
    /// @param value bool
    /// @return void
    void setAllowSelect(bool value);

    /// Sets the sequence displayed by this control.
    /// @param sequence const QString &
    /// @return void
    void setSequence(const QString &sequence);

    /// Gets the sequence displayed by this control.
    /// @return QString
    QString sequence() const;

public Q_SLOTS:
    /// Copies the current selection to the clipboard.
    /// @return void
    void copyToClipboard();

Q_SIGNALS:
    /// Triggered when the current selection changes.
    /// @return void
    void selectionChanged();
    void selectionFinished();

private:
    QElapsedTimer timeSinceDoubleClick_;
    QList<Selection> selections_;
    int left_;
    int right_;
    int charWidth_;
    int colPixelWidth_;
    int colPadding_;
    int indexColWidth_;
    int lineHeight_;
    int leftMargin_;
    int rightMargin_;
    int textAreaWidth_;
    int mouseDownIndex_;
    int paddedColPixelWidth_;
    QPoint mouseDownPoint_;
    QString sequence_;
    bool allowSelect_;
    QAction *actionCopy_;

    /// The current selection changed.
    /// @return void
    void onSelectionChanged();

    /// Adjusts the drawing region values if necessary.
    /// @return void
    void adjustComputedDrawingRegions();

    /// Scrolls the specified point into view.
    /// @param point const QPoint &
    /// @return void
    void adjustScrollbarsToPoint(const QPoint &point);

    /// Adjusts the current selection to the specified index.
    /// @param index int
    /// @return void
    void adjustSelectionToIndex(int index);

    /// Begins selecting.
    /// @param e const QMouseEvent *
    /// @return void
    void beginSelecting(const QMouseEvent *e);

    /// Begins tripple clicking.
    /// @return void
    void beginTrippleClick();

    /// Computes the number of chars/rows above the specified y coordinate.
    /// @param y int
    /// @return int
    int charsAbove(int y) const;

    /// Computes the number of chars left of the specified x coordinate in a single row.
    /// @param x int
    /// @return int
    int charsLeftOf(int x) const;

    /// Computes the number of columns per row.
    /// @return int
    int colsPerLine() const;

    /// Computes the drawing region values.
    /// @return void
    void computeDrawingRegions();

    /// Ensures that a selection exists, and if one doesn't, one is
    /// started at the specified index.
    /// @param index int
    /// @return bool
    bool ensureSelectionExists(int index);

    /// Determines the index at the specified point, given the specified
    /// direction.
    /// @param point const QPoint &
    /// @param direction SelectionDirection
    /// @return int
    int indexAt(const QPoint &point, SelectionDirection direction) const;

    /// Inserts the selection in order of start position.
    /// @param selection const Selection &
    /// @return void
    void insertInOrder(const Selection &selection);

    /// Determines whether or not the specified x coordinate falls inbetween
    /// columns.
    /// @param x int
    /// @return bool
    bool isInColumnPadding(int x) const;

    /// Determines whether or not the specified mouse button is down.
    /// @param button Qt::MouseButton
    /// @param e const QMouseEvent *
    /// @return bool
    bool isPressed(Qt::MouseButton button, const QMouseEvent *e) const;

    /// Determines whether or not the current click is part of a tripple-click.
    /// @return bool
    bool isTrippleClick() const;

    /// Triggered when a key is pressed.
    /// @param e QKeyEvent *
    /// @return void
    void keyPressEvent(QKeyEvent *e);

    /// Triggered when the mouse is pressed.
    /// @param e QMouseEvent *
    /// @return void
    void mousePressEvent(QMouseEvent *e);

    /// Triggered when the mouse is moving.
    /// @param e QMouseEvent *
    /// @return void
    void mouseMoveEvent(QMouseEvent *e);

    void mouseReleaseEvent(QMouseEvent *e);

    /// Triggered when the mouse is doubleclicked.
    /// @param e QMouseEvent *
    /// @return void
    void mouseDoubleClickEvent(QMouseEvent *e);

    /// Triggered when the control is painting.
    /// @param event QPaintEvent *
    /// @return void
    void paintEvent(QPaintEvent *event);

    /// Triggered when the control resizes.
    /// @param e QResizeEvent *
    /// @return void
    void resizeEvent(QResizeEvent *e);

    /// Sanitizes the selection to make sure it is a valid range.
    /// @param selection const Selection &
    /// @return Selection
    Selection sanitizeSelection(const Selection &selection) const;

    /// Converts from relative to absolute coordinate.
    /// @param point const QPoint &
    /// @return QPoint
    QPoint toAbsoluteCoordinate(const QPoint &point) const;
};

#endif // SEQUENCETEXTVIEW_H
