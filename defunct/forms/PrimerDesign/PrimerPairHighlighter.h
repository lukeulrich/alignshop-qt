/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPAIRHIGHLIGHTER_H
#define PRIMERPAIRHIGHLIGHTER_H

#include <PrimerDesign/PrimerPair.h>
#include <DnaString.h>
#include <widgets/SequenceTextView.h>

using namespace Widgets;

namespace PrimerDesign
{
    /// Responsible for highlighting a primer pair in a sequence text view.
    class PrimerPairHighlighter : public QObject
    {
        Q_OBJECT;

    public:
        /// The constructor.
        /// @param dnaView SequenceTextView *
        explicit PrimerPairHighlighter(SequenceTextView *dnaView);

        /// Sets the current primer pair list.
        /// @param list ObservablePrimerPairList *
        /// @return void
        void setCurrentList(ObservablePrimerPairList *list);

        /// Highlights the specified pair.
        /// @param pair PrimerPair *
        /// @return void
        void highlight(PrimerPair *pair);

        /// Clears the current highlight.
        /// @return void
        void clear();

    private slots:
        /// Triggered when the current pair changes.
        /// @return void
        void onCurrentPairChanged();

    private:
        SequenceTextView *dnaView_;
        ObservablePrimerPairList *currentList_;
    };
}

#endif // PRIMERPAIRHIGHLIGHTER_H
