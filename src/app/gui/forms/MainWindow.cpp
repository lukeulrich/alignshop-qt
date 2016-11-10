/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFinalState>
#include <QtCore/QSettings>
#include <QtCore/QState>

#include <QtGui/QAbstractButton>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QDockWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QItemSelection>
#include <QtGui/QMessageBox>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QProgressDialog>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoView>


#include <boost/make_shared.hpp>

#include <QtDebug>

#include "AminoMsaWindow.h"
#include "BlastDatabaseWindow.h"
#include "BlastDialog.h"
#include "DnaMsaWindow.h"
#include "MainWindow.h"
#include "MsaWindow.h"
#include "ui_MainWindow.h"

#include "dialogs/AboutDialog.h"
#include "dialogs/NewSequenceEntityDialog.h"
#include "dialogs/LicenseInfoDialog.h"

#include "../../core/Entities/BlastReport.h"
#include "../../core/Entities/TransientTask.h"
#include "../../core/factories/DynamicSeqFactory.h"
#include "../../core/factories/AminoSeqFactory.h"
#include "../../core/factories/DnaSeqFactory.h"
#include "../../core/constants.h"

#include "../Commands/ConditionalUndoCommand.h"
#include "../Commands/InsertAdocTreeNodesCommand.h"
#include "../Commands/InsertBlastReportsCommand.h"
#include "../delegates/ClipboardStateItemDelegate.h"
#include "../delegates/SpinBoxDelegate.h"
#include "../models/ColumnAdapters/AminoMsaColumnAdapter.h"
#include "../models/ColumnAdapters/AminoSeqColumnAdapter.h"
#include "../models/ColumnAdapters/BlastReportColumnAdapter.h"
#include "../models/ColumnAdapters/DnaMsaColumnAdapter.h"
#include "../models/ColumnAdapters/DnaSeqColumnAdapter.h"
#include "../models/ColumnAdapters/TransientTaskColumnAdapter.h"
#include "../models/AdocTreeModel.h"
#include "../models/AdocTreeNodeFilterModel.h"
#include "../models/BlastDatabaseModel.h"
#include "../models/MultiSeqTableModel.h"
#include "../models/TaskModel.h"
#include "../widgets/SequenceTextView.h"
#include "../widgets/views/WheelZoomGraphicsView.h"
#include "../EventFilters/WidgetFocusObserver.h"
#include "../SequenceImporter.h"
#include "../Services/AdocTreeNodeEraserService.h"
#include "../Services/BlastSequenceImporter.h"
#include "../Services/DnaSeqFeatureItemSynchronizer.h"
#include "../Services/HeaderColumnSelector.h"
#include "../Services/TaskAdocConnector.h"
#include "../Services/TaskManager.h"
#include "../Services/Tasks/BlastTask.h"
#include "../Services/Tasks/BuildMsaTask.h"

#if defined QT_DEBUG && defined Q_OS_LINUX
#include <modeltest.h>
#endif

#include "flextabs/BlastViewTab.h"
#include "flextabs/PrimersViewTab.h"

#include "../Commands/InsertTaskNodesCommand.h"

#include "../painting/LinearGraphicsScene.h"
#include "../painting/gitems/SeqBioStringItem.h"
#include "../painting/gitems/AminoSeqItem.h"
#include "../painting/gitems/DnaSeqItem.h"
#include "../painting/gitems/RangeHandleItem.h"
#include "../painting/gitems/RangeHandlePairItem.h"
#include "../painting/gitems/LinearRulerItem.h"


static const double kDefaultSeqSceneVerticalPadding = 5.;      // Amount of border to give to the scene
static const double kDefaultSeqSceneHorizontalPadding = 10.;   // Amount of border to give to the scene


#include "dialogs/MsaAlignerOptionsDialog.h"
#include "../../core/factories/MsaBuilderFactory.h"
#include "../factories/MsaBuilderOptionsWidgetFactory.h"
#include "../../core/Services/AbstractMsaBuilder.h"

