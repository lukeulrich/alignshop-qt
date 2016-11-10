/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERRESULTSPAGE_H
#define PRIMERRESULTSPAGE_H

#include <QWizardPage>
#include <PrimerDesign/PrimerPair.h>
#include <forms/PrimerDesign/PrimerDesignBasePage.h>
#include <PrimerDesign/PrimerPairListModel.h>

namespace Ui {
    class PrimerResultsPage;
}

namespace PrimerDesign
{
    /// The control responsible for displaying the primer design results and allowing
    /// the user to select one or more primer pairs to keep.
    class PrimerResultsPage : public PrimerDesignBasePage
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent PrimerDesignWizard * (Defaults to 0.)
        explicit PrimerResultsPage(PrimerDesignWizard *parent = 0);

        /// The destructor.
        ~PrimerResultsPage();

        /// Initializes the page.
        /// @return void
        virtual void initializePage();

        /// Validates the page.
        /// @return bool
        virtual bool validatePage();

    private slots:
        /// Triggered when the selected primer pair changes.
        /// @param selected const QItemSelection &
        /// @param deselected const QItemSelection &
        /// @return void
        void onPrimerSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    private:
        Ui::PrimerResultsPage *ui;
        PrimerPairListModel *model_;
        ObservablePrimerPairList list_;
    };
}

#endif // PRIMERRESULTSPAGE_H
