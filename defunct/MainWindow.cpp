/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MainWindow.h"
#include "ui_MainWindow.h"

// ------------------------------------------------------------------------------------------------
// Supporting headers
#include <QtCore/QModelIndex>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QtDebug>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QWidget>

#include "forms/ImportSequencesDialog.h"
#include "forms/MsaWindow.h"
#include "forms/PrimerFinderDialog.h"
#include "forms/ProjectGroupSelectionDialog.h"

#include "models/AllSliceModel.h"
#include "models/AminoSliceModel.h"
#include "models/DnaSliceModel.h"
#include "models/MsaSubseqModel.h"
#include "models/RelatedTableModel.h"
#include "models/RestrictionEnzymeTableModel.h"
#include "models/RnaSliceModel.h"
#include "models/SliceSortProxyModel.h"
#include "models/SubseqSliceModel.h"
#include "models/TableModel.h"

#include "widgets/LineEditDelegate.h"

#include "AdocTreeNode.h"
#include "AminoString.h"
#include "DbAnonSeqFactory.h"
#include "DnaString.h"
#include "Msa.h"
#include "RnaString.h"
#include "SequenceImporter.h"
#include "SynchronousAdocDataSource.h"
#include "ThreadedAdocDataSource.h"
// #include <QtCore/QThread>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Construct the MainWindow and prepare it for action. This involves substantial preparation including
  * model instantiation, temporary SQLite data file initialization, GUI tweaking and much more.
  *
  * @param parent [QWidget *]
  */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    // ----------------------------
    // Initialize all pointers to 0
    adocDataSource_ = 0;

    selectAdocFileDialog_ = 0;
    importSequencesDialog_ = 0;
    primerFinderDialog_ = 0;
    projectGroupSelectionDialog_ = 0;

    adocTreeModel_ = 0;
    aminoSubseqsTable_ = 0;
    dnaSubseqsTable_ = 0;
    rnaSubseqsTable_ = 0;
    aminoSeqsTable_ = 0;
    dnaSeqsTable_ = 0;
    rnaSeqsTable_ = 0;

    containerFilterModel_ = 0;
    allSliceModel_ = 0;
    aminoSliceModel_ = 0;
    dnaSliceModel_ = 0;
    rnaSliceModel_ = 0;
    sliceSortProxyModel_ = 0;

    msaWindow_ = 0;

    // ----------------------------
    // View model setup - these are not the data models, but rather the adatpor
    // models used by the various UI views

    // Filters out all non-container nodes (only permits group nodes and msas)
    containerFilterModel_ = new AdocTypeFilterModel(this);
    containerFilterModel_->setAcceptNodeTypes(QList<AdocTreeNode::NodeType>() << AdocTreeNode::RootType
                                                                              << AdocTreeNode::GroupType);

    // Initialize the proxy model first so that it is deleted before any possible models it will be associated with
    // See log.txt [1 Sep 2010]
    sliceSortProxyModel_ = new SliceSortProxyModel(this);
    allSliceModel_ = new AllSliceModel(this);
    aminoSliceModel_ = new AminoSliceModel(this);
    dnaSliceModel_ = new DnaSliceModel(this);
    rnaSliceModel_ = new RnaSliceModel(this);

    // Initially, show all items (no filter)
    sliceSortProxyModel_->setSourceModel(allSliceModel_);

    // ---------------------------------
    // GUI control setup
    ui_->setupUi(this);     // Initialize our form components

    // --> Container tree view
    ui_->containerTreeView->setModel(containerFilterModel_);
    ui_->containerTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui_->containerTreeView->setItemDelegate(new LineEditDelegate(ui_->containerTreeView));

    // --> Item table view
    ui_->itemTableView->setItemDelegate(ui_->containerTreeView->itemDelegate());
    ui_->itemTableView->setModel(sliceSortProxyModel_);
    // Set default sort order to ascending
    ui_->itemTableView->sortByColumn(allSliceModel_->defaultSortColumn(), Qt::AscendingOrder);

    // --> View combobox
    // Add the items the viewComboBox
    ui_->viewComboBox->addItem("All items", eViewAll);
    ui_->viewComboBox->addItem("Proteins", eViewAmino);
    ui_->viewComboBox->addItem("Genes (DNA)", eViewDna);
    ui_->viewComboBox->addItem("Genes (RNA)", eViewRna);

    // ----------------------------------
    // Signals and slots
    // --> Actions
    QObject::connect(ui_->action_Import, SIGNAL(triggered()), this, SLOT(showImportSequencesDialog()));
    QObject::connect(ui_->actionNew_Group, SIGNAL(triggered()), this, SLOT(createEditNewGroup()));
    QObject::connect(ui_->actionNew_Project, SIGNAL(triggered()), this, SLOT(createEditNewTopLevelGroup()));

    // --> TreeView
    QObject::connect(ui_->containerTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(onContainerTreeCurrentChanged(QModelIndex, QModelIndex)));
    QObject::connect(ui_->itemTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemTableViewDoubleClick(QModelIndex)));

    // --> View combo box
    QObject::connect(ui_->viewComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewType(int)));

    // -----------------------------------
    // TODO: Reassign all action keyboard shortcuts to use the appropriate QKeySequence::StandardKey
    // Example:   newAction->setShortcut(QKeySequence::New);


    // -----------------------------------
    // Register metatypes
    qRegisterMetaType<Alphabet>("Alphabet");
    qRegisterMetaType<DbAnonSeqFactory *>("DbAnonSeqFactory *");
    qRegisterMetaType<const char *>("const char *");
    qRegisterMetaType<QList<DataRow> >("QList<DataRow>");

    // -----------------------------------
    // Finally load our persistent settings
    readPersistentSettings();

    // Temporary testing and utility crap
    QObject::connect(ui_->pushButton, SIGNAL(clicked()), this, SLOT(doSomething()));