void copyFolder(QString sourceFolder, QString destFolder)
{
    QDir sourceDir(sourceFolder);
    if(!sourceDir.exists())
        return;

    QDir destDir(destFolder);
    if(!destDir.exists())
        destDir.mkdir(destFolder);
    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        QFile::copy(srcName, destName);
    }
    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        copyFolder(srcName, destName);
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    undoStack_(nullptr),
    modifiedWithNoUndo_(false),
    ui_(new Ui::MainWindow),
    notesTextEdit_(nullptr),
    blastViewTab_(nullptr),
    primersViewTab_(nullptr),
    sequenceTextView_(nullptr),
    seqGraphicsScene_(nullptr),
    importFileDialog_(nullptr),
    saveAsDialog_(nullptr),
    sequenceImporter_(nullptr),
    adocModifiedBeforeNotes_(false),
    taskManagerWindow_(nullptr),
    taskManager_(nullptr),
    taskAdocConnector_(nullptr),
    blastDatabaseModel_(nullptr),
    blastDatabaseWindow_(nullptr),
    blastDialog_(nullptr),
    blastSequenceImporter_(nullptr),
    dnaSeqFeatureItemSynchronizer_(nullptr)
{
    Qt::SortOrder defaultSortOrder = Qt::AscendingOrder;

    connect(&adoc_, SIGNAL(modifiedChanged(bool)), SLOT(onModifiedChanged()));

    // -----------------------------------
    // The all important undo stack
    undoStack_ = new QUndoStack();
    connect(undoStack_, SIGNAL(cleanChanged(bool)), SLOT(onUndoStackCleanChanged(bool)));


    // -----------------------------------
    // Model and adapter setup
    adocTreeModel_ = new AdocTreeModel();
    adocTreeModel_->setUndoStack(undoStack_);

    containerModel_ = new AdocTreeNodeFilterModel(adocTreeModel_);  // Child of the AdocTreeModel instance
    containerModel_->setAcceptableNodeTypes(QSet<AdocNodeType>() << eRootNode << eGroupNode);
    containerModel_->setDynamicSortFilter(true);
    containerModel_->setSourceModel(adocTreeModel_);

    multiSeqTableModel_ = new MultiSeqTableModel(adocTreeModel_);   // Child of the AdocTreeModel instance
    multiSeqTableModel_->setDynamicSort(true);
    multiSeqTableModel_->sort(0, defaultSortOrder);
    multiSeqTableModel_->setUndoStack(undoStack_);
    multiSeqTableModel_->setSourceTreeModel(adocTreeModel_);
    connect(multiSeqTableModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onMultiSeqTableModelDataChanged(QModelIndex,QModelIndex)));

    aminoSeqColumnAdapter_ = new AminoSeqColumnAdapter(this);
    aminoSeqColumnAdapter_->setUndoStack(undoStack_);
    aminoMsaColumnAdapter_ = new AminoMsaColumnAdapter(this);
    aminoMsaColumnAdapter_->setUndoStack(undoStack_);
    blastReportColumnAdapter_ = new BlastReportColumnAdapter(this);
    blastReportColumnAdapter_->setUndoStack(undoStack_);
    dnaSeqColumnAdapter_ = new DnaSeqColumnAdapter(this);
    dnaSeqColumnAdapter_->setUndoStack(undoStack_);

    dnaMsaColumnAdapter_ = new DnaMsaColumnAdapter(this);
    dnaMsaColumnAdapter_->setUndoStack(undoStack_);
    transientTaskColumnAdapter_ = new TransientTaskColumnAdapter(this);
    transientTaskColumnAdapter_->setUndoStack(undoStack_);

    blastDatabaseModel_ = new BlastDatabaseModel(this);

    // Add an eraser service to keep tree model and repositories in sync
    new AdocTreeNodeEraserService(&adoc_, adocTreeModel_, this);

    blastSequenceImporter_ = new BlastSequenceImporter(this);
    blastSequenceImporter_->setAdoc(&adoc_);
    blastSequenceImporter_->setAdocTreeModel(adocTreeModel_);

    // -----------------------------------
    // UI setup
    ui_->setupUi(this);

    seqGraphicsScene_ = new LinearGraphicsScene(this);
    seqGraphicsScene_->setMaxPixelsPerUnit(12.);
    ui_->seqGraphicsView->hide();
    ui_->seqGraphicsView->setScene(seqGraphicsScene_);
    ui_->seqGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui_->seqGraphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    connect(ui_->seqGraphicsView, SIGNAL(wheelDelta(int,int,int)), SLOT(onSeqGraphicsViewWheelEvent(int)));

    blastDatabaseWindow_ = new BlastDatabaseWindow;
    try
    {
        blastDatabaseModel_->setBlastDbCmdPath(QCoreApplication::applicationDirPath() +
                                               QDir::separator() +
                                               constants::kBlastDbCmdRelativePath);
        blastDatabaseWindow_->setBlastDatabaseModel(blastDatabaseModel_);
    }
    catch (QString &error)
    {
        qDebug() << Q_FUNC_INFO << "Warning:" << error;
    }

    // -----------------------------------
    // Flex tab widget
    {
        const int notesTab = 0;
        const int blastTab = 1;
        const int seqTab = 2;
        const int primersTab = 3;

        // Ownership of these widgets is passed to the FlexTabWidget; yet we retain pointers to them for our own
        // purposees within this class.
        notesTextEdit_ = new QPlainTextEdit;
        ui_->flexTabWidget->registerWidget(notesTab, notesTextEdit_, "Notes");

        blastViewTab_ = new BlastViewTab;
        ui_->flexTabWidget->registerWidget(blastTab, blastViewTab_, "BLAST");
        blastViewTab_->setBlastDatabaseModel(blastDatabaseModel_);
        blastViewTab_->setBlastSequenceImporter(blastSequenceImporter_);
        blastViewTab_->setBlastDatabaseWindow(blastDatabaseWindow_);
        connect(multiSeqTableModel_, SIGNAL(rootChanged(QModelIndex)), blastViewTab_, SLOT(setDefaultImportIndex(QModelIndex)));

        primersViewTab_ = new PrimersViewTab(undoStack_);
        ui_->flexTabWidget->registerWidget(primersTab, primersViewTab_, "Primers");

        sequenceTextView_ = new SequenceTextView;
        sequenceTextView_->setFont(QFont("monospace"));
        ui_->flexTabWidget->registerWidget(seqTab, sequenceTextView_, "Sequence");

        ui_->flexTabWidget->setTabGroup(0, QVector<int>() << notesTab);
        ui_->flexTabWidget->setTabGroup(eBlastReportEntity, QVector<int>() << blastTab << notesTab);
        ui_->flexTabWidget->setTabGroup(eDnaSeqEntity, QVector<int>() << seqTab << primersTab << notesTab);
        ui_->flexTabWidget->setTabGroup(eAminoSeqEntity, QVector<int>() << seqTab << notesTab);

        ui_->flexTabWidget->setActiveTabGroup(0);
    }

    // Dna seq feature synchronizer
    dnaSeqFeatureItemSynchronizer_ = new DnaSeqFeatureItemSynchronizer(this);
    dnaSeqFeatureItemSynchronizer_->setDnaSeqPrimerModel(primersViewTab_->dnaSeqPrimerModel());

    // ----------------
    // Undo dock widget
    QDockWidget *undoHistoryDockWidget = new QDockWidget("Undo History (Shift+Ctrl+H)", this);
    undoHistoryDockWidget->setWidget(new QUndoView(undoStack_));
    addDockWidget(Qt::RightDockWidgetArea, undoHistoryDockWidget);
    undoHistoryDockWidget->setVisible(false);

    ui_->menu_View->addSeparator();
    QAction *toggleUndoHistoryAction = undoHistoryDockWidget->toggleViewAction();
    toggleUndoHistoryAction->setText("Undo History");
    toggleUndoHistoryAction->setShortcut(QKeySequence("Shift+Ctrl+H"));
    toggleUndoHistoryAction->setIcon(QIcon(":/aliases/images/icons/view-history"));
    ui_->menu_View->addAction(toggleUndoHistoryAction);


    // Give the folderPanel an initial 200 pixels, and 1 for the right side (it will expand as needed)
    ui_->folderDocumentSplitter->setSizes(QList<int>() << 200 << 1);

    ClipboardStateItemDelegate *clipboardStateItemDelegate = new ClipboardStateItemDelegate(this);
    ui_->treeView->setItemDelegateForColumn(0, clipboardStateItemDelegate);
    ui_->treeView->header()->setSortIndicator(0, defaultSortOrder);
    ui_->treeView->setAcceptDrops(true);
    ui_->treeView->setModel(containerModel_);
    connect(ui_->treeView, SIGNAL(clearCut()), adocTreeModel_, SLOT(clearCutCopyRows()));
    connect(ui_->treeView, SIGNAL(cut(QItemSelection)), SLOT(onTreeViewCut(QItemSelection)));
    connect(ui_->treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onTreeViewContextMenuRequested(QPoint)));
    connect(ui_->treeView, SIGNAL(deleteSelection(QItemSelection)), SLOT(onTreeViewDeleteSelection(QItemSelection)));
    connect(ui_->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onTreeViewCurrentChanged(QModelIndex,QModelIndex)));
    connect(ui_->treeView, SIGNAL(pasteTo(QModelIndex)), SLOT(onTreeViewPasteTo(QModelIndex)));

    ui_->tableView->setModel(multiSeqTableModel_);
    ui_->tableView->setItemDelegateForColumn(multiSeqTableModel_->groupLabelColumn(), clipboardStateItemDelegate);
    SpinBoxDelegate *spinBoxDelegate = new SpinBoxDelegate(this);
    ui_->tableView->setItemDelegateForColumn(MultiSeqTableModel::eStartColumn, spinBoxDelegate);
    ui_->tableView->setItemDelegateForColumn(MultiSeqTableModel::eStopColumn, spinBoxDelegate);
    ui_->tableView->setAcceptDrops(true);
    ui_->tableView->horizontalHeader()->setSortIndicator(0, defaultSortOrder);
    ui_->tableView->horizontalHeader()->resizeSection(MultiSeqTableModel::eStartColumn, 50);
    ui_->tableView->horizontalHeader()->resizeSection(MultiSeqTableModel::eStopColumn, 50);
    ui_->tableView->horizontalHeader()->resizeSection(MultiSeqTableModel::eLengthColumn, 50);
    ui_->tableView->horizontalHeader()->setMovable(true);
    ui_->tableView->hideColumn(MultiSeqTableModel::eIdColumn);
    ui_->tableView->hideColumn(MultiSeqTableModel::eNotesColumn);
    ui_->tableView->hideColumn(MultiSeqTableModel::eSequenceColumn);
    ui_->tableView->hideColumn(MultiSeqTableModel::eStartColumn);
    ui_->tableView->hideColumn(MultiSeqTableModel::eStopColumn);
    HeaderColumnSelector *selector = new HeaderColumnSelector(ui_->tableView->horizontalHeader(), ui_->tableView);
    selector->setDefaultColumns(QVector<int>() << MultiSeqTableModel::eNameColumn
                                << MultiSeqTableModel::eEntityTypeColumn
                                << MultiSeqTableModel::eSourceColumn
                                << MultiSeqTableModel::eLengthColumn
                                << MultiSeqTableModel::eDescriptionColumn);

    connect(ui_->tableView, SIGNAL(clearCut()), adocTreeModel_, SLOT(clearCutCopyRows()));
    connect(ui_->tableView, SIGNAL(cut(QItemSelection)), SLOT(onTableViewCut(QItemSelection)));
    connect(ui_->tableView, SIGNAL(deleteSelection(QItemSelection)), SLOT(onTableViewDeleteSelection(QItemSelection)));
    connect(ui_->tableView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onTableViewDoubleClicked(QModelIndex)));
    connect(ui_->tableView, SIGNAL(enterPressed(QModelIndex)), SLOT(onTableViewEnterPressed(QModelIndex)));
    connect(ui_->tableView, SIGNAL(pasteTo(QModelIndex)), SLOT(onTableViewPasteTo(QModelIndex)));
    connect(ui_->tableView->model(), SIGNAL(layoutChanged()), SLOT(onTableViewLayoutChanged()));
    connect(ui_->tableView->model(), SIGNAL(modelAboutToBeReset()), SLOT(onTableViewModelAboutToReset()));
    connect(ui_->tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onTableViewCurrentRowChanged(QModelIndex,QModelIndex)));
    connect(ui_->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onTableViewSelectionChanged()));

    connect(notesTextEdit_, SIGNAL(undoAvailable(bool)), SLOT(onTextEditUndoAvailable(bool)));



    // -----------------------------------
    // Actions
    connect(ui_->action_NewFolder, SIGNAL(triggered()), SLOT(onActionNewFolder()));
    connect(ui_->action_NewSequence, SIGNAL(triggered()), SLOT(onActionNewSequence()));
    connect(ui_->action_New, SIGNAL(triggered()), SLOT(newDocument()));
    connect(ui_->action_Open, SIGNAL(triggered()), SLOT(onActionOpen()));
    connect(ui_->action_Revert, SIGNAL(triggered()), SLOT(onActionRevert()));
    connect(ui_->action_Save, SIGNAL(triggered()), SLOT(save()));
    connect(ui_->action_SaveAs, SIGNAL(triggered()), SLOT(saveAs()));
    connect(ui_->action_Import, SIGNAL(triggered()), SLOT(onActionImport()));
    connect(ui_->action_Exit, SIGNAL(triggered()), SLOT(close()));
    connect(ui_->action_Undo, SIGNAL(triggered()), SLOT(onActionUndo()));
    connect(ui_->action_Redo, SIGNAL(triggered()), undoStack_, SLOT(redo()));
    connect(ui_->action_Cut, SIGNAL(triggered()), SLOT(onActionCut()));
    connect(ui_->action_Paste, SIGNAL(triggered()), SLOT(onActionPaste()));
    connect(ui_->action_Delete, SIGNAL(triggered()), SLOT(onActionDelete()));
    connect(ui_->actionBlast, SIGNAL(triggered()), SLOT(onActionBlast()));
    connect(ui_->actionAlign, SIGNAL(triggered()), SLOT(onActionAlign()));
    connect(ui_->action_BlastDatabases, SIGNAL(triggered()), SLOT(onActionBlastDatabaseManager()));
    connect(ui_->action_TaskManager, SIGNAL(triggered()), SLOT(onActionTaskManager()));
    connect(ui_->actionAboutAlignShop, SIGNAL(triggered()), SLOT(onActionAboutAlignShop()));
    connect(ui_->action_License, SIGNAL(triggered()), SLOT(showLicense()));
    connect(undoStack_, SIGNAL(canUndoChanged(bool)), ui_->action_Undo, SLOT(setEnabled(bool)));
    connect(undoStack_, SIGNAL(canRedoChanged(bool)), ui_->action_Redo, SLOT(setEnabled(bool)));
