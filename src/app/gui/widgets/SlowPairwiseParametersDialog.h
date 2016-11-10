/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SLOWPAIRWISEPARAMETERSDIALOG_H
#define SLOWPAIRWISEPARAMETERSDIALOG_H

#include <QtGui/QDialog>
#include "../../core/enums.h"

namespace Ui {
    class SlowPairwiseParametersDialog;
}

class OptionSet;

class SlowPairwiseParametersDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit SlowPairwiseParametersDialog(QWidget *parent = 0);
    ~SlowPairwiseParametersDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    OptionSet pairwiseOptions() const;
    void setGrammar(const Grammar &grammar);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setWeightMatricesBasedOnGrammar();
    QString clustalWMatrixOption() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::SlowPairwiseParametersDialog *ui_;
    Grammar grammar_;
};

#endif // SLOWPAIRWISEPARAMETERSDIALOG_H
