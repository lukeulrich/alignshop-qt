/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include <modeltest.h>

#include "DbSpec.h"

#include <QtCore/QProcess>


// ------------------------------------------------------------------------------------------------
// UI class import and naming
namespace Ui
{
    class MainWindow;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Forward declarations
class QFileDialog;
class QTreeView;

class AbstractAdocDataSource;
class AdocTreeModel;
class AdocTreeNode;
class AdocTypeFilterModel;
class AllSliceModel;
class AminoSliceModel;
class DnaSliceModel;
class RelatedTableModel;
class RnaSliceModel;
class SliceSortProxyModel;
class TableModel;

// Dialogs
class ImportSequencesDialog;
class PrimerFinderDialog;
class ProjectGroupSelectionDialog;

class MsaWindow;
class Msa;

/**
  * The main AlignShop window
  *
  * The UI and related forms are maintained through the QtDesigner. All interaction is routed through this class
  * which contains and tracks all child windows/dialogs.
  */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    // Events
    void closeEvent(QCloseEvent *event);            //!< Executed when MainWindow receives a close event

    // Utility functions
    void readPersistentSettings();                  //!< Loads persistent settings (those that persist after program has finished)
    void writePersistentSettings();                 //!< Writes persistent settings (those that persist after program has finished)

public slots:
    void createEditNewGroup();                      //!< Response handler for creating a new group
    void createEditNewTopLevelGroup();              //!< Response handler for creating a new project
    void onContainerTreeCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onItemTableViewDoubleClick(const QModelIndex &index);
    void setViewType(int viewType);
    void showImportSequencesDialog();               //!< Initiates the import sequence process

    void doSomething();

    void onReadyReadStdout();
    void onStateChanged(QProcess::ProcessState newState);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // ------------------------------------------------------------------------------------------------
    // Public slots
    void openFile(const QString &file);             //!< Attempt to open file

protected:
    void recurseFetchExpandedGroups(const QModelIndex &index, const QTreeView *treeView, QList<QModelIndex> *list) const;

private:
    void initializeAdocDataSource();
    void initializeTableModels();

    AbstractAdocDataSource *adocDataSource_;        //!< Overarching data source containing all relevant data

    // UI elements
    Ui::MainWindow *ui_;                            //!< MainWindow form
    QFileDialog *selectAdocFileDialog_;             //!< Dialog used to select a single Adoc file for opening
    ImportSequencesDialog *importSequencesDialog_;  //!< Detailed dialog box for importing sequences
    PrimerFinderDialog *primerFinderDialog_;        //!< Detailed dialog box for designing primers
    ProjectGroupSelectionDialog *projectGroupSelectionDialog_;

    // Actual data models that interface with the data source
    AdocTreeModel *adocTreeModel_;
    RelatedTableModel *aminoSubseqsTable_;
    RelatedTableModel *dnaSubseqsTable_;
    RelatedTableModel *rnaSubseqsTable_;
    TableModel *aminoSeqsTable_;
    TableModel *dnaSeqsTable_;
    TableModel *rnaSeqsTable_;

    // Model adaptors
    // There are four view types: amino, dna, rna, and all items. Functionally speaking, all items is simply the adoc
    // data tree model by itself with no proxy; however, operations preserving expanded nodes when switching between
    // view types is complicated by having parallel models without the same proxy chain. In other words, the exact node
    // mapping process will be different if switching from the adoc tree model to a proxy model vs from a proxy to another
    // proxy. To simplify this process at the cost of some extra CPU cycles, we utilize a dummy pass through model for
    // the all items view, which is simply an AdocTypeFilterModel but without any filtering. This then places all view
    // models at the same level in the proxy chain hierarchy and dramatically simplifies managing changes.
    AdocTypeFilterModel *containerFilterModel_;             //!< Filters out all items that are not container-level items
    AllSliceModel *allSliceModel_;
    AminoSliceModel *aminoSliceModel_;
    DnaSliceModel *dnaSliceModel_;
    RnaSliceModel *rnaSliceModel_;
    SliceSortProxyModel *sliceSortProxyModel_;

    DbSpec adocDbSpec_;

    MsaWindow *msaWindow_;

    QProcess *process_;

private slots:
    void on_onActionDesign_Primers_triggered();

    void on_actionNew_document_triggered();
    void on_actionOpen_triggered();                         //!< Executed when actionOpen is triggered
    void onAdocDataSourceOpenError(const QString &file, const QString &error);
    void onDataTreeError(const QString &error);
    void onDataTreeReady(AdocTreeNode *root);
    void on_actionClose_triggered();
    void onMsaReady(Msa *msa, int tag);
    void onMsaError(const QString &error, int tag);
};



#endif // MAINWINDOW_H