//    connect(undoStack_, SIGNAL(canUndoChanged(bool)), ui_->action_Save, SLOT(setEnabled(bool)));

    // Sync the table sort order with the multiSeqTable's internal sorting order; it is important to do this after
    // the setupUi method call otherwise, the ui objects will be uninitialized
    multiSeqTableModel_->sort(0, ui_->tableView->horizontalHeader()->sortIndicatorOrder());


    // -----------------------------------
    // Focus lost events for the text edit
    WidgetFocusObserver *widgetFocusObserver = new WidgetFocusObserver(notesTextEdit_, this);
    connect(widgetFocusObserver, SIGNAL(focusLost(Qt::FocusReason)), SLOT(onTextEditFocusLost()));



    // -----------------------------------
    // The following configures the ModelTest to drill our models for correctness.
#if defined QT_DEBUG && defined Q_OS_LINUX
    new ModelTest(adocTreeModel_, this);
    new ModelTest(containerModel_, this);
    new ModelTest(multiSeqTableModel_, this);
#endif


    // ------------------------------
    // Setup the task manager
    taskManager_ = new TaskManager();
    taskManager_->setMaxThreads(constants::kNumberOfCores);
    taskAdocConnector_ = new TaskAdocConnector(taskManager_, &adoc_, adocTreeModel_, transientTaskColumnAdapter_, this);


    // -----------------------------------
    // Finally load our persistent settings
    readPersistentSettings();

    // -----------------------------------
    // Create a new document from scratch
    newDocument();
//    openDocument("/home/ulrich/ExampleProject.ap");
}

/**
  */
