/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPARAMSPREVIEWDIALOG_H
#define PRIMERPARAMSPREVIEWDIALOG_H

#include <QDialog>
#include <PrimerDesign/PrimerPair.h>

namespace Ui {
    class PrimerParamsPreviewDialog;
}

namespace PrimerDesign
{
    /// The dialog displaying the primer params.
    class PrimerParamsPreviewDialog : public QDialog
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param pair PrimerPair *
        /// @param parent QWidget * (Defaults to 0.)
        explicit PrimerParamsPreviewDialog(PrimerPair *pair, QWidget *parent = 0);

        /// The destructor.
        ~PrimerParamsPreviewDialog();

    private slots:
        /// Triggered when the create more button is clicked.
        /// @return void
        void onCreateMoreClicked();

    private:
        Ui::PrimerParamsPreviewDialog *ui;
        PrimerDesignInput *params_;

        /// Converts a range object to display text.
        /// @param range const Range &
        /// @return QString
        QString toString(const Range &range);

        /// Converts a range object to display text.
        /// @param range const RangeF &
        /// @return QString
        QString toString(const RangeF &range);
    };
}

#endif // PRIMERPARAMSPREVIEWDIALOG_H
