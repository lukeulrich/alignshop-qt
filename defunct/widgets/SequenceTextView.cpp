#include <widgets/SequenceTextView.h>

using namespace Widgets;

namespace Widgets
{
    /// An in-memory cache of bitmaps representing chars.
    class PixMapCache
    {
    public:
        PixMapCache(const QFont &font)
        {
            QFontMetrics metric(font);
            font_ = font;
            charWidth_ = metric.width('A');
            charHeight_ = metric.height();
        }

        QPixmap getCharImage(const QChar &ch, const QColor &backColor, const QColor &color)
        {
            QString key = toKey(ch, backColor);

            if (cache_.contains(key))
            {
                return cache_[key];
            }

            QPixmap img = toPixMap(ch, backColor, color);
            cache_[key] = img;
            return img;
        }

    private:
        QHash<QString, QPixmap> cache_;
        QFont font_;
        int charWidth_;
        int charHeight_;

        QString toKey(const QChar &ch, const QColor &backColor)
        {
            return ch + QString::number(backColor.red() + (backColor.blue() << 8) + (backColor.green() << 16));
        }

        QPixmap toPixMap(const QChar &ch, const QColor &backColor, const QColor &color)
        {
            QPixmap img(charWidth_, charHeight_);
            QPainter charPaint(&img);
            charPaint.setFont(font_);
            charPaint.setPen(color);
            charPaint.fillRect(0, 0, charWidth_, charHeight_, backColor);
            charPaint.drawText(0, 0, charWidth_, charHeight_, Qt::AlignLeft, ch);
            return img;
        }
    };

    /// The sequence view paint logic.
    class SequenceViewPainter
    {
    public:
        SequenceViewPainter(SequenceTextView *view, QPainter *painter)
        {
            view_ = view;
            painter_ = painter;
            cache_ = new PixMapCache(view->font());
            indexColBackColor_ = QColor("#EFEFEF");
            indexColForeColor_ = QColor("#777");
            nextSelectionStart_ = -1;
            nextSelectionIndex_ = 0;
            x_ = view_->leftMargin_;
            y_ = SequenceTextView::PADDING_TOP;
            charsPerLine_ = view->colsPerLine() * SequenceTextView::COL_SIZE;

            initializeCurrentIndex();
            initializeSelectionStack();
        }

        ~SequenceViewPainter()
        {
            delete cache_;
        }

        void paint()
        {
            paintLeftIndexBackground();
            paintTextAreaBackground();
            paintRightIndexBackground();
            paintBorder();

            while (morePaintingRemains())
            {
                paintLine();
            }
        }

    private:
        PixMapCache *cache_;
        QColor indexColBackColor_;
        QColor indexColForeColor_;
        SequenceTextView *view_;
        QPainter *painter_;
        QStack<Selection> selectionStack_;
        Selection *currentSelection_;
        int charsPerLine_;
        int currentIndex_;
        int nextSelectionIndex_;
        int nextSelectionStart_;
        int x_;
        int y_;

        void addBackgroundSelection()
        {
            Selection backgroundSelection;
            backgroundSelection.start = 0;
            backgroundSelection.stop = view_->sequence_.length();
            backgroundSelection.backColor = Qt::white;
            backgroundSelection.color = Qt::black;

            selectionStack_.push(backgroundSelection);
        }

        bool atEndOfLine()
        {
            return currentIndex_ && !(currentIndex_ % charsPerLine_);
        }

        void computeIfPastCurrentSelection()
        {
            if (currentIndex_ > currentSelection_->stop)
            {
                selectionStack_.pop();
                while (selectionStack_.top().stop < currentIndex_)
                {
                    selectionStack_.pop();
                }

                cacheCurrentSelection();
            }
        }

