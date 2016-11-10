/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FASTPAIRWISEPARAMETERSDIALOG_H
#define FASTPAIRWISEPARAMETERSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class FastPairwiseParametersDialog;
}

class OptionSet;

/**
  * FastPairwiseParametersDialog provides a dialog for specifying the ClustalW fast pairwise alignment options.
  */
class FastPairwiseParametersDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and Destructor
    explicit FastPairwiseParametersDialog(QWidget *parent = 0);
    ~FastPairwiseParametersDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    OptionSet pairwiseOptions() const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString scoreString() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::FastPairwiseParametersDialog *ui_;
};

#endif // FASTPAIRWISEPARAMETERSDIALOG_H
