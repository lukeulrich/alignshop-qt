#include "IPrimerDesignOM.h"
#include <forms/PrimerDesign/SequenceView.h>

using namespace PrimerDesign;

ObservablePrimerPairList * IPrimerDesignOMProvider::currentPrimerPairs(const QWidget *widget)
{
    PrimerPairGroup *current = groups(widget).current();
    if (current)
    {
        return &(current->pairs);
    }

    return 0;
}

ObservableSequence * IPrimerDesignOMProvider::currentSequence(const QWidget *widget)
{
    PrimerPairGroup *current = groups(widget).current();
    if (current)
    {
        return &(current->sequence);
    }

    return 0;
}

ObservablePrimerPairGroupList & IPrimerDesignOMProvider::groups(const QWidget *widget)
{
    SequenceView *win = (SequenceView*)widget->window();
    return win->primerDesignOM()->primerPairGroups;
}
