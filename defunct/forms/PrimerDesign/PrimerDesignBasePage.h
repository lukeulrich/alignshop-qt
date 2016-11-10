/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERDESIGNBASEPAGE_H
#define PRIMERDESIGNBASEPAGE_H

#include <QtGui/QWizardPage>
#include <QtCore/QTextStream>
#include <PrimerDesign/PrimerDesignInput.h>
#include <PrimerDesign/PrimerPairFinder.h>
#include <forms/PrimerDesign/PrimerDesignWizard.h>

namespace PrimerDesign
{
    /// The base page for the primer design wizard pages.
    class PrimerDesignBasePage : public QWizardPage
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent PrimerDesignWizard *
        PrimerDesignBasePage(PrimerDesignWizard *parent);

        /// Gets the underlying primer design wizard object.
        /// @return PrimerDesignWizard *
        PrimerDesignWizard *baseWizard();
    };
}

#endif // PRIMERDESIGNBASEPAGE_H
