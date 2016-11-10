/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QPersistentModelIndex>
#include <QtGui/QMainWindow>

#include "../../core/Adoc.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/Entities/AbstractMsa.h"

class QFileDialog;
class QGraphicsView;
class QItemSelection;
class QModelIndex;
class QPlainTextEdit;
class QTableView;
class QUndoStack;

class AbstractMsaBuilder;
class AdocTreeModel;
class AdocTreeNodeFilterModel;
class AminoMsaColumnAdapter;
class AminoSeqColumnAdapter;
class BlastDatabaseModel;
class BlastDatabaseWindow;
class BlastDialog;
class BlastReportColumnAdapter;
class BlastReportModel;
class BlastSequenceImporter;
class DnaMsaColumnAdapter;
class DnaSeqColumnAdapter;
class DnaSeqCommandPrimerMutator;
class DnaSeqFeatureItemSynchronizer;
class DnaSeqPrimerVectorMutator;
class DnaSeqPrimerModel;
class LinearGraphicsScene;
class MsaAlignerOptionsDialog;
class MsaBuilderFactory;
class MsaWindow;
class MultiSeqTableModel;
class SequenceImporter;
class SequenceTextView;
class TaskAdocConnector;
class TaskManager;
class TextPixmapRenderer;
class TransientTaskColumnAdapter;

// Flex tabs
class BlastViewTab;
class PrimersViewTab;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString titleString() const;

public slots:
    void createGroupAndStartEditing(const QModelIndex &parentIndex = QModelIndex(), const QString &seedText = "New group");
    void newDocument();
    void openDocument(const QString &fileName);
    bool save();
    bool saveAs();


Q_SIGNALS:
    void aboutToClose();


protected:
    void closeEvent(QCloseEvent *closeEvent);


private Q_SLOTS:
    // Action reaction slots
    void onActionNewSequence();
    void onActionNewFolder();
    void onActionImport();
    void onActionOpen();
    void onActionRevert();
    void onActionUndo();
    void onActionCut();
    void onActionPaste();
    void onActionDelete();
    void onActionAlign();
    void onActionBlast();
    void onActionBlastDatabaseManager();
    void onActionTaskManager();
    void onActionAboutAlignShop();
    void showLicense();

    // TreeView reaction slots
    void onTreeViewContextMenuRequested(const QPoint &position);
    void onTreeViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onTreeViewCut(const QItemSelection &selection);
    void onTreeViewDeleteSelection(const QItemSelection &treeSelection);
    void onTreeViewPasteTo(const QModelIndex &index);

    // TableView reaction slots
    void onTableViewCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void onTableViewCut(const QItemSelection &tableSelection);
    void onTableViewDeleteSelection(const QItemSelection &tableSelection);
    void onTableViewDoubleClicked(const QModelIndex &clickedIndex);
    void onTableViewEnterPressed(const QModelIndex &currentIndex);
    void onTableViewLayoutChanged();
    void onTableViewModelAboutToReset();                                        // For when the table view has changed comletely
    void onTableViewPasteTo(const QModelIndex &tableIndex);
    void onTableViewSelectionChanged();

    // Various other reaction slots
    void onEntityStateExited();
    void onImportError(const QString &errorMessage);
    void onImportSuccessful(const QModelIndex &parentIndex);
    void onModifiedChanged();
    void onMsaWindowAboutToClose(MsaWindow *msaWindow);
    void onMultiSeqTableModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onSeqGraphicsViewWheelEvent(int delta);
    void onTextEditFocusLost();
    void onTextEditUndoAvailable(bool available);
    void onUndoStackCleanChanged(bool clean);



private:
    int askUserToSaveChanges();
    bool canAlignSelection() const;                 //!< Returns true if the selection contains at least two amino or dna sequences and all other selected items are of the same type; false otherwise
    bool canBlastSelection() const;                 //!< Returns true if the selection contains at least one amino or dna sequence and all sequences are of the same type; false otherwise
    bool canMakePrimersWithSelection() const;       //!< Returns true if the currently selected index is a DNA entity; false otherwise
    void deleteAdocTreeIndices(const QModelIndexList &modelIndexList);  // modelIndexList should belong to the treemodel
    void enableDisableCutPasteDeleteActions();
    bool loadAlignment(const AbstractMsaSPtr &abstractMsa);
    void openMsaWindowFromNode(AdocTreeNode *adocTreeNode);
    void updateEntityAdapterSpecifications();
    void updateEntityNotes(const QModelIndex &index);
    void readPersistentSettings();                  //!< Loads persistent settings (those that persist after program has finished)
    void writePersistentSettings();                 //!< Writes persistent settings (those that persist after program has finished)

    AbstractMsaBuilder *getAminoMsaBuilderFromUser();
    AbstractMsaBuilder *getDnaMsaBuilderFromUser();
    AbstractMsaBuilder *getMsaBuilderOptions(const MsaBuilderFactory &msaBuilderFactory, MsaAlignerOptionsDialog *msaAlignerOptionsDialog) const;


    // Stack based variables
    Adoc adoc_;

    // The all-important undo stack
    QUndoStack *undoStack_;
    bool modifiedWithNoUndo_;

    // Model and adapter variables
    AdocTreeModel *adocTreeModel_;
    AdocTreeNodeFilterModel *containerModel_;
    MultiSeqTableModel *multiSeqTableModel_;
    AminoMsaColumnAdapter *aminoMsaColumnAdapter_;
    AminoSeqColumnAdapter *aminoSeqColumnAdapter_;
    BlastReportColumnAdapter *blastReportColumnAdapter_;
    DnaMsaColumnAdapter *dnaMsaColumnAdapter_;
    DnaSeqColumnAdapter *dnaSeqColumnAdapter_;
    TransientTaskColumnAdapter *transientTaskColumnAdapter_;

    // UI related variables
    Ui::MainWindow *ui_;
    QPlainTextEdit *notesTextEdit_;
    BlastViewTab *blastViewTab_;
    PrimersViewTab *primersViewTab_;
    SequenceTextView *sequenceTextView_;
    LinearGraphicsScene *seqGraphicsScene_;
    QFileDialog *importFileDialog_;
    QFileDialog *saveAsDialog_;
    // Hash of open MsaWindow associated with Msa entities
    QHash<AbstractMsaSPtr, MsaWindow *> msaWindows_;

    // The importer instance
    SequenceImporter *sequenceImporter_;

    // The following variables are needed for tracking the current entity index and dealing with its notes
    QPersistentModelIndex activeEntityIndex_;        // The entity currently being viewed in the notes tab
    bool adocModifiedBeforeNotes_;

    QWidget *taskManagerWindow_;
    TaskManager *taskManager_;
    TaskAdocConnector *taskAdocConnector_;

    // Blast database model
    BlastDatabaseModel *blastDatabaseModel_;
    BlastDatabaseWindow *blastDatabaseWindow_;

    // Blast dialog
    BlastDialog *blastDialog_;

    // Blast sequence importer
    BlastSequenceImporter *blastSequenceImporter_;

    DnaSeqFeatureItemSynchronizer *dnaSeqFeatureItemSynchronizer_;
};

#endif // MAINWINDOW_H
