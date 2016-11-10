/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERDETAILSPANEL_H
#define PRIMERDETAILSPANEL_H

#include <QtGui>
#include <PrimerDesign/PrimerPairGroup.h>
#include <forms/PrimerDesign/PrimerPairHighlighter.h>

namespace Ui {
    class SequenceDetailsPanel;
}

namespace PrimerDesign
{
    /// Visually represents the sequence details.
    class SequenceDetailsPanel : public QFrame
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent QWidget * (Defaults to 0.)
        explicit SequenceDetailsPanel(QWidget *parent = 0);

        /// The destructor.
        ~SequenceDetailsPanel();

    private slots:
        /// Triggered when the name changes.
        /// @return void
        void nameChanged();

        /// Triggered when the current sequence changes.
        /// @return void
        void onCurrentChanged();

        /// Triggered when the current sequence is changing.
        /// @return void
        void onCurrentChanging();

        /// Triggered when the current sequence's name was edited.
        /// @return void
        void onSequenceNameValueEdited();

        /// Triggered when the current sequence's name is being edited.
        /// @return void
        void onSequenceNotesValueEdited();

    private:
        Ui::SequenceDetailsPanel *ui;
        PrimerPairHighlighter *highlighter_;
    };
}

#endif // PRIMERDETAILSPANEL_H