MainWindow::~MainWindow()
{
    delete blastDatabaseWindow_;
    delete taskManagerWindow_;

    // Order of deletion is critical here!

    // These may have shared references with a repository. Therefore, they are de-allocated before the repositories
    // which occurs when the adoc_ member is popped off the stack.
    seqGraphicsScene_->clear();
    delete ui_->flexTabWidget;      // To clear any shared pointers these may contain
    delete taskManager_;

    // Because some of the commands on the undoStack may reference the adocTreeModel in their destructors, it is vital
    // to release the undoStack *before* adocTreeModel_.
    delete undoStack_;
    delete adocTreeModel_;

    // Moreover, we need to delete the ui after the undo stack because in response to some actions, the ui accessed and if it
    // is deleted first, then a seg fault will occur.
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString MainWindow::titleString() const
{
    QString text = QFileInfo(adoc_.fileName()).fileName();
    if (adoc_.isModified() || adoc_.isTemporary())
        text += "*";
    text += " - AlignShop";
    return text;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * Currently limited to editing within the treeview
  */
void MainWindow::createGroupAndStartEditing(const QModelIndex &parentIndex, const QString &seedText)
{
    ASSERT(adoc_.isOpen());
    ASSERT(parentIndex.isValid() == false || parentIndex.model() == adocTreeModel_);

    QModelIndex groupIndex = containerModel_->mapFromSource(adocTreeModel_->newGroup(seedText, parentIndex));
    ui_->treeView->setCurrentIndex(groupIndex);
    ui_->treeView->edit(groupIndex);
}

/**
  */
void MainWindow::newDocument()
{
    if (adoc_.isOpen())
    {
        if (adoc_.isModified())
        {
            switch (askUserToSaveChanges())
            {
            case QMessageBox::Save:
                save();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
                return;
            }
        }

        undoStack_->clear();

        // Trigger the release of any allocated data by setting the root of the tree model to null
        adocTreeModel_->setRoot(nullptr);

        // Finally close out the document
        adoc_.close();
    }

    if (adoc_.create())
    {
        updateEntityAdapterSpecifications();

        adocTreeModel_->setRoot(adoc_.entityTree());
        adocTreeModel_->newGroup("Proteins", QModelIndex());
        adocTreeModel_->newGroup("Genes", QModelIndex());
        undoStack_->clear();    // So we don't see the nodes we just appended here in the undo/redo stack

        setWindowTitle(titleString());
    }
}

/**
  * @param fileName [const QString &]
  */
void MainWindow::openDocument(const QString &fileName)
{
    if (adoc_.isOpen())
    {
        // If we have opened the same file, simple do nothing
        if (QFileInfo(fileName) == QFileInfo(adoc_.fileName()))
            return;

        if (adoc_.isModified())
        {
            switch (askUserToSaveChanges())
            {
            case QMessageBox::Save:
                save();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
                return;
            }
        }

        undoStack_->clear();
        adocTreeModel_->setRoot(nullptr);
        adoc_.close();
    }

    if (adoc_.open(fileName))
    {
        adocTreeModel_->setRoot(adoc_.entityTree());
        updateEntityAdapterSpecifications();
        setWindowTitle(titleString());
        containerModel_->sort(0);
    }
    else
    {
        QMessageBox::warning(this, "Invalid AlignShop project file", QString("%1 is not a valid AlignShop Project file."
                                                                             " Please select another file").arg(fileName), QMessageBox::Ok);
    }
}

/**
  * @returns bool
  */
bool MainWindow::save()
{
    if (!adoc_.isTemporary() && adoc_.isModified() == false)
        return true;

    // Capture any changed notes in the notes text editor
    if (ui_->tableView->currentIndex().isValid())
        updateEntityNotes(ui_->tableView->currentIndex());

    bool saveSuccessful = false;
    if (adoc_.isTemporary() == false)
    {
        setCursor(Qt::WaitCursor);
        // The undo stack is cleared **before** the save call so that the command destruction process can issue notices
        // through the AdocTreeModel of any outstanding nodes that are no longer referenced. This prevents potential
        // "dangling entity" warnings.
        //
        // Hmm... 8 Nov 2011 - moved clearing the undo stack after the save and I don't see the dangling entity warning
        // mentioned previously. Not sure why not. But this latter approach is definitely preferred and makes for more
        // reasonable handling with regard to things like cleaning up blast reports.
        saveSuccessful = adoc_.save();
        if (saveSuccessful)
        {
            modifiedWithNoUndo_ = false;
            undoStack_->clear();
        }

        setCursor(QCursor());
    }
    else
    {
        // We do not immediately clear the undoStack here because the user might choose to cancel saving as a different
        // filename. If he cancels, it is desirable to have the undoStack still present.
        saveSuccessful = saveAs();
    }

    multiSeqTableModel_->refreshEntityIds();

    return saveSuccessful;
}

/**
  * @returns bool
  */
bool MainWindow::saveAs()
{
    // Dynamically create the save as dialog if needed
    if (saveAsDialog_ == nullptr)
    {
        saveAsDialog_ = new QFileDialog(this, "Save File As", QString(), "AlignShop project (*.ap);;All files (*)");
        saveAsDialog_->setAcceptMode(QFileDialog::AcceptSave);
        saveAsDialog_->setDefaultSuffix("ap");
    }

    bool saveSuccessful = false;
    while (1)
    {
        if (saveAsDialog_->exec() != QFileDialog::AcceptSave)
            break;

        QString targetFileName = saveAsDialog_->selectedFiles().first();
        if (targetFileName == adoc_.fileName())
        {
            // User selected the same file, reject this and try again
            QMessageBox::warning(this, "Save As Error", "Save as must utilize a different filename", QMessageBox::Ok);
            continue;
        }

        // Make sure that the target file name does not have a data directory
        QString targetDataPath = targetFileName;
        if (targetDataPath.endsWith(".ap"))
            targetDataPath.remove(targetDataPath.length() - 3, 3);
        targetDataPath += adoc_.dataPathSuffix();

        QDir targetDataDir = QDir(targetDataPath);
        if (targetDataDir.exists())
        {
            QMessageBox::warning(this,
                                 "Save As Error",
                                 QString("The directory, %1, already exists which conflicts with the data directory "
                                         "corresponding to the selected file name. Please input a different filename "
                                         "and try again.").arg(targetDataDir.path()),
                                 QMessageBox::Ok);
            continue;
        }

        QString existingDataPath = adoc_.dataPath();

        setCursor(Qt::WaitCursor);
        // Now that the user has confirmed a different file name to save as, go ahead and clear the undo stack
        bool saveCoreFileSuccessful = adoc_.saveAs(saveAsDialog_->selectedFiles().first());
        if (!saveCoreFileSuccessful)
        {
            QMessageBox::warning(this, "Save As Error", "Unable to save file", QMessageBox::Ok);
            break;
        }

        undoStack_->clear();
        setWindowTitle(titleString());

        // Now, attempt to copy any existing data directory
        QDir existingDataDirectory(existingDataPath);
        if (existingDataPath.isEmpty() || !existingDataDirectory.exists())
        {
            saveSuccessful = true;
            break;
        }

        copyFolder(existingDataDirectory.absolutePath(), targetDataDir.absolutePath());

        saveSuccessful = true;
        break;
    }

    setCursor(QCursor());
    return saveSuccessful;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param closeEvent [QCloseEvent *]
  */
void MainWindow::closeEvent(QCloseEvent *closeEvent)
{
    // Close any open MsaWindows
    foreach (MsaWindow *msaWindow, msaWindows_.values())
    {
        if (!msaWindow->close())
        {
            closeEvent->ignore();
            return;
        }
    }

    if (adoc_.isOpen() && adoc_.isModified())
    {
        switch (askUserToSaveChanges())
        {
        case QMessageBox::Save:
            save();

            // ISSUE: There is no straightforward way (with the current setup) of checking if the user completed the
            // save (in the case of saveAs especially) or if he cancelled out of it. Thus, a workaround is to check if
            // it is no longer modified. If it is still modified, we can assume for now that the user cancelled (or
            // there was some other problem :\).
            if (adoc_.isModified() == false)
                break;

            // If we get here, then we are assuming the user cancelled out of the save process
            closeEvent->ignore();
            return;
        case QMessageBox::Cancel:
            closeEvent->ignore();
            return;
        case QMessageBox::Discard:
        default:
            break;
        }
    }

    emit aboutToClose();
    closeEvent->accept();

    // To purge any remaining data file remnants
    while (!undoStack_->isClean())
        undoStack_->undo();

    writePersistentSettings();

    // Close down entire application and do not wait for all windows to close.
    QApplication::quit();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void MainWindow::onActionNewSequence()
{
    DnaSeqFactory dnaSeqFactory(adoc_.dstringRepository());
    AminoSeqFactory aminoSeqFactory(adoc_.astringRepository());
    DynamicSeqFactory dynamicSeqFactory;
    dynamicSeqFactory.setSeqFactoryForGrammar(eDnaGrammar, &dnaSeqFactory);
    dynamicSeqFactory.setSeqFactoryForGrammar(eAminoGrammar, &aminoSeqFactory);
    NewSequenceEntityDialog dialog(&dynamicSeqFactory, this);
    if (!dialog.exec())
        return;

    // Create the entity!
    IEntitySPtr entity = dialog.sequenceEntity();
    IRepository *repository = adoc_.repository(entity);
    if (!repository->addGeneric(entity, false))
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error creating sequence");
        msgBox.setText("There was an error when adding the new sequence to the repository. Please try again.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    // Create an AdocTreeNode for this entity
    AdocTreeNode *node = new AdocTreeNode(entity);
    if (!adocTreeModel_->appendRows(AdocTreeNodeVector() << node, containerModel_->mapToSource(ui_->treeView->currentIndex())))
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error adding sequence");
        msgBox.setText("An error occurred while adding your sequence to the data tree. Please try again.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        delete node;
        return;
    }
}

void MainWindow::onActionNewFolder()
{
    createGroupAndStartEditing(containerModel_->mapToSource(ui_->treeView->currentIndex()));
}

/**
  */
void MainWindow::onActionImport()
{
    if (sequenceImporter_ == nullptr)
    {
        sequenceImporter_ = new SequenceImporter(this);
        connect(sequenceImporter_, SIGNAL(importError(QString)), SLOT(onImportError(QString)));
        connect(sequenceImporter_, SIGNAL(importSuccessful(QModelIndex)), SLOT(onImportSuccessful(QModelIndex)));
    }

    // Get a file from the user
    if (importFileDialog_ == nullptr)
    {
        QStringList nameFilters = DataFormat::nameFilters(sequenceImporter_->dataFormats());
        nameFilters << "Any file (*)";

        importFileDialog_ = new QFileDialog(this, "Select file to import");
        importFileDialog_->setFileMode(QFileDialog::ExistingFile);
        importFileDialog_->setNameFilters(nameFilters);
    }

    if (importFileDialog_->exec() == QFileDialog::Accepted)
    {
        QModelIndex treeIndex = containerModel_->mapToSource(ui_->treeView->currentIndex());
        sequenceImporter_->importFile(importFileDialog_->selectedFiles().first(), &adoc_, adocTreeModel_, treeIndex);
    }
}

/**
  */
void MainWindow::onActionOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file", "", "AlignShop project (*.ap);;Any file (*)");
    if (fileName.isEmpty())
        return;

    openDocument(fileName);
}

/**
  */
void MainWindow::onActionRevert()
{
    ASSERT(adoc_.isOpen());
    ASSERT(adoc_.isTemporary() == false);

    if (adoc_.isModified())
    {
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Are you sure you want to revert all changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Yes)->setText("Revert Changes");
        msgBox.setDefaultButton(QMessageBox::Cancel);
        switch (msgBox.exec())
        {
        case QMessageBox::Cancel:
            return;

        default:
            break;
        }
    }

    QString originalFileName = adoc_.fileName();
    if (!QFileInfo(originalFileName).exists())
    {
        QMessageBox::warning(this, "Original file not found",
                             "Unable to revert to original file because it is not on the filesystem",
                             QMessageBox::Ok);
        return;
    }

    // Undo all the actions as a simple way to remove any associated files
    while (!undoStack_->isClean())
        undoStack_->undo();
    undoStack_->clear();
    adocTreeModel_->setRoot(nullptr);
    adoc_.close();

    openDocument(originalFileName);
}

/**
  * Because some commands are conditional, it is necessary to confirm that the undo should continue. This is a two-step
  * process:
  * 1) First check if the command to be undone is a ConditionalUndoCommand (via dynamic_cast)
  * 2) Perform the undo if it is not or if acceptUndo() returns true
  */
void MainWindow::onActionUndo()
{
    if (undoStack_->index() == 0)
        return;

    const QUndoCommand *command = undoStack_->command(undoStack_->index() - 1);
    const ConditionalUndoCommand *conditionalCommand = dynamic_cast<const ConditionalUndoCommand *>(command);
    if (!conditionalCommand || conditionalCommand->acceptUndo())
        undoStack_->undo();
}

void MainWindow::onActionCut()
{
    onTableViewCut(ui_->tableView->selectionModel()->selection());
}

void MainWindow::onActionPaste()
{
    onTableViewPasteTo(ui_->tableView->currentIndex());
}

void MainWindow::onActionDelete()
{
    onTableViewDeleteSelection(ui_->tableView->selectionModel()->selection());
}

/**
  */
void MainWindow::onActionAlign()
{
    QModelIndexList selectedIndexes = ui_->tableView->selectionModel()->selectedRows();
    ASSERT(selectedIndexes.size() > 1);

    // Create an in-memory copy of the input sequences
    QVector<AbstractSeqSPtr> inputSequences;
    foreach (const QModelIndex &index, selectedIndexes)
        inputSequences << AbstractSeqSPtr(static_cast<AbstractSeq *>(multiSeqTableModel_->entityFromIndex(index)->clone()));

    // Extract grammar from first sequence
    Grammar grammar = inputSequences.first()->abstractAnonSeq()->seq_.grammar();
    AbstractMsaBuilder *msaBuilder = nullptr;
    if (grammar == eAminoGrammar)
        msaBuilder = getAminoMsaBuilderFromUser();
    else if (grammar == eDnaGrammar)
        msaBuilder = getDnaMsaBuilderFromUser();
    if (!msaBuilder)
        return;

    // A. Create the task - ownership gets passwed to the TaskManager's TaskModel when it is enqueued
    BuildMsaTask *task = new BuildMsaTask("Build MSA", msaBuilder, inputSequences);

    // B. Create the task entity
    TransientTaskSPtr taskEntity = boost::make_shared<TransientTask>("New alignment", task);
    adoc_.transientTaskRepository()->add(taskEntity, false);

    // C. Create the corresponding tree node
    AdocTreeNode *msaNode = new AdocTreeNode(taskEntity);

    // D. Hook up the signals
    taskAdocConnector_->add(task, msaNode);

    // E. Add to the tree
    AdocTreeNode *parentNode = multiSeqTableModel_->root();
    QUndoCommand *command = new InsertTaskNodesCommand(adocTreeModel_, msaNode, parentNode, taskManager_, this);
    undoStack_->push(command);

    // F. Kick off the task
    taskManager_->enqueue(new TaskTreeNode(task));
}

/**
  * Currently only supports BLASTing amino sequences
  */
void MainWindow::onActionBlast()
{
    ASSERT(canBlastSelection());

    // Check that the current file is not a temporary file
    if (adoc_.isTemporary())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Please save before BLASTing");
        msgBox.setText("Because all BLAST searches are stored as external files relative to the main document, "
                       "it is not possible to execute BLAST if it has not been previously saved to the filesystem. "
                       "Would you like to save and continue?");
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Save);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;

        // Otherwise, save was pressed, attempt to save and if successful, then continue
        if (!save())
            return;

        // The save was successful, continue on with the BLAST
        ASSERT(adoc_.isTemporary() == false);
    }

    // ---------------------------------------------------
    // Check that at least one amino blast database exists
    QVector<QPersistentModelIndex> blastDatabaseIndices = blastDatabaseModel_->aminoBlastDatabases();
    if (blastDatabaseIndices.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("BLAST configuration needed");
        msgBox.setText("No protein BLAST databases have been configured. You must set one up in the BLAST "
                       "database manager before you may run BLAST.");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Open)->setText("Open BLAST Database Manager");
        if (msgBox.exec() == QMessageBox::Open)
        {
            blastDatabaseWindow_->show();
            blastDatabaseWindow_->raise();
            blastDatabaseWindow_->activateWindow();
        }

        return;
    }

    // Pick out the relevant sequences and instantiate the blast window!
    if (blastDialog_ == nullptr)
        blastDialog_ = new BlastDialog(this);

    blastDialog_->setMaxThreads(taskManager_->maxThreads());

    // --------------------------------------
    // Insert sequences into the table widget
    QModelIndexList selectedIndexes = ui_->tableView->selectionModel()->selectedRows();
    ASSERT(selectedIndexes.size() > 0);

    QTableWidget *tableWidget = blastDialog_->sequenceTableWidget();
    tableWidget->setRowCount(selectedIndexes.size());
    int i = 0;
    foreach (const QModelIndex &index, selectedIndexes)
    {
        QModelIndex nameIndex = index.sibling(index.row(), MultiSeqTableModel::eNameColumn);
        ASSERT(nameIndex.isValid());
        QTableWidgetItem *nameItem = new QTableWidgetItem(nameIndex.data().toString());
        tableWidget->setItem(i, 0, nameItem);

        QModelIndex sequenceIndex = index.sibling(index.row(), MultiSeqTableModel::eSequenceColumn);
        ASSERT(sequenceIndex.isValid());
        QTableWidgetItem *sequenceItem = new QTableWidgetItem(sequenceIndex.data().toString());
        tableWidget->setItem(i, 1, sequenceItem);

        ++i;
    }
    int nSequences = i;

    // -----------------------------
    // Update the list of BLAST databases
    blastDialog_->setBlastDatabaseIndices(blastDatabaseIndices);

    if (!blastDialog_->exec())
        return;

    // Essentially creating 2 hierarchies here:
    // 1) The AdocTreeNode hierarchy
    // 2) The Task hierarchy
    TaskTreeNode *masterTaskNode = nullptr;
    AdocTreeNode *parentNode = multiSeqTableModel_->root();
    ConditionalUndoCommand *masterCommand = nullptr;
    QString commandText = "BLAST ";
    // Kick off a BLAST task!
    if (nSequences > 1)
    {
        masterCommand = new ConditionalUndoCommand(QString("BLAST %1 sequences").arg(nSequences));

        // Create a folder for this batch of sequences
        AdocTreeNode *blastGroup = new AdocTreeNode(eGroupNode, "BLASTs");
        new InsertAdocTreeNodesCommand(adocTreeModel_, blastGroup, parentNode, masterCommand);
        parentNode = blastGroup;

        masterTaskNode = new TaskTreeNode(new Task(Ag::Group, "Batch BLAST job"));

        commandText += QString("%1 sequences").arg(nSequences);
    }
    else
    {
        ASSERT(nSequences == 1);

        QModelIndex index = selectedIndexes.first();
        commandText += index.sibling(index.row(), MultiSeqTableModel::eNameColumn).data().toString();
    }

    AdocTreeNodeVector blastTreeNodes;
    foreach (const QModelIndex &index, selectedIndexes)
    {
        QModelIndex nameIndex =     index.sibling(index.row(), MultiSeqTableModel::eNameColumn);
        QModelIndex idColumn =      index.sibling(index.row(), MultiSeqTableModel::eIdColumn);
        QModelIndex startColumn =   index.sibling(index.row(), MultiSeqTableModel::eStartColumn);
        QModelIndex stopColumn =    index.sibling(index.row(), MultiSeqTableModel::eStopColumn);
        QModelIndex sequenceIndex = index.sibling(index.row(), MultiSeqTableModel::eSequenceColumn);
        ASSERT(nameIndex.isValid());
        ASSERT(idColumn.isValid());
        ASSERT(startColumn.isValid());
        ASSERT(stopColumn.isValid());
        ASSERT(sequenceIndex.isValid());

        // A) Create the task - ownership ultimately gets passed to the TaskManager's taskModel when it is enqueued
        BlastTask *blastTask = new BlastTask(idColumn.data().toInt(),
                                             BioString(sequenceIndex.data().toByteArray(), eAminoGrammar),
                                             ClosedIntRange(startColumn.data().toInt(), stopColumn.data().toInt()),
                                             blastDialog_->blastOptions(),
                                             adoc_.blastDataPath(),
                                             QString("BLAST %1").arg(nameIndex.data().toString()),
                                             blastDatabaseModel_->specFor(blastDialog_->blastDatabaseIndex()));
        blastTask->setMaxThreads(blastDialog_->nThreads());
        if (nSequences > 1)
            masterTaskNode->appendChild(new TaskTreeNode(blastTask));
        else
            masterTaskNode = new TaskTreeNode(blastTask);

        // B) Create the task entity
        TransientTaskSPtr taskEntity = boost::make_shared<TransientTask>(nameIndex.data().toString() + " BLAST report", blastTask);
        adoc_.transientTaskRepository()->add(taskEntity, false);

        // C) Create the corresponding tree node
        AdocTreeNode *blastReportNode = new AdocTreeNode(taskEntity);
        blastTreeNodes << blastReportNode;

        // D) Hook up the signals
        taskAdocConnector_->add(blastTask, blastReportNode);
    }

    ASSERT(blastTreeNodes.size() > 0);

    // Create the nodes command
    QUndoCommand *command = new InsertBlastReportsCommand(adocTreeModel_, blastTreeNodes, parentNode, taskManager_, this, masterCommand);
    if (masterCommand != nullptr)
    {
        masterCommand->setText(commandText);
        undoStack_->push(masterCommand);
    }
    else
    {
        command->setText(commandText);
        undoStack_->push(command);
    }

    // Finally, kick off the task
    taskManager_->enqueue(masterTaskNode);
}