        void computeIfInNextSelection()
        {
            QList<Selection> &selections = view_->selections_;
            if (currentIndex_ == nextSelectionStart_)
            {
                selectionStack_.push(selections[nextSelectionIndex_]);
                nextSelectionStart_ = -1;
                while (++nextSelectionIndex_ < selections.length())
                {
                    Selection &nextSelection = selections[nextSelectionIndex_];
                    if (currentIndex_ == nextSelection.start)
                    {
                        selectionStack_.push(nextSelection);
                    }
                    else
                    {
                        nextSelectionStart_ = nextSelection.start;
                        break;
                    }
                }

                cacheCurrentSelection();
            }
        }

        void initializeCurrentIndex()
        {
            currentIndex_ = view_->indexAt(QPoint(view_->leftMargin_, 0));
        }

        void initializeSelectionStack()
        {
            addBackgroundSelection();

            QList<Selection> &selections = view_->selections_;

            for (nextSelectionIndex_ = 0; nextSelectionIndex_ < selections.count(); ++nextSelectionIndex_)
            {
                if (selections[nextSelectionIndex_].start > currentIndex_)
                {
                    nextSelectionStart_ = selections[nextSelectionIndex_].start;
                    break;
                }

                if (selections[nextSelectionIndex_].stop >= currentIndex_)
                {
                    selectionStack_.push(selections[nextSelectionIndex_]);
                }
            }

            cacheCurrentSelection();
        }

        bool isInColumnPadding() const
        {
            return currentIndex_ &&
                !(currentIndex_ % SequenceTextView::COL_SIZE) &&
                (x_ > view_->leftMargin_ + view_->colPadding_);
        }

        bool morePaintingRemains() const
        {
            // TODO: Return false if we are past the clip region.
            return (currentIndex_ < view_->sequence_.length() && y_ < view_->height());
        }

        bool moveToNextChar()
        {
            return ++currentIndex_ < view_->sequence_.length();
        }

        void paintBorder()
        {
            painter_->setPen(QColor("#E3E9EF"));
            painter_->drawRect(0, 0, view_->width() - 1, view_->height() - 1);
            painter_->setPen(Qt::black);
            QPainterPath path = painter_->clipPath();
            path.addRect(1, 1, view_->width() - 2, view_->height() - 2);
            painter_->setClipPath(path);
        }

        void paintColumnPadding()
        {
            painter_->fillRect(x_, y_ , view_->colPadding_, view_->lineHeight_, currentSelection_->backColor);
            x_ += view_->colPadding_;
        }

        void paintCurrent()
        {
            painter_->drawPixmap(
                x_,
                y_,
                cache_->getCharImage(view_->sequence_[currentIndex_],
                currentSelection_->backColor,
                currentSelection_->color));
            x_ += view_->charWidth_;
        }

        void paintLeftIndex()
        {
            painter_->setPen(indexColForeColor_);

            painter_->drawText(
                0,
                y_,
                view_->indexColWidth_ - (SequenceTextView::PADDING / 2),
                view_->lineHeight_,
                Qt::AlignRight,
                QString::number(currentIndex_ + 1));

            painter_->setPen(currentSelection_->color);
        }

        void paintLeftIndexBackground()
        {
            painter_->fillRect(view_->left_, 0, view_->indexColWidth_, view_->height(), indexColBackColor_);
        }

        void paintLine()
        {
            paintLeftIndex();

            do
            {
                computeIfPastCurrentSelection();

                if (isInColumnPadding())
                {
                    paintColumnPadding();
                }

                computeIfInNextSelection();

                paintCurrent();
            } while (moveToNextChar() && !atEndOfLine());

            paintRightIndex();

            x_ = view_->leftMargin_;
            y_ += view_->lineHeight_;
        }

        void paintRightIndex()
        {
            painter_->setPen(indexColForeColor_);

            painter_->drawText(
                view_->rightMargin_ + SequenceTextView::PADDING + (SequenceTextView::PADDING / 2),
                y_,
                view_->indexColWidth_ - (SequenceTextView::PADDING / 2),
                view_->lineHeight_,
                Qt::AlignLeft,
                QString::number(currentIndex_));

            painter_->setPen(currentSelection_->color);
        }

