#include <QApplication>
#include <QtCore>
#include <QtDebug>
#include <QtGui>
#include <memory>

#include "../AlignShop/src/graphics/LinearColorStyle.h"

using namespace std;

class Spacer : public QWidget
{
public:
    Spacer(QColor color) : color_(color)
    {
    }

    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.fillRect(rect(), color_);
    }

private:
    QColor color_;
};

class MyWidget : public QAbstractScrollArea
{
public:
    MyWidget() : style_(PairQColor(QColor(12, 76, 32), QColor(234, 23, 78)))
    {
        setViewportMargins(40, 10, 20, 30);

        horizontalScrollBar()->setRange(0, 1000);
        verticalScrollBar()->setRange(0, 1000);

        QGridLayout *gridLayout = new QGridLayout();
        gridLayout->setSpacing(0);
        gridLayout->setMargin(0);

        Spacer *fake = new Spacer(Qt::green);
        fake->setFixedSize(40, 10);
        gridLayout->addWidget(fake, 0, 0);

        Spacer *horiz = new Spacer(Qt::red);
        Spacer *vert = new Spacer(Qt::magenta);

        gridLayout->addWidget(horiz, 0, 1);
        gridLayout->addWidget(vert, 1, 0);
        gridLayout->addWidget(this->viewport(), 1, 1);

        this->setLayout(gridLayout);

        setMinimumSize(QSize(150, 150));
    }

    void resizeEvent(QResizeEvent *e)
    {
        QAbstractScrollArea::resizeEvent(e);
        setUpdatesEnabled(false);

        QRect vRect = verticalScrollBar()->geometry();
        QRect hRect = horizontalScrollBar()->geometry();

        vRect.setTop(10);
        vRect.setHeight(viewport()->height());
        verticalScrollBar()->setGeometry(vRect);

        hRect.setLeft(40);
        hRect.setWidth(viewport()->width());
        horizontalScrollBar()->setGeometry(hRect);

        horizontalScrollBar()->setRange(0, qMax(0, 1000 - viewport()->width()));
        verticalScrollBar()->setRange(0, qMax(0, 1000 - viewport()->height()));
        setUpdatesEnabled(true);
    }

    virtual void paintEvent(QPaintEvent *paintEvent)
    {
        int left = horizontalScrollBar()->value();
        int right = qMin(left + viewport()->width(), 1000);

        QPainter painter(viewport());
        for (int i=left, j=0; i< right; ++i, ++j)
        {
            qreal p = static_cast<double>(i) / 1000.;
            painter.fillRect(j, 0, 1, 50, style_.linearColor(p));
        }

        painter.setFont(QFont("monospace", 32));
        painter.drawText(-left, 75, "This is one very long string to help demonstrate scrolling");
    }

private:
    LinearColorStyle style_;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QSplitter splitter;
    splitter.setHandleWidth(2);
    splitter.setStyleSheet("QSplitter::handle:horizontal { background: white; border-left: 1px dotted #000; } "
                           "QSplitter::handle:horizontal:hover { background: gray; }");
    splitter.addWidget(new Spacer(Qt::white));
    splitter.addWidget(new Spacer(Qt::white));
    splitter.addWidget(new MyWidget());
    splitter.show();


    return app.exec();
}
