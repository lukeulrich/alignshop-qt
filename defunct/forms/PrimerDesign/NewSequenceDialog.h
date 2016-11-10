/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef NEWSEQUENCEDIALOG_H
#define NEWSEQUENCEDIALOG_H

#include <QDialog>
#include <PrimerDesign/DnaSequence.h>

namespace Ui {
    class NewSequenceDialog;
}

namespace PrimerDesign
{
    /// The dialog responsible for gathering new sequence data.
    class NewSequenceDialog : public QDialog
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent QWidget * (Defaults to 0.)
        explicit NewSequenceDialog(QWidget *parent = 0);

        /// The destructor.
        ~NewSequenceDialog();

        /// Retrieves the dna sequence which was input.
        /// @return DnaSequence
        DnaSequence toSequence();

    private:
        Ui::NewSequenceDialog *ui;

        /// Validates the sequence name.
        /// @return bool
        bool validateName();

        /// Validates the sequence value.
        /// @return bool
        bool validateSequence();

    private slots:
        /// Enables the submit button if the form is valid.
        /// @return void
        void enableOkIfValidForm();
    };
}

#endif // NEWSEQUENCEDIALOG_H