        void paintRightIndexBackground()
        {
            painter_->fillRect(
                view_->right_ - view_->indexColWidth_, 0, view_->indexColWidth_, view_->height(), indexColBackColor_);
        }

        void paintTextAreaBackground()
        {
            painter_->fillRect(view_->leftMargin_, 0, view_->textAreaWidth_, view_->height(), Qt::white);
        }

        void cacheCurrentSelection()
        {
            currentSelection_ = &selectionStack_.top();
            painter_->setPen(currentSelection_->color);
        }
    };
}

const int SequenceTextView::COL_SIZE = 10;
const int SequenceTextView::PADDING_TOP = 0;
const int SequenceTextView::PADDING = 10;
const int SequenceTextView::TRIPLE_CLICK_SPEED = 300;

Selection::Selection()
{
    color = Qt::black;
    backColor = QColor("#A8CDF1");
}

bool Selection::intersects(const Selection &selection) const
{
    return start < selection.stop && stop > selection.start;
}

SequenceTextView::SequenceTextView(QWidget *parent) :
    QAbstractScrollArea(parent)
{
    setAllowSelect(true);

    actionCopy_ = new QAction(tr("Copy"), this);
    addAction(actionCopy_);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    QObject::connect(actionCopy_, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
}

SequenceTextView::~SequenceTextView()
{
    delete actionCopy_;
}

void SequenceTextView::addSelection(int start, int stop)
{
    Selection sel;
    sel.start = start;
    sel.stop = stop;
    addSelection(sel);
}

void SequenceTextView::addSelection(const Selection &selection)
{
    Selection sanitizedSelection = sanitizeSelection(selection);
    insertInOrder(sanitizedSelection);
    viewport()->update();
    onSelectionChanged();
}

void SequenceTextView::adjustComputedDrawingRegions()
{
    bool scrollWasVisible = isScrollable();
    computeDrawingRegions();
    bool scrollVisibilityChanged = scrollWasVisible != isScrollable();
    if (scrollVisibilityChanged)
    {
        computeDrawingRegions();
    }
}

void SequenceTextView::adjustScrollbarsToPoint(const QPoint &point)
{
    int y = point.y();
    if (y < 0)
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + y);
    }
    else if (y > height())
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + (y - height()));
    }
}

void SequenceTextView::adjustSelectionToIndex(int index)
{
    Selection *current = currentSelection();
    current->start = qMax(0, qMin(index, mouseDownIndex_));
    current->stop = qMin(sequence_.length() - 1, qMax(index, mouseDownIndex_));

    onSelectionChanged();
}

bool SequenceTextView::allowSelect() const
{
    return allowSelect_;
}

void SequenceTextView::beginSelecting(const QMouseEvent *e)
{
    clearSelections();
    mouseDownIndex_ = indexAt(e->pos());
    mouseDownPoint_ = e->pos();
}

void SequenceTextView::beginTrippleClick()
{
    timeSinceDoubleClick_.start();
}

int SequenceTextView::charsAbove(int y) const
{
    int linesAboveY = (y - PADDING_TOP) / lineHeight_;

    int charsPerLine = colsPerLine() * COL_SIZE;

    return charsPerLine * linesAboveY;
}

int SequenceTextView::charsLeftOf(int x) const
{
    int columnsLeftOfX = x / paddedColPixelWidth_;
    int positionInCol = x - (columnsLeftOfX * paddedColPixelWidth_);

    int charsLeftOfX = columnsLeftOfX * COL_SIZE;

    if (positionInCol >= 0)
    {
        int charsInColUpToX = qMin(COL_SIZE, positionInCol / charWidth_);
        charsLeftOfX += charsInColUpToX;
    }

    return charsLeftOfX;
}

void SequenceTextView::clearSelections()
{
    selections_.clear();
    viewport()->update();
    onSelectionChanged();
}

