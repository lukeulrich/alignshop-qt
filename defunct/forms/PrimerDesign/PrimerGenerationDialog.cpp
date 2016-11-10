#include "PrimerGenerationDialog.h"
#include "ui_PrimerGenerationDialog.h"
#include <PrimerDesign/PrimerPairFinder.h>

using namespace PrimerDesign;

class PrimerGenThread : public QThread
{
public:
    volatile bool complete;
    volatile bool cancelled;
    PrimerPairFinderResult results;

    PrimerGenThread(PrimerDesignInput *input, QWidget *parent)
        : QThread(parent)
    {
        cancelled = false;
        complete = false;
        finder_ = new PrimerPairFinder(input);
    }

    ~PrimerGenThread()
    {
        delete finder_;
    }

    void cancel()
    {
        cancelled = true;
        finder_->cancel();
    }

protected:
    void run()
    {
        results = finder_->findPrimerPairs();

        if (!results.isError)
        {
            QList<PrimerPair > pairs = results.value;
            qSort(pairs.begin(), pairs.end(), pairSorter);
            results.value = pairs.mid(0, qMin(pairs.count(), 50));
        }

        complete = true;
    }

private:
    PrimerPairFinder *finder_;

private:
    static bool pairSorter(const PrimerPair &p1, const PrimerPair &p2)
    {
        return p1.score() < p2.score();
    }
};

PrimerGenerationDialog::PrimerGenerationDialog(PrimerDesignInput *input, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrimerGenerationDialog)
{
    ui->setupUi(this);

    QPoint parentPosition = parent->mapToGlobal(parent->geometry().center());
    QRect bounds = this->geometry();
    bounds.setX(parentPosition.x() - (bounds.width() / 2));
    bounds.setY(parentPosition.y() - (bounds.height() / 2));
    setGeometry(bounds);

    thread_ = new PrimerGenThread(input, this);
    timer_ = new QTimer(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    QObject::connect(timer_, SIGNAL(timeout()), this, SLOT(updateProgress()));
    QObject::connect(thread_, SIGNAL(finished()), this, SLOT(generationComplete()));

    timer_->start(250);
}

PrimerGenerationDialog::~PrimerGenerationDialog()
{
    delete ui;
    delete timer_;
    delete thread_;
}

void PrimerGenerationDialog::updateProgress()
{
    ui->progressBar->setValue((ui->progressBar->value() + 1) % ui->progressBar->maximum());
}

void PrimerGenerationDialog::closeEvent(QCloseEvent *evt)
{
    if (!isReadyToClose())
    {
        evt->ignore();
    }
}

bool PrimerGenerationDialog::isReadyToClose()
{
    return ((PrimerGenThread*)thread_)->complete;
}

void PrimerGenerationDialog::showEvent(QShowEvent *)
{
    thread_->start(QThread::HighPriority);
}

void PrimerGenerationDialog::generationComplete()
{
    if (((PrimerGenThread*)thread_)->cancelled)
    {
        reject();
    }
    else
    {
        accept();
    }
}

void PrimerGenerationDialog::beginCancel()
{
    ((PrimerGenThread*)thread_)->cancel();
}

void PrimerGenerationDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(e);
    }
}

PrimerPairFinderResult PrimerGenerationDialog::finderResult() const
{
    return ((PrimerGenThread*)thread_)->results;
}