/**
  */
void MainWindow::onActionBlastDatabaseManager()
{
    blastDatabaseWindow_->show();
    blastDatabaseWindow_->activateWindow();
    blastDatabaseWindow_->raise();
}

/**
  */
void MainWindow::onActionTaskManager()
{
    if (taskManagerWindow_ == nullptr)
    {
        // Do not set treeView's parent because we do not want it to be a child widget
        QTreeView *treeView = new QTreeView;
        treeView->setModel(taskManager_->taskModel());
    //    treeView->setAttribute(Qt::WA_DeleteOnClose);
        treeView->setWindowTitle("Task manager");
        treeView->setWindowIcon(QIcon(":/aliases/images/icons/task-manager"));
        taskManagerWindow_ = treeView;
    }

    taskManagerWindow_->show();
    taskManagerWindow_->activateWindow();
    taskManagerWindow_->raise();
}

void MainWindow::onActionAboutAlignShop()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void MainWindow::showLicense()
{
    LicenseInfoDialog licenseDialog(this);
    licenseDialog.exec();
}


// -------------------------------
// -------------------------------
// TreeView reaction slots
/**
  * @param position [const QPoint &]
  */
void MainWindow::onTreeViewContextMenuRequested(const QPoint &position)
{
    QPoint globalPosition = ui_->treeView->viewport()->mapToGlobal(position);
    QMenu menu;
    menu.addActions(ui_->menuNew->actions());
    menu.exec(globalPosition);
}

/**
  * Update the current table view root with the currently highlighted node.
  *
  * @param current [const QModelIndex &]
  * @param previous [const QModelIndex &]
  */
void MainWindow::onTreeViewCurrentChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    ui_->action_NewSequence->setEnabled(current.isValid());
    ASSERT(current.isValid() == false || current.model() == containerModel_);

    multiSeqTableModel_->setRoot(containerModel_->mapToSource(current));
}

/**
  * @param selection [const QItemSelection &]
  */
void MainWindow::onTreeViewCut(const QItemSelection &selection)
{
    ASSERT(adocTreeModel_ != nullptr);
    adocTreeModel_->cutRows(containerModel_->mapSelectionToSource(selection).indexes());
}

/**
  * @param treeSelection [const QItemSelection &]
  */
void MainWindow::onTreeViewDeleteSelection(const QItemSelection &treeSelection)
{
    deleteAdocTreeIndices(containerModel_->mapSelectionToSource(treeSelection).indexes());
}

/**
  * @param index [const QModelIndex &]
  */
void MainWindow::onTreeViewPasteTo(const QModelIndex &index)
{
    ASSERT(adocTreeModel_ != nullptr);
    adocTreeModel_->paste(containerModel_->mapToSource(index));
}


// -------------------------------
// -------------------------------
// TableView reaction slots
/**
  * Update the currently selected entity.
  *
  * @param current [const QModelIndex &]
  * @param previous [const QModelIndex &]
  */
