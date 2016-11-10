/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERDESIGNWIZARD_H
#define PRIMERDESIGNWIZARD_H

#include <QWizard>
#include <QtCore>
#include <PrimerDesign/DnaSequence.h>
#include <PrimerDesign/PrimerDesignInput.h>
#include <PrimerDesign/PrimerPairFinder.h>

namespace Ui {
    class PrimerDesignWizard;
}

namespace PrimerDesign
{
    /// The primer design wizard object responsible for gathering user input and
    /// generating primers pairs based on that input.
    class PrimerDesignWizard : public QWizard
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent QWidget *
        /// @param sequence DnaSequence *
        /// @param params const PrimerDesignInput *
        PrimerDesignWizard(QWidget *parent, DnaSequence *sequence, const PrimerDesignInput *params);

        /// The destructor.
        ~PrimerDesignWizard();

        /// Gets the primer design input parameters.
        /// @return PrimerDesignInput *
        PrimerDesignInput *getInput();

        /// Gets the results of the find operation.
        /// @return PrimerPairFinderResult *
        PrimerPairFinderResult *getFinderResult();

        /// Sets the result of the find operation.
        /// @param result const PrimerPairFinderResult &
        /// @return void
        void setFinderResult(const PrimerPairFinderResult &result);

    private:
        static const char * WORKING_FILE;
        Ui::PrimerDesignWizard *ui;
        PrimerDesignInputRef input_;
        PrimerPairFinderResult finderResult_;
    };
}

#endif // PRIMERDESIGNWIZARD_H