void SequenceTextView::copyToClipboard()
{
    if (selections_.count())
    {
        Selection &current = selections_.first();
        QApplication::clipboard()->setText(sequence_.mid(current.start, (current.stop - current.start) + 1));
    }
}

Selection *SequenceTextView::currentSelection()
{
    if (selections_.count())
    {
        return &selections_.first();
    }

    return 0;
}

int SequenceTextView::colsPerLine() const
{
    int value = textAreaWidth_ / paddedColPixelWidth_;

    if ((value * paddedColPixelWidth_) + colPixelWidth_ < textAreaWidth_)
    {
        value += 1;
    }

    return value;
}

void SequenceTextView::computeDrawingRegions()
{
    const int NUM_INDEX_COLS = 2;

    QFont myFont = font();
    QFontMetrics fm(myFont);
    QString maxIndex = QString::number(sequence_.length());

    int scrollWidth = isScrollable() ? verticalScrollBar()->width() : 0;

    charWidth_ = fm.width('A');
    colPadding_ = (charWidth_ / 2) + charWidth_;
    colPixelWidth_ = charWidth_ * COL_SIZE;
    paddedColPixelWidth_ = colPixelWidth_ + colPadding_;

    indexColWidth_ = (maxIndex.length() * charWidth_) + PADDING;
    lineHeight_ = fm.height();

    int paddedIndexColPixelWidth = indexColWidth_ + PADDING;

    left_ = 0;
    right_ = qMax(width() - scrollWidth, (paddedIndexColPixelWidth * NUM_INDEX_COLS) + paddedColPixelWidth_);
    leftMargin_ = paddedIndexColPixelWidth;
    rightMargin_ = right_ - paddedIndexColPixelWidth;
    textAreaWidth_ = rightMargin_ - leftMargin_;

    int totalCols = sequence_.length() / COL_SIZE;
    int totalLines = totalCols / colsPerLine();
    QSize areaSize = viewport()->size();
    int viewableLines = areaSize.height() / lineHeight_;

    verticalScrollBar()->setSingleStep(lineHeight_);
    verticalScrollBar()->setPageStep((areaSize.height() / lineHeight_) * lineHeight_);
    verticalScrollBar()->setRange(0, ((totalLines - viewableLines) * lineHeight_) + lineHeight_);
}

bool SequenceTextView::ensureSelectionExists(int index)
{
    if (index >= 0 && !selections_.count())
    {
        if (mouseDownIndex_ < 0)
        {
            mouseDownIndex_ = index;
        }

        Selection current;
        current.start = mouseDownIndex_;
        current.stop = mouseDownIndex_;
        addSelection(current);
    }

    return selections_.count();
}

int SequenceTextView::indexAt(const QPoint &point, SelectionDirection direction) const
{
    QPoint translatedPoint = toAbsoluteCoordinate(point);
    int index = charsLeftOf(translatedPoint.x()) + charsAbove(translatedPoint.y());

    if (isInColumnPadding(translatedPoint.x()))
    {
        if (direction == SelectionDirection_Unknown)
        {
            return -1;
        }
        else if (direction == SelectionDirection_Right)
        {
            return index - 1;
        }
    }

    return index;
}

int SequenceTextView::indexAt(const QPoint &point) const
{
    return indexAt(point, SelectionDirection_Unknown);
}

void SequenceTextView::insertInOrder(const Selection &selection)
{
    int numSelections = selections_.length();

    for (int i = 0; i < numSelections; ++i)
    {
        if (selections_[i].start > selection.start)
        {
            selections_.insert(i, selection);
            return;
        }
    }

    selections_.append(selection);
}

bool SequenceTextView::isInColumnPadding(int x) const
{
    int columnsLeftOfX = x / paddedColPixelWidth_;
    int positionInCol = x - (columnsLeftOfX * paddedColPixelWidth_);

    return positionInCol > colPixelWidth_;
}

