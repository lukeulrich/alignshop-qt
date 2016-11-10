#include "PrimerPairHighlighter.h"

using namespace PrimerDesign;
using namespace Widgets;

PrimerPairHighlighter::PrimerPairHighlighter(SequenceTextView *dnaView)
{
    dnaView_ = dnaView;
    currentList_ = 0;
}

void PrimerPairHighlighter::clear()
{
    dnaView_->clearSelections();
}

void PrimerPairHighlighter::highlight(PrimerPair *pair)
{
    clear();

    if (!pair)
    {
        return;
    }

    QString sequence = dnaView_->sequence();
    DnaString reverse = pair->reversePrimer().sequence();
    QString forwardPrimer = pair->forwardPrimer().sequence()
            .mid(pair->params()->forwardPrefix.count());
    QString reversePrimer = reverse.reverseComplement()
            .mid(pair->params()->reversePrefix.count());

    int foreIndex = sequence.indexOf(forwardPrimer, 0, Qt::CaseInsensitive);
    int aftIndex = sequence.indexOf(reversePrimer, 0, Qt::CaseInsensitive);

    dnaView_->addSelection(
        foreIndex,
        foreIndex + pair->forwardPrimer().sequence().length() - 1);

    Selection reverseSelection;

    reverseSelection.color = QColor("#000");
    reverseSelection.backColor = QColor("#AAE0B3");
    reverseSelection.start = aftIndex;
    reverseSelection.stop = aftIndex + pair->reversePrimer().sequence().length() - 1;

    dnaView_->addSelection(reverseSelection);
}

void PrimerPairHighlighter::setCurrentList(ObservablePrimerPairList *list)
{
    if (currentList_)
    {
        QObject::disconnect(currentList_->notifier(), SIGNAL(currentChanged()), this, SLOT(onCurrentPairChanged()));
    }

    currentList_ = list;

    if (list)
    {
        QObject::connect(list->notifier(), SIGNAL(currentChanged()), this, SLOT(onCurrentPairChanged()));
    }

    onCurrentPairChanged();
}

void PrimerPairHighlighter::onCurrentPairChanged()
{
    if (!currentList_)
    {
        return;
    }

    highlight(currentList_->current());
}