void MainWindow::onTableViewCurrentRowChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    enableDisableCutPasteDeleteActions();

    blastViewTab_->setBlastReport(BlastReportSPtr());
    ui_->seqGraphicsView->hide();
    seqGraphicsScene_->clear();

    activeEntityIndex_ = current;
    adocModifiedBeforeNotes_ = adoc_.isModified();
    notesTextEdit_->clear();
    if (current.isValid() == false || multiSeqTableModel_->isGroupIndex(current))
    {
        ui_->flexTabWidget->setActiveTabGroup(0);

        activeEntityIndex_ = QModelIndex();
        notesTextEdit_->setEnabled(false);
        return;
    }

    // Safe to assume that this index is an entity because the MultiSeqTableModel only contains groups and entities
    notesTextEdit_->setEnabled(true);
    QModelIndex notesIndex = multiSeqTableModel_->index(current.row(), MultiSeqTableModel::eNotesColumn);
    notesTextEdit_->setPlainText(notesIndex.data().toString());

    // If this is a blast report, then attempt to read it into memory
    const IEntitySPtr &entity = multiSeqTableModel_->entityFromIndex(current);
    if (entity->type() == eBlastReportEntity)
    {
        const BlastReportSPtr &blastReport = boost::static_pointer_cast<BlastReport>(entity);
        blastViewTab_->setBlastReport(blastReport);
        ui_->flexTabWidget->setActiveTabGroup(eBlastReportEntity);
    }
    else
    {
        if (entity->type() == eDnaSeqEntity)
            primersViewTab_->setDnaSeq(boost::shared_static_cast<DnaSeq>(entity));

        if (entity->type() == eAminoSeqEntity ||
            entity->type() == eDnaSeqEntity)
        {
            ui_->seqGraphicsView->show();

            AbstractSeqItem *seqItem = nullptr;
            if (entity->type() == eAminoSeqEntity)
                seqItem = new AminoSeqItem(boost::static_pointer_cast<AminoSeq>(entity), aminoSeqColumnAdapter_);
            else // if (entity->type() == eDnaSeqEntity)
                seqItem = new DnaSeqItem(boost::static_pointer_cast<DnaSeq>(entity), dnaSeqColumnAdapter_);

            RangeHandlePairItem *rangeHandles = new RangeHandlePairItem(seqItem, 10);
            SeqBioStringItem *seqBioString = new SeqBioStringItem(seqItem, QFont("Monospace"));
            rangeHandles->setCoreStemHeight(seqBioString->height());
            connect(seqBioString, SIGNAL(heightChanged(double)), rangeHandles, SLOT(setCoreStemHeight(double)));
            seqGraphicsScene_->addSeqBioStringItem(seqBioString);

            LinearRulerItem *linearRulerItem = new LinearRulerItem(seqItem->length());
            seqGraphicsScene_->addLinearItem(linearRulerItem);
            linearRulerItem->setParentItem(seqBioString);
            linearRulerItem->setPos(seqBioString->x(),
                                    rangeHandles->startHandle()->y() - linearRulerItem->height() - 10);

            seqGraphicsScene_->setSceneRect(seqGraphicsScene_->paddedItemsBoundingRect(kDefaultSeqSceneVerticalPadding, kDefaultSeqSceneHorizontalPadding));

            const AbstractSeqSPtr abstractSeq = boost::static_pointer_cast<AbstractSeq>(entity);
            sequenceTextView_->setSequence(abstractSeq->bioString().asByteArray());

            ui_->flexTabWidget->setActiveTabGroup(entity->type());

            if (entity->type() == eDnaSeqEntity)
                dnaSeqFeatureItemSynchronizer_->setDnaSeqItem(static_cast<DnaSeqItem *>(seqItem));
        }
        else
        {
            ui_->flexTabWidget->setActiveTabGroup(0);
        }
    }
}

/**
  * @param tableSelection [const QItemSelection &]
  */
void MainWindow::onTableViewCut(const QItemSelection &tableSelection)
{
    ASSERT(adocTreeModel_ != nullptr);
    adocTreeModel_->cutRows(multiSeqTableModel_->mapSelectionToTree(tableSelection).indexes());
}

/**
  * @param tableSelection [const QItemSelection &]
  */
void MainWindow::onTableViewDeleteSelection(const QItemSelection &tableSelection)
{
    deleteAdocTreeIndices(multiSeqTableModel_->mapSelectionToTree(tableSelection).indexes());
}

/**
  * @param clickedIndex [const QModelIndex &]
  */
void MainWindow::onTableViewDoubleClicked(const QModelIndex &clickedIndex)
{
    // Ensure we have a valid index
    if (!clickedIndex.isValid())
        return;

    AdocTreeNode *node = multiSeqTableModel_->nodeFromIndex(clickedIndex);
    bool isMsa = node->nodeType_ == eAminoMsaNode ||
                 node->nodeType_ == eDnaMsaNode;
    if (isMsa)
    {
        openMsaWindowFromNode(node);
        return;
    }
    else if (node->nodeType_ != eGroupNode)
    {
        ui_->tableView->edit(clickedIndex);
        return;
    }

    // Obtain the tree model index for this node
    QModelIndex treeIndex = adocTreeModel_->indexFromNode(node);
    QModelIndex containerIndex = containerModel_->mapFromSource(treeIndex);

    // Runtime check to make sure that we reference a non-root node (root is not visible)
    ASSERT_X(containerIndex.isValid(), "mapped container index must be valid; check proxy model filtering rules");

    // Make sure the parent is expanded if it is not already
    ui_->treeView->expand(containerIndex.parent());

    // And select the new index
    ui_->treeView->setCurrentIndex(containerIndex);
}

/**
  * @param currentIndex [const QModelIndex &]
  */
void MainWindow::onTableViewEnterPressed(const QModelIndex &currentIndex)
{
    if (!currentIndex.isValid())
        return;

    AdocTreeNode *node = multiSeqTableModel_->nodeFromIndex(currentIndex);
    if (node->nodeType_ != eGroupNode)
        return;

    // Obtain the tree model index for this node
    QModelIndex treeIndex = adocTreeModel_->indexFromNode(node);
    QModelIndex containerIndex = containerModel_->mapFromSource(treeIndex);

    // Runtime check to make sure that we reference a non-root node (root is not visible)
    ASSERT_X(containerIndex.isValid(), "mapped container index must be valid; check proxy model filtering rules");

    // Make sure the parent is expanded if it is not already
    ui_->treeView->expand(containerIndex.parent());

    // And select the new index
    ui_->treeView->setCurrentIndex(containerIndex);
}

/**
  */
void MainWindow::onTableViewLayoutChanged()
{
    if (activeEntityIndex_.isValid())
    {
//        ui_->tableView->selectionModel()->select(currentEntityIndex_, QItemSelectionModel::ClearAndSelect);
        ui_->tableView->scrollTo(activeEntityIndex_);
    }
}

/**
  * This is used to update the notes editor when the table view model has been reset.
  */
void MainWindow::onTableViewModelAboutToReset()
{
    notesTextEdit_->clear();
    notesTextEdit_->setEnabled(false);

    // Release any used pointers
    ui_->seqGraphicsView->hide();
    seqGraphicsScene_->clear();
    blastViewTab_->setBlastReport(BlastReportSPtr());
    ui_->actionBlast->setEnabled(false);
    ui_->actionAlign->setEnabled(false);
    primersViewTab_->setDnaSeq(DnaSeqSPtr());

    ui_->flexTabWidget->setActiveTabGroup(0);
}

/**
  * @param tableIndex [const QModelIndex &]
  */
void MainWindow::onTableViewPasteTo(const QModelIndex &tableIndex)
{
    ASSERT(adocTreeModel_ != nullptr);
    adocTreeModel_->paste(multiSeqTableModel_->mapToTree(tableIndex));
}

/**
  * Note - we do not use the parameter versions of the selection changed, because this only includes the most recent
  * change. Instead, we pull the full selection from the tableview.
  */
void MainWindow::onTableViewSelectionChanged()
{
    ui_->actionAlign->setEnabled(canAlignSelection());
    ui_->actionBlast->setEnabled(canBlastSelection());
    enableDisableCutPasteDeleteActions();
}

// -------------------------------
// -------------------------------
// Other reaction slots
/**
  */
void MainWindow::onEntityStateExited()
{
    notesTextEdit_->clear();
    notesTextEdit_->setEnabled(false);
}

/**
  * @param errorMessage [const QString &]
  */
void MainWindow::onImportError(const QString &errorMessage)
{
    QMessageBox::warning(this, "Import error", errorMessage, QMessageBox::Ok);
}

/**
  * @param parentIndex [const QModelIndex &]
  */
void MainWindow::onImportSuccessful(const QModelIndex &parentIndex)
{
    if (parentIndex.isValid() == false)
        return;

    ASSERT(parentIndex.model() == adocTreeModel_);
    QModelIndex containerIndex = containerModel_->mapFromSource(parentIndex);
    ASSERT(containerIndex.isValid());
    ui_->treeView->setCurrentIndex(containerIndex);
}

/**
  */
void MainWindow::onModifiedChanged()
{
    setWindowTitle(titleString());
    if (adoc_.isModified() == false)
        update();

    ui_->action_Revert->setEnabled(!adoc_.isTemporary() && adoc_.isModified());
    ui_->action_Save->setEnabled(adoc_.isTemporary() || adoc_.isModified());
}

/**
  * Special case: if the Msa has been imported but not yet saved, then it is imperative that it is not unloaded (and
  * consequently freed). On the other hand, if it has been saved to the database, it is safe to unload the alignment
  * from memory.
  *
  * @param msaWindow [MsaWindow *]
  */
void MainWindow::onMsaWindowAboutToClose(MsaWindow *msaWindow)
{
    ASSERT(msaWindow != nullptr);
    const AbstractMsaSPtr &abstractMsa = msaWindows_.key(msaWindow);
    ASSERT(abstractMsa);

    msaWindows_.remove(abstractMsa);
}

/**
  * Check if the value being changed is the notes index.
  *
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void MainWindow::onMultiSeqTableModelDataChanged(const QModelIndex &topLeft, const QModelIndex & /* bottomRight */)
{
    if (activeEntityIndex_.isValid() &&
        topLeft.row() == activeEntityIndex_.row())
    {
        switch (topLeft.column())
        {
        case MultiSeqTableModel::eNotesColumn:
            notesTextEdit_->setPlainText(topLeft.data().toString());
            break;
        case MultiSeqTableModel::eSequenceColumn:
            sequenceTextView_->setSequence(topLeft.data().toString());
            break;

        default:
            return;
        }
    }
}

/**
  */
void MainWindow::onSeqGraphicsViewWheelEvent(int delta)
{
    // Save current horizontal scroll position
    double scrollX = static_cast<double>(ui_->seqGraphicsView->horizontalScrollBar()->value()) /
                     static_cast<double>(ui_->seqGraphicsView->horizontalScrollBar()->maximum());

    seqGraphicsScene_->scalePixelsPerUnit(delta);
    seqGraphicsScene_->setSceneRect(seqGraphicsScene_->paddedItemsBoundingRect(kDefaultSeqSceneVerticalPadding, kDefaultSeqSceneHorizontalPadding));
    ui_->seqGraphicsView->horizontalScrollBar()->setValue(scrollX * ui_->seqGraphicsView->horizontalScrollBar()->maximum());
}

