#ifndef PRIMERGENERATIONDIALOG_H
#define PRIMERGENERATIONDIALOG_H

/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include <QtGui>
#include <PrimerDesign/PrimerDesignInput.h>
#include <PrimerDesign/PrimerPairFinder.h>

namespace Ui {
    class PrimerGenerationDialog;
}

namespace PrimerDesign
{
    /// The dialog which generates primer pairs and indicates progress for the generation.
    class PrimerGenerationDialog : public QDialog
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param input PrimerDesignInput *
        /// @param parent QWidget * (Defaults to 0.)
        explicit PrimerGenerationDialog(PrimerDesignInput *input, QWidget *parent = 0);

        /// The destructor.
        ~PrimerGenerationDialog();

        /// Gets the result of the find operation.
        /// @return PrimerPairFinderResult
        PrimerPairFinderResult finderResult() const;

    protected:
        /// Called when the window is closed.
        /// @param e QCloseEvent *
        /// @return void
        void closeEvent(QCloseEvent *e);

        /// Called when the window is shown.
        /// @param e QShowEvent *
        /// @return void
        void showEvent(QShowEvent *e);

        /// Called when a key is pressed.
        /// @param e QKeyEvent *
        /// @return void
        void keyPressEvent(QKeyEvent *e);

    private slots:
        /// Triggered when the progress should be updated.
        /// @return void
        void updateProgress();

        /// Triggered when generation of primers is complete.
        /// @return void
        void generationComplete();

        /// Triggered when primer generation should be cancelled.
        /// @return void
        void beginCancel();

    private:
        Ui::PrimerGenerationDialog *ui;
        QTimer *timer_;
        QThread *thread_;

        /// Gets a value indicating whether or not the window is safe to be closed.
        /// @return bool
        bool isReadyToClose();
    };
}

#endif // PRIMERGENERATIONDIALOG_H
