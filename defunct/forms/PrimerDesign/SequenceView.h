/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtGui>
#include <forms/PrimerDesign/NewSequenceDialog.h>
#include <PrimerDesign/ObservableSequence.h>
#include <PrimerDesign/SequenceListModel.h>
#include <PrimerDesign/IPrimerDesignOM.h>

namespace Ui {
    class SequenceView;
}

namespace PrimerDesign
{
    /// The main window of the primer designer.
    class SequenceView : public QMainWindow, public IPrimerDesignOMProvider
    {
        Q_OBJECT

    public:
        /// Constructs a new instance.
        /// @param parent QWidget * (Defaults to 0.)
        explicit SequenceView(QWidget *parent = 0);

        /// The destructor.
        ~SequenceView();

        /// Filters window events.
        /// @param object QObject *
        /// @param event QEvent *
        /// @return bool
        bool eventFilter(QObject *object, QEvent *event);

        /// Gets the primer design object model for this application instance.
        /// @return PrimerDesignOM*
        PrimerDesignOM* primerDesignOM();

    protected:
        /// Triggered when the window is closing.
        /// @param e QCloseEvent *
        /// @return void
        void closeEvent(QCloseEvent *e);

    private:
        Ui::SequenceView *ui;
        SequenceListModel sequences_;
        QSortFilterProxyModel sortedModel_;
        PrimerDesignOM primerDesignOM_;
        QString currentFileName_;

        /// Binds to the primer design object model.
        /// @return void
        void bindToObjectModel();

        /// Loads the previously run data file.
        /// @return void
        void loadPreviousRun();

        /// Sorts the data model (the sequence list).
        /// @return void
        void sort();

        /// Writes the current object model to disk.
        /// @return void
        void writeToFile();

        /// Sets the name of the current data file.
        /// @param filename const QString &
        /// @return void
        void setCurrentFileName(const QString &filename);

        /// Loads the current data file.
        /// @return void
        void load();

        /// Updates the sequence list view's selected index.
        /// @return void
        void updateSequenceListViewSelectedIndex();

        /// Toggles whether or not the zero-item message is displayed.
        /// @return void
        void toggleZeroItemsMessageVisibility();

        /// Persists the current filename so that it can be loaded when the
        /// application runs again.
        /// @return void
        void rememberCurrentFileName();

        /// Loads the last file which was run.
        /// @return bool
        bool loadLastFile();

    private slots:
        /// Triggered when the 'add sequence' button is clicked.
        /// @return void
        void onAddSequenceClicked();

        /// Triggered when the user opts to open.
        /// @return void
        void onFileOpenClicked();

        /// Triggered when the user opts to save.
        /// @return void
        void saveAs();

        /// Triggered when the user opts to save.
        /// @return void
        void save();

        /// Triggered when the current sequence changes.
        /// @return void
        void onCurrentChanged();

        /// Triggered when a sequence is added.
        /// @param index int
        /// @return void
        void onSequenceAdded(int index);

        /// Triggered when the sequence's name changes.
        /// @param sequence ObservableSequence *
        /// @return void
        void onNameChanged(ObservableSequence *sequence);

        /// Triggered when the selected sequence changes.
        /// @param selected const QItemSelection &
        /// @param deselected const QItemSelection &
        /// @return void
        void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    };
}

#endif // SEQUENCEVIEW_H