/**
  */
void MainWindow::onTextEditFocusLost()
{
    if (activeEntityIndex_.isValid())
    {
        QModelIndex notesIndex = multiSeqTableModel_->index(activeEntityIndex_.row(), MultiSeqTableModel::eNotesColumn);
        multiSeqTableModel_->setData(notesIndex, notesTextEdit_->toPlainText());
    }
}

/**
  * @param available [bool]
  */
void MainWindow::onTextEditUndoAvailable(bool available)
{
    if (!adocModifiedBeforeNotes_)
        adoc_.setModified(available);
}

/**
  * @param bool [clean]
  */
void MainWindow::onUndoStackCleanChanged(bool clean)
{
    if (modifiedWithNoUndo_ && clean)
        return;

    if (!clean && adoc_.isModified())
        modifiedWithNoUndo_ = true;

    adoc_.setModified(!clean);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int MainWindow::askUserToSaveChanges()
{
    QMessageBox messageBox(this);
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setText("The current document has been modified.");
    messageBox.setInformativeText("Would you like to save your changes?");
    messageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Save);
    messageBox.button(QMessageBox::Save)->setText("Save Changes");

    return messageBox.exec();
}

/**
  * @returns bool
  */
bool MainWindow::canAlignSelection() const
{
    QModelIndexList selectedIndexes = ui_->tableView->selectionModel()->selectedRows();
    bool canAlign = false;
    if (selectedIndexes.size() > 1)
    {
        bool multipleTypes = false;

        AdocNodeType type = multiSeqTableModel_->nodeFromIndex(selectedIndexes.first())->nodeType_;
        foreach (const QModelIndex &index, selectedIndexes)
        {
            AdocTreeNode *node = multiSeqTableModel_->nodeFromIndex(index);
            if (type != node->nodeType_)
            {
                multipleTypes = true;
                break;
            }
        }

        // TODO: Support blasting dna sequences
        canAlign = !multipleTypes && (type == eAminoSeqNode || type == eDnaSeqNode);
    }

    return canAlign;
}

/**
  * Currently limits BLAST to protein sequences.
  *
  * TODO: Add support for blasting dna sequences
  *
  * @returns bool
  */
bool MainWindow::canBlastSelection() const
{
    QModelIndexList selectedIndexes = ui_->tableView->selectionModel()->selectedRows();
    bool canBlast = false;
    if (selectedIndexes.size() > 0)
    {
        bool multipleTypes = false;

        AdocNodeType type = multiSeqTableModel_->nodeFromIndex(selectedIndexes.first())->nodeType_;
        foreach (const QModelIndex &index, selectedIndexes)
        {
            AdocTreeNode *node = multiSeqTableModel_->nodeFromIndex(index);
            if (type != node->nodeType_)
            {
                multipleTypes = true;
                break;
            }
        }

        // TODO: Support blasting dna sequences
        canBlast = !multipleTypes && (type == eAminoSeqNode /* || type == eDnaSeqNode */);
    }

    return canBlast;
}

/**
  * @returns bool
  */
bool MainWindow::canMakePrimersWithSelection() const
{
    QModelIndex currentIndex = ui_->tableView->currentIndex();
    if (!currentIndex.isValid())
        return false;

    AdocTreeNode *node = multiSeqTableModel_->nodeFromIndex(currentIndex);
    return node->nodeType_ == eDnaSeqNode;
}

/**
  * @param modelIndexList [const QModelIndexList &]
  */
void MainWindow::deleteAdocTreeIndices(const QModelIndexList &modelIndexList)
{
    ASSERT(adocTreeModel_ != nullptr);

    // Check if any part of the selection includes a task
    foreach (const QModelIndex &index, modelIndexList)
    {
        ASSERT(index.model() == adocTreeModel_);
        if (!adocTreeModel_->nodeFromIndex(index)->containsNotOverTask())
            continue;

        QMessageBox msgBox;
        msgBox.setWindowTitle("Unable to remove selection");
        msgBox.setText("Selection contains one or more active or pending tasks (e.g. BLAST). "
                       "If you still wish to remove these tasks, first remove or stop the relevant "
                       "tasks from the Task Manager.");
        msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Open);
        msgBox.button(QMessageBox::Open)->setText("Open TaskManager");
        if (msgBox.exec() == QMessageBox::Open)
        {
            taskManagerWindow_->show();
            taskManagerWindow_->activateWindow();
            taskManagerWindow_->raise();
        }
        return;
    }

    // TODO: Check if any part of the selection includes an open msa window
    adocTreeModel_->removeRows(modelIndexList);
}

void MainWindow::enableDisableCutPasteDeleteActions()
{
    bool atLeastOneItemSelected = ui_->tableView->selectionModel()->selectedIndexes().size() > 0;
    ui_->action_Cut->setEnabled(atLeastOneItemSelected);
    ui_->action_Delete->setEnabled(atLeastOneItemSelected);

    QModelIndex currentIndex = ui_->tableView->currentIndex();
    ui_->action_Paste->setEnabled(adocTreeModel_->hasCutRows() && multiSeqTableModel_->isGroupIndex(currentIndex));
}

/**
  * Displays a progress dialog as the alignment is loaded from the database and provides for canceling if needed.
  *
  * @param msaEntity [AbstractMsa *msaEntity]
  * @returns bool
  */
bool MainWindow::loadAlignment(const AbstractMsaSPtr &msaEntity)
{
    int stepsPerIteration = 150;

    ASSERT(msaEntity != nullptr);
    if (msaEntity->msa() != nullptr)
        return true;

    IMsaRepository *msaRepository = adoc_.msaRepository(msaEntity);
    int totalSteps = msaRepository->beginLoadAlignment(msaEntity);

    QProgressDialog progressDialog(this);
    progressDialog.setLabelText("Reading alignment...");
    progressDialog.setCancelButtonText("Cancel");
    progressDialog.setMaximum(totalSteps);
    progressDialog.setWindowModality(Qt::WindowModal);
    for (int i=0; i< totalSteps; i += stepsPerIteration)
    {
        int stepsDone = msaRepository->loadAlignmentStep(stepsPerIteration);
        progressDialog.setValue(stepsDone);

        if (progressDialog.wasCanceled())
        {
            msaRepository->cancelLoadAlignment();
            return false;
        }
    }
    progressDialog.setValue(totalSteps);

    // !! Note: It is vital that endLoadAlignment is called, otherwise, the alignment data will not be associated
    //          with the AbstractMsa
    msaRepository->endLoadAlignment();

    return true;
}

/**
  * @param adocTreeNode [AdocTreeNode *]
  */
void MainWindow::openMsaWindowFromNode(AdocTreeNode *adocTreeNode)
{
    IRepository *repository = adoc_.repository(static_cast<EntityType>(adocTreeNode->nodeType_));
    IEntitySPtr entity = repository->find(adocTreeNode->entityId());
    ASSERT(boost::shared_dynamic_cast<AbstractMsa>(entity));
    AbstractMsaSPtr msaEntity = boost::shared_static_cast<AbstractMsa>(entity);
    ASSERT(msaEntity);

    // Is there a MsaWindow already editing this alignment?
    if (msaWindows_.contains(msaEntity))
    {
        ASSERT(msaEntity->msa() != nullptr);

        // Highlight the msaWindow
        ASSERT(msaWindows_.value(msaEntity) != nullptr);
        msaWindows_.value(msaEntity)->raise();
        msaWindows_.value(msaEntity)->activateWindow();
        return;
    }

    if (!loadAlignment(msaEntity))
        return;

    ASSERT(msaEntity->msa() != nullptr);

    MsaWindow *msaWindow = nullptr;
    switch(msaEntity->grammar())
    {
    case eAminoGrammar:
        msaWindow = new AminoMsaWindow(&adoc_, taskManager_, blastDatabaseModel_, blastDatabaseWindow_);
        break;
    case eDnaGrammar:
        msaWindow = new DnaMsaWindow(&adoc_, taskManager_);
        break;

    default:
        ASSERT(0);
        return;
    }

    msaWindow->setAttribute(Qt::WA_DeleteOnClose);      // Delete the window when it closes :)
    msaWindows_.insert(msaEntity, msaWindow);

    connect(msaWindow, SIGNAL(aboutToClose(MsaWindow*)), SLOT(onMsaWindowAboutToClose(MsaWindow*)));
    msaWindow->show();

    // Currently set the msa entity *after* showing the window so that the number of labels that may be displayed in
    // the graphics view may be accurately determined from its geometry (which is not be available unless it is visible)
    // When a custom QGraphicsView is in place which properly handles resize events, this will no longer be necessary.
    msaWindow->setMsaEntity(msaEntity);
}

/**
  * Necessary whenever the adoc changes (e.g. from opening/closing a document)
  */