//    qDebug() << "Main thread id:" << QThread::currentThreadId();

    // Save us some time
    openFile("blah.db");
    adocDataSource_->readMsa(1, eAminoAlphabet, 0);
}

MainWindow::~MainWindow()
{
    // Safe guard against any source model being removed before the proxy model and thus potentially permitting
    // the proxyModel to access an invalid source model pointer. See log.txt [1 Sep 2010]
    sliceSortProxyModel_->setSourceModel(0);

    delete ui_;

    if (msaWindow_)
        delete msaWindow_;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void MainWindow::closeEvent(QCloseEvent *event)
{
    // When the MainWindow is closed, all child widgets are hidden. For view widgets, this results in
    // recalculating the column widths for any columns that have been set to automatically resize to
    // their contents width. This recalculation must fetch the data for each item in these columns
    // and results in an undesirable and completely unnecessary delay.
    //
    // To sidestep this effect, the associated model is simply cleared before continuing the close operation.
    ui_->itemTableView->setModel(0);

    writePersistentSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::readPersistentSettings()
{
    QSettings settings("Agile Genomics, LLC", "AlignShop");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(800, 640)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

void MainWindow::writePersistentSettings()
{
    QSettings settings("Agile Genomics, LLC", "AlignShop");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

/**
  * Updates the root index of the itemTableView to current if it is valid. If the current column is something
  * other than zero nothing will change.
  *
  * @param current [const QModelIndex &]
  * @param previous [const QModelIndex &]
  */
void MainWindow::onContainerTreeCurrentChanged(const QModelIndex &containerCurrent, const QModelIndex & /* previous */)
{
    if (!containerCurrent.isValid())
        return;

    if (containerCurrent.column() != 0)
    {
        qWarning() << "MainWindow::updateItemTreeViewRoot - column should be zero (actual:" << containerCurrent.column() << ")";
        return;
    }

    SliceProxyModel *sliceProxyModel = static_cast<SliceProxyModel *>(sliceSortProxyModel_->sourceModel());
    ASSERT_X(containerCurrent.model() == containerFilterModel_, "containerCurrent index does not belong to the appropriate model");
    ASSERT_X(ui_->itemTableView->model(), "itemTableView must have a non-null model");
    ASSERT(sliceProxyModel);

    // Map the containerCurrent index to AdocTreeModel
    QModelIndex sourceIndex = containerFilterModel_->mapToSource(containerCurrent);

    // Turn off visual updates until we have finished changing the root index and sorting; prevent flicker
    ui_->itemTableView->setUpdatesEnabled(false);
    sliceProxyModel->setSourceParent(sourceIndex);
    ui_->itemTableView->resizeColumnToContents(sliceProxyModel->defaultSortColumn());
    ui_->itemTableView->sortByColumn(sliceProxyModel->defaultSortColumn(), ui_->itemTableView->horizontalHeader()->sortIndicatorOrder());
    ui_->itemTableView->setUpdatesEnabled(true);
}

/**
  * If proxy index is valid and a group node, map to its corresponding container filter index, and set this index as the
  * currently selected index. If this is a different node than previously established, it will emit the currentChanged
  * signal, which will call the onContainerTreeCurrentChanged slot and update the view accordingly.
  *
  * @param proxyIndex [const QModelIndex &]
  */
void MainWindow::onItemTableViewDoubleClick(const QModelIndex &clickedIndex)
{
    // Ensure we have a valid index
    if (!clickedIndex.isValid())
        return;

    SliceProxyModel *sliceProxyModel = static_cast<SliceProxyModel *>(sliceSortProxyModel_->sourceModel());
    QModelIndex sourceIndex = sliceProxyModel->mapToSource(sliceSortProxyModel_->mapToSource(clickedIndex));
    QModelIndex containerIndex = containerFilterModel_->mapFromSource(sourceIndex);

    // Check if it is a group node
    AdocTreeNode *node = adocTreeModel_->nodeFromIndex(sourceIndex);
//    AdocTreeNode::NodeType nodeType = clickedIndex.data(AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>();
    AdocTreeNode::NodeType nodeType = node->nodeType_;
    switch (nodeType)
    {
    case AdocTreeNode::MsaAminoType:
        adocDataSource_->readMsa(node->fkId_, eAminoAlphabet, 0);
        return;

    case AdocTreeNode::GroupType:
        break;

    default:
        return;
    }

    // Runtime check to make sure that we reference a non-root node (root is not visible)
    ASSERT_X(containerIndex.isValid(), "mapped container index must be valid; check proxy filter rules");
    if (!containerIndex.isValid())  // Release mode guard
        return;

    // Make sure the parent is expanded if it is not already
    ui_->containerTreeView->expand(containerIndex.parent());

    // And select the new index
    ui_->containerTreeView->setCurrentIndex(containerIndex);
}

/**
  * Because all view types are sublcasses of AdocSortFilterProxyModel and the same number of model layers
  * apart from the root, it is possible to treat view changes uniformly via polymorphism. Part of this process
  * is saving the expanded nodes before changing the view type and then restoring them afterwards. Before the
  * view is changed, the expanded nodes belong to the oldModel. These must be mapped to the VaryingColumnProxyModel
  * and finally the data tree source because the VaryingColumnProxyModel::setColumnCount() method resets the
  * associated model and will invalidate the expanded node list. Thus, we keep them valid by mapping them to the
  * root tree model and then finally map them back again to the new model afterwards.
  *
  * To prevent flickering and otherwise awkward screen updates, we disable repainting until all the changes have
  * been made.
  *
  * @param viewType [int]
  */
void MainWindow::setViewType(int viewType)
{
    Q_ASSERT_X(viewType >= 0 && viewType <= 3, QString("MainWindow::setViewType(%1)").arg(viewType).toAscii(), "viewType parameter out of range");

    SliceProxyModel *oldModel = static_cast<SliceProxyModel *>(sliceSortProxyModel_->sourceModel());
    SliceProxyModel *newModel = 0;

    switch (viewType)
    {
    case eViewAmino:        newModel = aminoSliceModel_;        break;
    case eViewDna:          newModel = dnaSliceModel_;          break;
    case eViewRna:          newModel = rnaSliceModel_;          break;
    case eViewAll:
    default:
        newModel = allSliceModel_;
    }

    if (oldModel == newModel)
        return;

    // To avoid flickering, we temporarily disable painting
    ui_->itemTableView->setUpdatesEnabled(false);

    sliceSortProxyModel_->setSourceModel(0);

    // Minor optimization that disconnects all the signals
    oldModel->clear();

    // Update the new model
    sliceSortProxyModel_->setSourceModel(newModel);

    // Restore the painting
    ui_->itemTableView->setUpdatesEnabled(true);

    // Reset the ItemTableView root to the currently selected index
    onContainerTreeCurrentChanged(ui_->containerTreeView->currentIndex(), QModelIndex());
}

/**
  * Opens the import sequences dialog and initiates an import routine.
  */
void MainWindow::showImportSequencesDialog()
{
    if (!importSequencesDialog_)
        importSequencesDialog_ = new ImportSequencesDialog(this);

    // We have not yet displayed the owning import sequences dialog box, but will be displaying its
    // file selection dialog box. Problem: we want this centered on the current window. Thus we have
    // to manually do it here.
    importSequencesDialog_->positionSequenceFileDialog(this);

    QString sequenceFile = importSequencesDialog_->getSequenceFile();
    if (sequenceFile.isEmpty())     // User did not select a file
        return;

    // To give a faster response, show the dialog before processing the file and processEvents
    // to make sure this signal is received.
    importSequencesDialog_->show();
    QCoreApplication::processEvents();

    importSequencesDialog_->processFile(sequenceFile);
    if (importSequencesDialog_->exec() == QDialog::Accepted)
    {
        /*

        // Because ImportSequencesDialog::onCheckedChange disables the importButton whenever there are no sequences
        // selected and since we received an accept result from the dialog, there should be at least one valid
        // sequence with a well-defined alphabet.
        //
        // Double check for precaution.
        int nChecked = importSequencesDialog_->countChecked();
        Q_ASSERT_X(nChecked > 0, "MainWindow::showImportSequencesDialog", "at least one sequence should have been checked here");
        if (nChecked)
        {
            // Now request destination of imported sequences
            if (!projectGroupSelectionDialog_)
                projectGroupSelectionDialog_ = new ProjectGroupSelectionDialog(adocTreeModel_, this);

            if (projectGroupSelectionDialog_->exec() == QDialog::Accepted)
            {
                // Import the sequences
                SequenceImporter importer;
                importer.setAdocTreeModel(adocTreeModel_);
//                importer.setAnonSeqFactories(adoc_.anonSeqFactories());

                try
                {
                    if (importSequencesDialog_->isAlignmentChecked())
                    {
                        Q_ASSERT_X(nChecked >= 2, "MainWindow::showImportSequencesDialog", "at least two sequences should be selected when importing an alignment");

                        // Name the alignment after the base file name
                        QFileInfo fi(sequenceFile);
                        QString alignmentName = fi.baseName();
                        if (alignmentName.isEmpty())    // If for some reason there is an empty base name, simply give it a generic name
                            alignmentName = "New alignment";
                        importer.importAlignment(alignmentName,
                                                 importSequencesDialog_->selectedSequences(),
                                                 importSequencesDialog_->alphabet(),
                                                 projectGroupSelectionDialog_->selectedIndex());
                    }
                    else    // Importing purely sequence data
                    {
                        importer.importSequences(importSequencesDialog_->selectedSequences(),
                                                 importSequencesDialog_->alphabet(),
                                                 projectGroupSelectionDialog_->selectedIndex());
                    }
                }
                catch (DatabaseError &e)
                {
                    QMessageBox::warning(this, "Database error", e.sql(), QMessageBox::Ok);
                }
                catch (Exception &e)
                {
                    QMessageBox::warning(this, "Import error", e.what(), QMessageBox::Ok);
                }

                qDebug() << adocTreeModel_->data(projectGroupSelectionDialog_->selectedIndex(), Qt::DisplayRole);
            }
        }
        */
    }

    // Free up any memory allocated by the model associated with the importSequencesDialog
    importSequencesDialog_->clearModel();
}


/**
  * [UNTESTED]
  */
void MainWindow::createEditNewGroup()
{
    if (!adocTreeModel_)
        return;

    QModelIndex subGroupIndex = adocTreeModel_->newGroup("New group", containerFilterModel_->mapToSource(ui_->containerTreeView->currentIndex()));
    if (!subGroupIndex.isValid())
    {
        QMessageBox::warning(this, tr("Unable to create group"), tr("Please select a valid item before creating a new group"), QMessageBox::Ok);
        return;
    }

    subGroupIndex = containerFilterModel_->mapFromSource(subGroupIndex);
    ui_->containerTreeView->setCurrentIndex(subGroupIndex);
    ui_->containerTreeView->edit(subGroupIndex);
}

void MainWindow::createEditNewTopLevelGroup()
{
    if (!adocTreeModel_)
        return;

    QModelIndex groupIndex = adocTreeModel_->newGroup("New project", QModelIndex());
    if (!groupIndex.isValid())
    {
        QMessageBox::warning(this, tr("Unable to create new group"), tr("An unexpected error occurred. Please try again or contact support."), QMessageBox::Ok);
        return;
    }

    groupIndex = containerFilterModel_->mapFromSource(groupIndex);
    ui_->containerTreeView->setCurrentIndex(groupIndex);
    ui_->containerTreeView->edit(groupIndex);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Recursively traverses the nodes beneath index and stores all expanded child treeView nodes (that are
  * groups) in list.
  *
  * OPTIMIZATION: Investigate tail-recursion
  *
  * @param index [const QModelIndex &]
  * @param treeView [const QTreeView *]
  * @param list [QList<QModelIndex> *]
  * @see setViewType()
  */
void MainWindow::recurseFetchExpandedGroups(const QModelIndex &index, const QTreeView *treeView, QList<QModelIndex> *list) const
{
    Q_ASSERT_X(treeView, "MainWindow::recurseFetchExpandedGroups", "treeView parameter must not be null");
    Q_ASSERT_X(list, "MainWindow::recurseFetchExpandedGroups", "list parameter must not be null");
    if (!treeView || !list) // Release mode catch
        return;

    for (int i=0, z=index.model()->rowCount(index); i<z; ++i)
    {
        QModelIndex childIndex = index.child(i, 0);
        if (!childIndex.isValid())
            continue;

        // Only save this node if it is a group and expanded
        if (childIndex.data(AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>() == AdocTreeNode::GroupType
            && treeView->isExpanded(childIndex))
        {
            list->append(childIndex);
            recurseFetchExpandedGroups(childIndex, treeView, list);
        }
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void MainWindow::on_onActionDesign_Primers_triggered()
{
    if (!primerFinderDialog_)
        primerFinderDialog_ = new PrimerFinderDialog(this);

    primerFinderDialog_->exec();
}






void MainWindow::doSomething()
{
    // Lets start a process
    process_ = new QProcess(this);
    connect(process_, SIGNAL(readyReadStandardOutput()), SLOT(onReadyReadStdout()));
    connect(process_, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
    connect(process_, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << "-d" << "/binf/research/nn-sec/blastdb/uniref50.segcoil"
              << "-a" << "4"
              << "-j" << "2";

    process_->start("/binf/bin/ag-blastpgp", arguments);

    qDebug() << "DoSomething";
}

void MainWindow::onReadyReadStdout()
{
    qDebug() << process_->readAllStandardOutput();
}

void MainWindow::onStateChanged(QProcess::ProcessState newState)
{
    switch (newState)
    {
    case QProcess::NotRunning:
        qDebug() << "State change: not running";
        break;
    case QProcess::Starting:
        qDebug() << "State change: starting";
        process_->write("MVLSEGEWQLVLHVWAKVEADVAGHGQDILIRLFKSHPETLEKFDRVKHLKTEAEMKASEDLKKHGVTVLTALGAILKKKGHHEAELKPLAQSHATKHKIPIKYLEFISEAIIHVLHSRHPGNFGADAQGAMNKALELFRKDIAAKYKELGYQG");
        process_->closeWriteChannel();
        break;
    case QProcess::Running:
        qDebug() << "State change: running";
        break;
    }
}

void MainWindow::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process finished with code" << exitCode << "and status" << exitStatus;
}


void MainWindow::openFile(const QString &file)
{
    initializeAdocDataSource();

    adocDataSource_->open(file);
}


void MainWindow::initializeAdocDataSource()
{
    if (adocDataSource_)
        return;

    adocDataSource_ = new SynchronousAdocDataSource(this);
//    adocDataSource_ = new ThreadedAdocDataSource(this);

    // Setup the AnonSeqFactories
    DbAnonSeqFactory *aminoFactory = new DbAnonSeqFactory;
    DbAnonSeqFactory *dnaFactory = new DbAnonSeqFactory;
    DbAnonSeqFactory *rnaFactory = new DbAnonSeqFactory;

    // Set the prototype BioString types
    aminoFactory->setBioStringPrototype(new AminoString);
    dnaFactory->setBioStringPrototype(new DnaString);
    rnaFactory->setBioStringPrototype(new RnaString);

    aminoFactory->setSourceTable(constants::kTableAstrings);
    dnaFactory->setSourceTable(constants::kTableDstrings);
    rnaFactory->setSourceTable(constants::kTableRstrings);

    adocDataSource_->setAnonSeqFactory(eAminoAlphabet, aminoFactory);
    adocDataSource_->setAnonSeqFactory(eDnaAlphabet, dnaFactory);
    adocDataSource_->setAnonSeqFactory(eRnaAlphabet, rnaFactory);

    // ------------------------------------------------------------------------
    // Signal / slot connections
    connect(adocDataSource_, SIGNAL(openError(QString,QString)), SLOT(onAdocDataSourceOpenError(QString,QString)));
    connect(adocDataSource_, SIGNAL(opened()), adocDataSource_, SLOT(readDataTree()));              // After successfully opening a data file, immediately attempt to read the data tree
    connect(adocDataSource_, SIGNAL(dataTreeError(QString)), SLOT(onDataTreeError(QString)));
    connect(adocDataSource_, SIGNAL(dataTreeError(QString)), adocDataSource_, SLOT(close()));       // If unable to read the data tree, go ahead and close the data source
    connect(adocDataSource_, SIGNAL(dataTreeReady(AdocTreeNode*)), SLOT(onDataTreeReady(AdocTreeNode*)));

    connect(adocDataSource_, SIGNAL(msaReady(Msa*,int)), this, SLOT(onMsaReady(Msa*,int)));
    connect(adocDataSource_, SIGNAL(msaError(QString,int)), this, SLOT(onMsaError(QString,int)));
}

void MainWindow::initializeTableModels()
{
    ASSERT(adocDataSource_);

    if (!aminoSeqsTable_)
    {
        aminoSeqsTable_ = new TableModel(this);
        aminoSeqsTable_->setSource(adocDataSource_, constants::kTableAminoSeqs, AminoSliceModel::seqFields_);
        aminoSeqsTable_->setFriendlyFieldNames(AminoSliceModel::friendlySeqFieldNames_);

        ASSERT(!aminoSubseqsTable_);
        aminoSubseqsTable_ = new RelatedTableModel(this);
        aminoSubseqsTable_->setSource(adocDataSource_, constants::kTableAminoSubseqs, AminoSliceModel::subseqFields_);
        aminoSubseqsTable_->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqsTable_, "amino_seq_id", true);
        aminoSubseqsTable_->setFriendlyFieldNames(AminoSliceModel::friendlySubseqFieldNames_);

        // Automatically clear table models when the data source is closed
        connect(adocDataSource_, SIGNAL(closed()), aminoSeqsTable_, SLOT(clear()));
        connect(adocDataSource_, SIGNAL(closed()), aminoSubseqsTable_, SLOT(clear()));
    }

    if (!dnaSeqsTable_)
    {
        dnaSeqsTable_ = new TableModel(this);
        dnaSeqsTable_->setSource(adocDataSource_, constants::kTableDnaSeqs, DnaSliceModel::seqFields_);
        dnaSeqsTable_->setFriendlyFieldNames(DnaSliceModel::friendlySeqFieldNames_);

        ASSERT(!dnaSubseqsTable_);
        dnaSubseqsTable_ = new RelatedTableModel(this);
        dnaSubseqsTable_->setSource(adocDataSource_, constants::kTableDnaSubseqs, DnaSliceModel::subseqFields_);
        dnaSubseqsTable_->setFriendlyFieldNames(DnaSliceModel::friendlySubseqFieldNames_);

        // Automatically clear table models when the data source is closed
        connect(adocDataSource_, SIGNAL(closed()), dnaSeqsTable_, SLOT(clear()));
        connect(adocDataSource_, SIGNAL(closed()), dnaSubseqsTable_, SLOT(clear()));
    }

    if (!rnaSeqsTable_)
    {
        rnaSeqsTable_ = new TableModel(this);
        rnaSeqsTable_->setSource(adocDataSource_, constants::kTableRnaSeqs, RnaSliceModel::seqFields_);
        rnaSeqsTable_->setFriendlyFieldNames(RnaSliceModel::friendlySeqFieldNames_);

        ASSERT(!rnaSubseqsTable_);
        rnaSubseqsTable_ = new RelatedTableModel(this);
        rnaSubseqsTable_->setSource(adocDataSource_, constants::kTableRnaSubseqs, RnaSliceModel::subseqFields_);
        rnaSubseqsTable_->setFriendlyFieldNames(RnaSliceModel::friendlySubseqFieldNames_);

        // Automatically clear table models when the data source is closed
        connect(adocDataSource_, SIGNAL(closed()), rnaSeqsTable_, SLOT(clear()));
        connect(adocDataSource_, SIGNAL(closed()), rnaSubseqsTable_, SLOT(clear()));
    }
}

void MainWindow::on_actionNew_document_triggered()
{
    if (adocDataSource_ && adocDataSource_->isOpen())
    {
        // TODO: open up new instance of application with empty file
        // For now, just display messagebox

        return;
    }

    // Create a new document for this instance
}

void MainWindow::on_actionOpen_triggered()
{
    if (!selectAdocFileDialog_)
    {
        selectAdocFileDialog_ = new QFileDialog(this);

        connect(selectAdocFileDialog_, SIGNAL(fileSelected(QString)), this, SLOT(openFile(QString)));
    }

    if (adocDataSource_ && adocDataSource_->isOpen())
    {
        // TODO: open up new instance of application with this file
        // For now, just display messagebox
        QMessageBox msgBox;
        msgBox.setText("Please close the open file first");
        msgBox.exec();

        return;
    }

    // Show the open file dialog
    selectAdocFileDialog_->exec();
}

void MainWindow::onAdocDataSourceOpenError(const QString &file, const QString &error)
{
    // Show error message
}

void MainWindow::onDataTreeError(const QString &error)
{
    // Show error message
}

void MainWindow::onDataTreeReady(AdocTreeNode *root)
{
    ASSERT(root);

    if (!adocTreeModel_)
    {
        adocTreeModel_ = new AdocTreeModel(this);

        // Make sure the table models that read/write data are available
        initializeTableModels();

        // Update the UI models with this tree model
        ASSERT(containerFilterModel_);
        containerFilterModel_->setSourceModel(adocTreeModel_);

        // Associate the slice models with this tree model
        allSliceModel_->setTreeModel(adocTreeModel_);
        aminoSliceModel_->setTreeModel(adocTreeModel_);
        aminoSliceModel_->setSourceTables(eAminoAlphabet, aminoSubseqsTable_, aminoSeqsTable_);
        dnaSliceModel_->setTreeModel(adocTreeModel_);
        dnaSliceModel_->setSourceTables(eDnaAlphabet, dnaSubseqsTable_, dnaSeqsTable_);
        rnaSliceModel_->setTreeModel(adocTreeModel_);
        rnaSliceModel_->setSourceTables(eRnaAlphabet, rnaSubseqsTable_, rnaSeqsTable_);
    }

    if (!adocTreeModel_->setRoot(root))
    {
        adocDataSource_->close();

        // Show error message!

        return;
    }
}

void MainWindow::on_actionClose_triggered()
{
    if (!adocDataSource_ || !adocDataSource_->isOpen())
    {
        QMessageBox msgBox;
        msgBox.setText("File not open");
        msgBox.exec();
        return;
    }

    adocDataSource_->close();

    // Reset the tree model
    adocTreeModel_->setRoot(new AdocTreeNode(AdocTreeNode::RootType, "Root"));
}

void MainWindow::onMsaReady(Msa *msa, int /* tag */)
{
    if (!msaWindow_)
    {
        msaWindow_ = new MsaWindow;
        switch (msa->alphabet())
        {
        case eAminoAlphabet:
            ASSERT(aminoSubseqsTable_);
            ASSERT(aminoSeqsTable_);
            msaWindow_->msaSubseqModel()->setAnnotationTables(aminoSubseqsTable_, aminoSeqsTable_);
            break;
        case eDnaAlphabet:
            ASSERT(dnaSubseqsTable_);
            ASSERT(dnaSeqsTable_);
            msaWindow_->msaSubseqModel()->setAnnotationTables(dnaSubseqsTable_, dnaSeqsTable_);
            break;
        case eRnaAlphabet:
            ASSERT(rnaSubseqsTable_);
            ASSERT(rnaSeqsTable_);
            msaWindow_->msaSubseqModel()->setAnnotationTables(rnaSubseqsTable_, dnaSeqsTable_);
            break;

        default:
            delete msaWindow_;
            msaWindow_ = 0;

            qDebug() << "Unrecognized Msa alphabet";
            return;
        }

        // Temporary convenience to close main window when we are done
        connect(msaWindow_, SIGNAL(closed()), SLOT(close()));
    }

    msaWindow_->setMsa(msa);
    msaWindow_->show();
}

void MainWindow::onMsaError(const QString &error, int /* tag */)
{
    QMessageBox errorBox;
    errorBox.setText(error);
    errorBox.exec();
}
