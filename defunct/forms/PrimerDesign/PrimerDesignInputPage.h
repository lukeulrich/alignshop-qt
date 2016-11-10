/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PrimerDesignInputPage_H
#define PrimerDesignInputPage_H

#include <QtGui>
#include <PrimerDesign/AmpliconRange.h>
#include <forms/PrimerDesign/PrimerDesignBasePage.h>
#include <forms/PrimerDesign/RestrictionEnzymeTextbox.h>
#include "SequenceValidator.h"

namespace Ui {
    class PrimerDesignInputPage;
}

namespace PrimerDesign
{
    /// The page responsible for gathering user input for the primer design process.
    class PrimerDesignInputPage : public PrimerDesignBasePage
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent PrimerDesignWizard * (Defaults to 0.)
        explicit PrimerDesignInputPage(PrimerDesignWizard *parent = 0);

        /// The destructor.
        ~PrimerDesignInputPage();

        /// Cleans up the page.
        /// @return void
        virtual void cleanupPage();

        /// Initializes the page.
        /// @return void
        virtual void initializePage();

        /// Validates the page.
        /// @return bool
        virtual bool validatePage();

    private:
        bool updating_;
        Ui::PrimerDesignInputPage *ui;
        AmpliconRange windowBounds_;
        QThread *worker_;
        SequenceValidator seqValidator_;

        /// Sets the amplicon window bounds.
        /// @param start int
        /// @param stop int
        /// @return void
        void setWindowBounds(int start, int stop);

        /// Updates the min and max amplicon elements.
        /// @return void
        void updateMinAndMaxElements();

        /// Updates the amplicon selection.
        /// @return void
        void updateSelection();

        /// Launches the restriction enzyme browser.
        /// @param txt RestrictionEnzymeTextbox *
        /// @param captionParam QString
        /// @return void
        void browse(RestrictionEnzymeTextbox *txt, QString captionParam);

    private slots:
        /// Hides the error message.
        /// @return void
        void hideError();

        /// Triggered when the amplicon selection changes.
        /// @return void
        void onSelectionChanged();

        /// Triggered when the amplicon start changes.
        /// @return void
        void onStartChanged();

        /// Triggered when the amplicon stop changes.
        /// @return void
        void onStopChanged();

        /// Triggered when the amplicon window range changes.
        /// @return void
        void onWindowRangeChanged();

        /// Triggered when the forward prefix browse button is clicked.
        /// @return void
        void forwardPrefixBrowseClicked();

        /// Triggered when the reverse prefix browse button is clicked.
        /// @return void
        void reversePrefixBrowseClicked();
    };
}

#endif // PrimerDesignInputPage_H