void MainWindow::updateEntityAdapterSpecifications()
{
    ASSERT(aminoSeqColumnAdapter_ != nullptr);
    ASSERT(adoc_.aminoSeqRepository() != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification aminoSeqSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                           adoc_.aminoSeqRepository(),
                                                                           aminoSeqColumnAdapter_);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eIdColumn, AminoSeqColumnAdapter::eIdColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, AminoSeqColumnAdapter::eTypeColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eNameColumn, AminoSeqColumnAdapter::eNameColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eSourceColumn, AminoSeqColumnAdapter::eSourceColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eStartColumn, AminoSeqColumnAdapter::eStartColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eStopColumn, AminoSeqColumnAdapter::eStopColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eLengthColumn, AminoSeqColumnAdapter::eLengthColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eDescriptionColumn, AminoSeqColumnAdapter::eDescriptionColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eNotesColumn, AminoSeqColumnAdapter::eNotesColumn);
    aminoSeqSpec.setMapping(MultiSeqTableModel::eSequenceColumn, AminoSeqColumnAdapter::eSequenceColumn);
    multiSeqTableModel_->setAdapterSpecification(eAminoSeqEntity, aminoSeqSpec);


    ASSERT(aminoMsaColumnAdapter_ != nullptr);
    ASSERT(adoc_.aminoMsaRepository() != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification aminoMsaSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                           adoc_.aminoMsaRepository(),
                                                                           aminoMsaColumnAdapter_);
    aminoMsaSpec.setMapping(MultiSeqTableModel::eIdColumn, AminoMsaColumnAdapter::eIdColumn);
    aminoMsaSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, AminoMsaColumnAdapter::eTypeColumn);
    aminoMsaSpec.setMapping(MultiSeqTableModel::eNameColumn, AminoMsaColumnAdapter::eNameColumn);
    aminoMsaSpec.setMapping(MultiSeqTableModel::eDescriptionColumn, AminoMsaColumnAdapter::eDescriptionColumn);
    aminoMsaSpec.setMapping(MultiSeqTableModel::eNotesColumn, AminoMsaColumnAdapter::eNotesColumn);
    multiSeqTableModel_->setAdapterSpecification(eAminoMsaEntity, aminoMsaSpec);


    ASSERT(dnaSeqColumnAdapter_ != nullptr);
    ASSERT(adoc_.dnaSeqRepository() != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification dnaSeqSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                         adoc_.dnaSeqRepository(),
                                                                         dnaSeqColumnAdapter_);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eIdColumn, DnaSeqColumnAdapter::eIdColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, DnaSeqColumnAdapter::eTypeColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eNameColumn, DnaSeqColumnAdapter::eNameColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eSourceColumn, DnaSeqColumnAdapter::eSourceColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eStartColumn, DnaSeqColumnAdapter::eStartColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eStopColumn, DnaSeqColumnAdapter::eStopColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eLengthColumn, DnaSeqColumnAdapter::eLengthColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eDescriptionColumn, DnaSeqColumnAdapter::eDescriptionColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eNotesColumn, DnaSeqColumnAdapter::eNotesColumn);
    dnaSeqSpec.setMapping(MultiSeqTableModel::eSequenceColumn, DnaSeqColumnAdapter::eSequenceColumn);
    multiSeqTableModel_->setAdapterSpecification(eDnaSeqEntity, dnaSeqSpec);


    ASSERT(dnaMsaColumnAdapter_ != nullptr);
    ASSERT(adoc_.dnaMsaRepository() != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification dnaMsaSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                         adoc_.dnaMsaRepository(),
                                                                         dnaMsaColumnAdapter_);
    dnaMsaSpec.setMapping(MultiSeqTableModel::eIdColumn, DnaMsaColumnAdapter::eIdColumn);
    dnaMsaSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, DnaMsaColumnAdapter::eTypeColumn);
    dnaMsaSpec.setMapping(MultiSeqTableModel::eNameColumn, DnaMsaColumnAdapter::eNameColumn);
    dnaMsaSpec.setMapping(MultiSeqTableModel::eDescriptionColumn, DnaMsaColumnAdapter::eDescriptionColumn);
    dnaMsaSpec.setMapping(MultiSeqTableModel::eNotesColumn, DnaMsaColumnAdapter::eNotesColumn);
    multiSeqTableModel_->setAdapterSpecification(eDnaMsaEntity, dnaMsaSpec);


    ASSERT(transientTaskColumnAdapter_ != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification taskSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                       adoc_.transientTaskRepository(),
                                                                       transientTaskColumnAdapter_);
    taskSpec.setMapping(MultiSeqTableModel::eIdColumn, TransientTaskColumnAdapter::eIdColumn);
    taskSpec.setMapping(MultiSeqTableModel::eNameColumn, TransientTaskColumnAdapter::eNameColumn);
    taskSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, TransientTaskColumnAdapter::eTypeColumn);
    multiSeqTableModel_->setAdapterSpecification(eTransientTaskEntity, taskSpec);


    ASSERT(blastReportColumnAdapter_ != nullptr);
    AbstractMultiEntityTableModel::EntityAdapterSpecification blastReportSpec(MultiSeqTableModel::eNumberOfColumns,
                                                                              adoc_.blastReportRepository(),
                                                                              blastReportColumnAdapter_);
    blastReportSpec.setMapping(MultiSeqTableModel::eIdColumn, BlastReportColumnAdapter::eIdColumn);
    blastReportSpec.setMapping(MultiSeqTableModel::eNameColumn, BlastReportColumnAdapter::eNameColumn);
    blastReportSpec.setMapping(MultiSeqTableModel::eEntityTypeColumn, BlastReportColumnAdapter::eTypeColumn);
    blastReportSpec.setMapping(MultiSeqTableModel::eDescriptionColumn, BlastReportColumnAdapter::eDescriptionColumn);
    blastReportSpec.setMapping(MultiSeqTableModel::eNotesColumn, BlastReportColumnAdapter::eNotesColumn);
    blastReportSpec.setMapping(MultiSeqTableModel::eSequenceColumn, BlastReportColumnAdapter::eQuerySequenceColumn);
    multiSeqTableModel_->setAdapterSpecification(eBlastReportEntity, blastReportSpec);
}

/**
  * Updates the entity notes for the table view index
  */
void MainWindow::updateEntityNotes(const QModelIndex &index)
{
    ASSERT(index.model() == multiSeqTableModel_);
    ASSERT(index.isValid());

    multiSeqTableModel_->setData(multiSeqTableModel_->index(index.row(), MultiSeqTableModel::eNotesColumn),
                                 notesTextEdit_->toPlainText());
}

/**
  */
void MainWindow::readPersistentSettings()
{
    ASSERT(blastDatabaseModel_ != nullptr);

    QSettings settings;

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(800, 640)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    seqGraphicsScene_->setPixelsPerUnit(settings.value("pixelsPerUnit", 1.).toDouble());
    ui_->tableView->horizontalHeader()->restoreState(settings.value("tableHeader").toByteArray());
    settings.endGroup();

    settings.beginGroup("BlastDatabaseManager");
    blastDatabaseModel_->setBlastPaths(settings.value("blastPaths").toStringList());
    settings.endGroup();
}

/**
  */
void MainWindow::writePersistentSettings()
{
    ASSERT(blastDatabaseModel_ != nullptr);

    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("pixelsPerUnit", seqGraphicsScene_->pixelsPerUnit());
    settings.setValue("tableHeader", ui_->tableView->horizontalHeader()->saveState());
    settings.endGroup();

    settings.beginGroup("BlastDatabaseManager");
    settings.setValue("blastPaths", blastDatabaseModel_->blastPaths());
    settings.endGroup();
}

AbstractMsaBuilder *MainWindow::getAminoMsaBuilderFromUser()
{
    static MsaAlignerOptionsDialog *aminoAlignerOptionsDialog = nullptr;

    MsaBuilderFactory msaBuilderFactory;
    if (aminoAlignerOptionsDialog == nullptr)
    {
        aminoAlignerOptionsDialog = new MsaAlignerOptionsDialog(this);
        MsaBuilderOptionsWidgetFactory optionsWidgetFactory;
        QVector<IMsaBuilderOptionsWidget *> optionWidgets = optionsWidgetFactory.makeMsaBuilderOptionsWidgets(msaBuilderFactory.supportedMsaBuilderIds(), eAminoGrammar);
        aminoAlignerOptionsDialog->setMsaBuilderOptionsWidgets(optionWidgets);
    }

    return getMsaBuilderOptions(msaBuilderFactory, aminoAlignerOptionsDialog);
}

AbstractMsaBuilder *MainWindow::getDnaMsaBuilderFromUser()
{
    static MsaAlignerOptionsDialog *dnaAlignerOptionsDialog = nullptr;

    MsaBuilderFactory msaBuilderFactory;
    if (dnaAlignerOptionsDialog == nullptr)
    {
        dnaAlignerOptionsDialog = new MsaAlignerOptionsDialog(this);
        MsaBuilderOptionsWidgetFactory optionsWidgetFactory;
        QVector<IMsaBuilderOptionsWidget *> optionWidgets = optionsWidgetFactory.makeMsaBuilderOptionsWidgets(msaBuilderFactory.supportedMsaBuilderIds(), eDnaGrammar);
        dnaAlignerOptionsDialog->setMsaBuilderOptionsWidgets(optionWidgets);
    }

    return getMsaBuilderOptions(msaBuilderFactory, dnaAlignerOptionsDialog);
}

AbstractMsaBuilder *MainWindow::getMsaBuilderOptions(const MsaBuilderFactory &msaBuilderFactory, MsaAlignerOptionsDialog *msaAlignerOptionsDialog) const
{
    ASSERT(msaAlignerOptionsDialog != nullptr);

    if (!msaAlignerOptionsDialog->exec())
        return nullptr;

    AbstractMsaBuilder *msaBuilder = msaBuilderFactory.makeMsaBuilder(msaAlignerOptionsDialog->msaBuilderId());
    msaBuilder->setOptions(msaAlignerOptionsDialog->msaBuilderOptions());
    return msaBuilder;

}
