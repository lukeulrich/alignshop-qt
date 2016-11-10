/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERLISTPANEL_H
#define PRIMERLISTPANEL_H

#include <QtGui>
#include <PrimerDesign/ObservableSequence.h>
#include <PrimerDesign/PrimerPairListModel.h>
#include <forms/PrimerDesign/PrimerDesignWizard.h>

namespace Ui {
    class PrimerListPanel;
}

namespace PrimerDesign
{
    /// Represents a list of primers.
    class PrimerListPanel : public QFrame
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent QWidget * (Defaults to 0.)
        explicit PrimerListPanel(QWidget *parent = 0);

        /// The destsructor.
        ~PrimerListPanel();

        /// Filters the events.
        /// @param object QObject *
        /// @param event QEvent *
        /// @return bool
        bool eventFilter(QObject *object, QEvent *event);

    private:
        Ui::PrimerListPanel *ui;
        PrimerPairListModel primerPairs_;
        QSortFilterProxyModel sortedModel_;
        int lastSortedColumn_;
        Qt::SortOrder lastSortOrder_;

        /// Adds a new primer primer.
        /// @param params const PrimerDesignInput *
        /// @return void
        void addPrimer(const PrimerDesignInput *params);

    private slots:
        /// Triggered when the selection changes.
        /// @param selected const QItemSelection &
        /// @param deselected const QItemSelection &
        /// @return void
        void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

        /// Triggered when the current primer pair list changes.
        /// @return void
        void onCurrentChanged();

        /// Triggered when the current primer pair list is changing.
        /// @return void
        void onCurrentChanging();

        /// Triggered when the user wishes to add a new primer(s)
        /// @return void
        void onAddPrimerClicked();

        /// Triggered when the user wants to see the current primer pair's parameters.
        /// @return void
        void onShowInfoClicked();

        /// Triggered when one or more rows change.
        /// @return void
        void rowsChanged();

        /// Triggered when the user copies.
        /// @return void
        void onCopyAction();

        /// Triggered when a column header is pressed.
        /// @param logicalIndex int
        /// @return void
        void onHeaderPressed(int logicalIndex);

        /// Triggered when a column's sort indicator changes.
        /// @param logicalIndex int
        /// @param order Qt::SortOrder
        /// @return void
        void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    };
}

#endif // PRIMERLISTPANEL_H