bool SequenceTextView::isPressed(Qt::MouseButton button, const QMouseEvent *e) const
{
    return ((e->buttons() & button) == button);
}

bool SequenceTextView::isScrollable() const
{
    return verticalScrollBar()->minimum() < verticalScrollBar()->maximum();
}

bool SequenceTextView::isTrippleClick() const
{
    qint64 elapsed = timeSinceDoubleClick_.elapsed();
    return (elapsed > 0 && elapsed < TRIPLE_CLICK_SPEED);
}

void SequenceTextView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (allowSelect() && isPressed(Qt::LeftButton, e))
    {
        int index = indexAt(e->pos());
        beginTrippleClick();
        selectBlockContainingIndex(index);
    }
}

void SequenceTextView::mouseMoveEvent(QMouseEvent *e)
{
    if (allowSelect() && isPressed(Qt::LeftButton, e))
    {
        SelectionDirection direction = SelectionDirection_Right;

        if (mouseDownPoint_.y() > e->y())
        {
            direction = SelectionDirection_Left;
        }

        int index = indexAt(e->pos(), direction);

        if (ensureSelectionExists(index))
        {
            adjustSelectionToIndex(index);
            adjustScrollbarsToPoint(e->pos());
            viewport()->update();
        }
    }
}

void SequenceTextView::keyPressEvent(QKeyEvent *e)
{
    bool ctrl = (Qt::ControlModifier & e->modifiers()) == Qt::ControlModifier;
    bool c = (e->key() == Qt::Key_C);

    if (ctrl && c)
    {
        copyToClipboard();
    }
}

void SequenceTextView::mousePressEvent(QMouseEvent *e)
{
    if (allowSelect() && isPressed(Qt::LeftButton, e))
    {
        if (isTrippleClick())
        {
            selectAll();
        }
        else
        {
            beginSelecting(e);
        }
    }
}

void SequenceTextView::paintEvent(QPaintEvent * /* e */)
{
    QPainter painter(this->viewport());
    SequenceViewPainter viewPainter(this, &painter);
    viewPainter.paint();
}

void SequenceTextView::resizeEvent(QResizeEvent * /* e */)
{
    adjustComputedDrawingRegions();
}

Selection SequenceTextView::sanitizeSelection(const Selection &selection) const
{
    Selection sanitizedSelection = selection;
    sanitizedSelection.start = qMax(0, sanitizedSelection.start);
    sanitizedSelection.stop = qMin(sequence_.length() - 1, sanitizedSelection.stop);
    return sanitizedSelection;
}

void SequenceTextView::selectAll()
{
    clearSelections();
    addSelection(0, sequence_.length());
}

void SequenceTextView::selectBlockContainingIndex(int index)
{
    if (index >= 0)
    {
        clearSelections();
        Selection selectedBlock;
        selectedBlock.start = index - (index % COL_SIZE);
        selectedBlock.stop = selectedBlock.start + COL_SIZE - 1;
        addSelection(selectedBlock);
    }
}

void SequenceTextView::setAllowSelect(bool value)
{
    allowSelect_ = value;

    if (value)
    {
        setCursor(Qt::IBeamCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }

    verticalScrollBar()->setCursor(Qt::ArrowCursor);
}

void SequenceTextView::setSequence(const QString &sequence)
{
    sequence_ = sequence;
    clearSelections();
    adjustComputedDrawingRegions();
    update();
}

QString SequenceTextView::sequence() const
{
    return sequence_;
}


QPoint SequenceTextView::toAbsoluteCoordinate(const QPoint &point) const
{
    int x = qMax(leftMargin_, point.x());
    int y = qMax(0, point.y());
    int verticalScroll = verticalScrollBar()->value();

    y += verticalScroll - (verticalScroll % lineHeight_);
    x -= leftMargin_;

    return QPoint(x, y);
}

void SequenceTextView::onSelectionChanged()
{
    actionCopy_->setEnabled(currentSelection());
    emit selectionChanged();
}
