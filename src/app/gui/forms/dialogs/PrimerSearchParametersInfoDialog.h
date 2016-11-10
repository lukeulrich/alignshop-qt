/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERSEARCHPARAMETERSINFODIALOG_H
#define PRIMERSEARCHPARAMETERSINFODIALOG_H

#include <QtGui/QDialog>
#include "../../../primer/PrimerSearchParameters.h"
#include "../../../core/global.h"

namespace Ui {
    class PrimerSearchParametersInfoDialog;
}

class RestrictionEnyzme;

class PrimerSearchParametersInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrimerSearchParametersInfoDialog(QWidget *parent = nullptr);
    ~PrimerSearchParametersInfoDialog();

    void setPrimerSearchParameters(const PrimerSearchParameters *primerSearchParameters);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString restrictionEnzymeText(const RestrictionEnzyme &restrictionEnzyme) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::PrimerSearchParametersInfoDialog *ui_;
};

#endif // PRIMERSEARCHPARAMETERSINFODIALOG_H
