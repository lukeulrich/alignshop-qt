/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QActionGroup>
#include <QtGui/QColor>
#include <QtGui/QFileDialog>
#include <QtGui/QFontMetrics>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoView>

#include "MsaWindow.h"
#include "ui_MsaWindow.h"

#include "../Commands/Msa/CollapseMsaRectLeftCommand.h"
#include "../Commands/Msa/CollapseMsaRectRightCommand.h"
#include "../Commands/Msa/TrimRowsLeftCommand.h"
#include "../Commands/Msa/TrimRowsRightCommand.h"
#include "../Commands/Msa/ExtendRowsLeftCommand.h"
#include "../Commands/Msa/ExtendRowsRightCommand.h"
#include "../Commands/Msa/InsertGapColumnsCommand.h"
#include "../Commands/Msa/LevelRowsLeftCommand.h"
#include "../Commands/Msa/LevelRowsRightCommand.h"
#include "../Commands/Msa/RemoveRowsCommand.h"
#include "../Commands/Msa/RemoveAllGapColumnsCommand.h"
#include "../Commands/Msa/SlideMsaRectCommand.h"

#include "../delegates/MsaLineEditDelegate.h"

#include "../models/MsaSubseqTableModel.h"

#include "../painting/gitems/LogoBarsItem.h"
#include "../painting/gitems/LogoItem.h"

#include "../widgets/MsaDataColumnWidget.h"
#include "../widgets/PercentSpinBox.h"
#include "../widgets/FontAndSizeChooser.h"

#include "../MsaTools/HandMsaTool.h"
#include "../MsaTools/SelectMsaTool.h"
#include "../MsaTools/ZoomMsaTool.h"
#include "../MsaTools/GapMsaTool.h"

#include "../gui_misc.h"        // For estimating the font size when rendering svg output
#include "../Services/HeaderColumnSelector.h"

#include "../../core/Entities/AbstractMsa.h"
#include "../../core/Entities/AbstractSeq.h"
#include "../../core/Entities/EntityFlags.h"
#include "../../core/Adoc.h"
#include "../../core/ObservableMsa.h"
#include "../../core/LiveMsaCharCountDistribution.h"
#include "../../core/LiveInfoContentDistribution.h"
#include "../../core/global.h"
#include "../../core/export/msa/FastaMsaExporter.h"
#include "../../core/export/msa/ClustalMsaExporter.h"


#include "../../graphics/ExactTextRenderer.h"
#include "../painting/SvgGeneratorEngine.h"
#include "../painting/NativeRenderEngine.h"


static const qreal kLabelRightMargin = 45.;   // Pixels

#include <QtDebug>


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adoc [Adoc *]
  * @param taskManager [TaskManager *]
  * @param parent [QWidget *]
  */
MsaWindow::MsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent) :
    QMainWindow(parent),
    undoStack_(nullptr),
    ui_(new Ui::MsaWindow),
    msaSubseqTableModel_(nullptr),
    logoAction_(nullptr),
    sequenceLabelsAction_(nullptr),
    adoc_(adoc),
    liveMsaCharCountDistribution_(nullptr),
    zoomSpinBox_(nullptr),
    locationLabel_(nullptr),
    saveImageFileDialog_(nullptr),
    exportFileDialog_(nullptr),
    msaToolGroup_(nullptr),
    handMsaTool_(nullptr),
    selectMsaTool_(nullptr),
    zoomMsaTool_(nullptr),
    taskManager_(taskManager),
    totalInfoContentLabel_(nullptr)
{
    undoStack_ = new QUndoStack(this);

    ui_->setupUi(this);

    // ---------------------
    // Add the font and zoom spin box
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui_->toolBar->addWidget(spacer);

    ui_->msaView->setFont(QFont("DejaVuSans", 9));

    // Font combo box
    FontAndSizeChooser *fontAndSizeChooser = new FontAndSizeChooser(this);
    fontAndSizeChooser->setCurrentFont(ui_->msaView->font());
    ui_->toolBar->addWidget(fontAndSizeChooser);
    connect(fontAndSizeChooser, SIGNAL(fontChanged(QFont)), ui_->msaView, SLOT(setFont(QFont)));

    // Stupid spacer widget to pad out the font combo box from the zoom spin box
    spacer = new QWidget();
    spacer->setFixedWidth(5);
    ui_->toolBar->addWidget(spacer);

    QLabel *zoomLabel = new QLabel("Zoom: ", this);
    ui_->toolBar->addWidget(zoomLabel);

    zoomSpinBox_ = new PercentSpinBox(this);
    zoomSpinBox_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    zoomSpinBox_->setKeyboardTracking(false);
    zoomSpinBox_->setSuffix("%");
    zoomSpinBox_->setDecimals(5);
    zoomSpinBox_->setStepFactor(ui_->msaView->zoomFactor());
    zoomSpinBox_->setRange(ui_->msaView->zoomMinimum() * 100., ui_->msaView->zoomMaximum() * 100.);
    zoomSpinBox_->setValue(ui_->msaView->zoom() * 100.);
    ui_->toolBar->addWidget(zoomSpinBox_);
    connect(zoomSpinBox_, SIGNAL(decimalPercentChanged(double)), ui_->msaView, SLOT(setZoom(double)));
    connect(ui_->msaView, SIGNAL(zoomChanged(double)), zoomSpinBox_, SLOT(setDecimalPercent(double)));
    connect(ui_->msaView, SIGNAL(viewportMarginsChanged(QMargins)), SLOT(onMsaViewMarginsChanged(QMargins)));
    connect(ui_->msaView->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(updateBottomMargin()));


    // ------------------------
    // Undo stack configuration
    ui_->msaView->setUndoStack(undoStack_);
    connect(ui_->actionUndo, SIGNAL(triggered()), undoStack_, SLOT(undo()));
    connect(ui_->actionRedo, SIGNAL(triggered()), undoStack_, SLOT(redo()));
    connect(undoStack_, SIGNAL(canUndoChanged(bool)), ui_->actionUndo, SLOT(setEnabled(bool)));
    connect(undoStack_, SIGNAL(canRedoChanged(bool)), ui_->actionRedo, SLOT(setEnabled(bool)));
    connect(undoStack_, SIGNAL(cleanChanged(bool)), SLOT(onUndoCleanChanged(bool)));
    connect(undoStack_, SIGNAL(indexChanged(int)), SLOT(enableDisableActions()));


    // ----------------
    // Undo dock widget
    QDockWidget *undoHistoryDockWidget = new QDockWidget("Undo History (Shift+Ctrl+H)", this);
    undoHistoryDockWidget->setWidget(new QUndoView(undoStack_));
    addDockWidget(Qt::RightDockWidgetArea, undoHistoryDockWidget);
    undoHistoryDockWidget->setVisible(false);


    // -----------------------------
    // Subseq table view dock widget
    ui_->subseqTableDockWidget->hide();
    new HeaderColumnSelector(ui_->subseqTableView->horizontalHeader(), ui_->subseqTableView);


    // ---------------------------
    // Msa view tweaks
    // o Remove the frame border from the MsaView
    ui_->msaView->setFrameShape(QFrame::NoFrame);
    // o Prevent collapse of MsaView
    ui_->horizontalSplitter->setCollapsible(1, false);
    // o Prevent collapse via the vertical splitter
    ui_->verticalSplitter->setCollapsible(0, false);


    // -----------------------------------
    // Show/hide controls for dock widgets
    QAction *toggleSequenceTableAction = ui_->subseqTableDockWidget->toggleViewAction();
    toggleSequenceTableAction->setIcon(QIcon(":/aliases/images/icons/view-form-table"));
    ui_->menu_View->addAction(toggleSequenceTableAction);

    // Logo
    // Initially the logo panel is hidden - calling the toggle method with a false value hides it
    setLogoCollapsed(true);
    logoAction_ = new QAction("Logo", this);
    logoAction_->setCheckable(true);
    ui_->menu_View->addAction(logoAction_);
    connect(logoAction_, SIGNAL(triggered()), SLOT(onLogoActionTriggered()));

    // Sequence labels
    sequenceLabelsAction_ = new QAction("Labels", this);
    sequenceLabelsAction_->setCheckable(true);
    ui_->menu_View->addAction(sequenceLabelsAction_);
    connect(sequenceLabelsAction_, SIGNAL(triggered()), SLOT(onSequenceLabelsActionTriggered()));

    // Start, stop, ruler widgets
    ui_->menu_View->addSeparator();
    QAction *toggleStartSideWidgetAction = new QAction("Start positions", this);
    toggleStartSideWidgetAction->setCheckable(true);
    toggleStartSideWidgetAction->setChecked(true);
    connect(toggleStartSideWidgetAction, SIGNAL(toggled(bool)), ui_->msaView, SLOT(setStartSideWidgetVisible(bool)));
    ui_->menu_View->addAction(toggleStartSideWidgetAction);

    QAction *toggleStopSideWidgetAction = new QAction("Stop positions", this);
    toggleStopSideWidgetAction->setCheckable(true);
    toggleStopSideWidgetAction->setChecked(true);
    connect(toggleStopSideWidgetAction, SIGNAL(toggled(bool)), ui_->msaView, SLOT(setStopSideWidgetVisible(bool)));
    ui_->menu_View->addAction(toggleStopSideWidgetAction);

    QAction *toggleMsaRulerAction = new QAction(QIcon(":aliases/images/icons/ruler"), "Msa Ruler", this);
    toggleMsaRulerAction->setCheckable(true);
    toggleMsaRulerAction->setChecked(true);
    connect(toggleMsaRulerAction, SIGNAL(toggled(bool)), ui_->msaView, SLOT(setMsaRulerVisible(bool)));
    ui_->menu_View->addAction(toggleMsaRulerAction);


    // History action
    ui_->menu_View->addSeparator();
    QAction *toggleUndoHistoryAction = undoHistoryDockWidget->toggleViewAction();
    toggleUndoHistoryAction->setText("Undo History");
    toggleUndoHistoryAction->setShortcut(QKeySequence("Shift+Ctrl+H"));
    toggleUndoHistoryAction->setIcon(QIcon(":/aliases/images/icons/view-history"));
    ui_->menu_View->addAction(toggleUndoHistoryAction);


    // ----------------
    // View menu tweaks
    // Namely for updating the checked status based on if it has a non-zero height
    connect(ui_->menu_View, SIGNAL(aboutToShow()), SLOT(onViewMenuAboutToShow()));


    // -------------------------------
    // Msa subseq table model and view
    msaSubseqTableModel_ = new MsaSubseqTableModel(this);
    msaSubseqTableModel_->setUndoStack(undoStack_);
    ui_->subseqTableView->setModel(msaSubseqTableModel_);
    ui_->subseqTableView->setItemDelegate(new LineEditDelegate(this));


    // -------------------------------
    // Set the initial size of the labels for the graphics view
    ui_->horizontalSplitter->setSizes(QList<int>() << 175 << 1);


    // ------------------------
    // Action signals
    connect(ui_->actionSave, SIGNAL(triggered()), SLOT(save()));
    connect(ui_->actionSaveImage, SIGNAL(triggered()), SLOT(onActionSaveImage()));
    connect(ui_->actionExport, SIGNAL(triggered()), SLOT(onActionExport()));
    connect(ui_->actionClose, SIGNAL(triggered()), SLOT(close()));
    connect(ui_->actionCollapseLeft, SIGNAL(triggered()), SLOT(onActionCollapseLeft()));
    connect(ui_->actionCollapseRight, SIGNAL(triggered()), SLOT(onActionCollapseRight()));
    connect(ui_->actionTrimRowsLeft, SIGNAL(triggered()), SLOT(onActionTrimRowsLeft()));
    connect(ui_->actionTrimRowsRight, SIGNAL(triggered()), SLOT(onActionTrimRowsRight()));
    connect(ui_->actionExtendRowsLeft, SIGNAL(triggered()), SLOT(onActionExtendRowsLeft()));
    connect(ui_->actionExtendRowsRight, SIGNAL(triggered()), SLOT(onActionExtendRowsRight()));
    connect(ui_->actionLevelRowsLeft, SIGNAL(triggered()), SLOT(onActionLevelRowsLeft()));
    connect(ui_->actionLevelRowsRight, SIGNAL(triggered()), SLOT(onActionLevelRowsRight()));
    connect(ui_->actionRemoveAllGapColumns, SIGNAL(triggered()), SLOT(onActionRemoveAllGapColumns()));
    connect(ui_->actionSelectAll, SIGNAL(triggered()), SLOT(onActionSelectAll()));
    connect(ui_->actionDeselectAll, SIGNAL(triggered()), SLOT(onActionDeselectAll()));

    connect(ui_->actionZoomIn, SIGNAL(triggered()), SLOT(onActionZoomIn()));
    ui_->actionZoomIn->setShortcuts(QList<QKeySequence>() << QString("=") << QString("+"));
    connect(ui_->actionZoomOut, SIGNAL(triggered()), SLOT(onActionZoomOut()));
    connect(ui_->actionZoom1_1, SIGNAL(triggered()), SLOT(onActionZoom1_1()));
    connect(ui_->actionZoom1_2, SIGNAL(triggered()), SLOT(onActionZoom1_2()));
    connect(ui_->actionZoom2_1, SIGNAL(triggered()), SLOT(onActionZoom2_1()));

    connect(ui_->msaView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onMsaViewContextMenuRequested(QPoint)));

    // --------------------------------------
    // Msa tools
    handMsaTool_ = new HandMsaTool(ui_->msaView, this);

    selectMsaTool_ = new SelectMsaTool(ui_->msaView, this);
    selectMsaTool_->setHandMsaTool(handMsaTool_);
    connect(selectMsaTool_, SIGNAL(selectionFinished(PosiRect)), SLOT(enableDisableActions()));
    connect(selectMsaTool_, SIGNAL(selectionCleared()), SLOT(enableDisableActions()));
    connect(selectMsaTool_, SIGNAL(slideStarted(PosiRect)), SLOT(onMsaSlideStarted(PosiRect)));
    connect(selectMsaTool_, SIGNAL(slideFinished(PosiRect)), SLOT(onMsaSlideFinished(PosiRect)));

    zoomMsaTool_ = new ZoomMsaTool(ui_->msaView, this);

    gapMsaTool_ = new GapMsaTool(ui_->msaView, this);
    connect(gapMsaTool_, SIGNAL(gapColumnsInsertFinished(ClosedIntRange,bool)), SLOT(onMsaGapColumnsInsertFinished(ClosedIntRange,bool)));

    connect(ui_->action_HandTool, SIGNAL(triggered()), SLOT(onActionHandTool()));
    connect(ui_->action_SelectTool, SIGNAL(triggered()), SLOT(onActionSelectTool()));
    connect(ui_->action_ZoomTool, SIGNAL(triggered()), SLOT(onActionZoomTool()));
    connect(ui_->action_GapTool, SIGNAL(triggered()), SLOT(onActionGapTool()));

    // Action group for the msa tools
    msaToolGroup_ = new QActionGroup(this);
    msaToolGroup_->addAction(ui_->action_HandTool);
    msaToolGroup_->addAction(ui_->action_SelectTool);
    msaToolGroup_->addAction(ui_->action_ZoomTool);
    msaToolGroup_->addAction(ui_->action_GapTool);
    ui_->action_SelectTool->setChecked(true);

    ui_->msaView->setCurrentMsaTool(selectMsaTool_);

    // 0. Undo
    // 1. Redo
    // 2. Separator
    // 3. Separator
    // 4. ...
    //
    // Insert in reverse order that they appear on the menu so that the extend sequence is first
    QList<QAction *> editActions = ui_->menu_Edit->actions();
    ui_->menu_Edit->insertAction(editActions.at(3), selectMsaTool_->trimSequenceAction());
    ui_->menu_Edit->insertAction(selectMsaTool_->trimSequenceAction(), selectMsaTool_->extendSequenceAction());


    // --------------------------
    // Msa vertical selection bar
    ui_->msaVertSelectionBar->setMsaView(ui_->msaView);


    // ------------------------
    // Configure the status bar
    locationLabel_ = new QLabel();
    statusBar()->addWidget(locationLabel_);
    connect(ui_->msaView, SIGNAL(mouseCursorMoved(QPoint,QPoint)), SLOT(onMouseCursorPointMoved(QPoint)));
    connect(ui_->msaView, SIGNAL(viewportMouseLeft()), SLOT(onViewportMouseLeft()));
    connect(ui_->msaView, SIGNAL(selectionChanged(PosiRect,PosiRect)), SLOT(onMsaViewSelectionChanged(PosiRect)));


    // ---------------
    // Intercept events from the label view
    ui_->labelView->installEventFilter(this);

    // Prevent the following msa window shortcuts from occurring when the labelView is in view. This is primarily to
    // ease editing items simply by pressing a key immediately.
    ui_->labelView->setIgnoreShortcutOverrides(actionShortCuts());

    // Ignore the two keys used to zoom in and out; do not use the shortcut version because it is tricky to deal with
    // the + sign, which requires the shift modifier to push, yet when telling the label view to ignore the plus sign,
    // the shift modifier is not included.
    ui_->labelView->setIgnoreShortcutKey('=');
    ui_->labelView->setIgnoreShortcutKey('+');

    // Use a custom delegate that pre-fills the line edit with text when editing is triggered
    ui_->labelView->setItemDelegate(new MsaLineEditDelegate(ui_->labelView));


    // ---------------------
    // Logo view - handle context menu
    connect(ui_->logoGraphicsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onLogoContextMenuRequested(QPoint)));


    // ----
    // Logo
    // Currently it is not linked to a live info content distribution, does not have a parent item, but is a child
    // of this window
    logoItem_ = new LogoItem();
    logoScene_ = new QGraphicsScene(this);
    logoScene_->addItem(logoItem_);
    ui_->logoGraphicsView->setScene(logoScene_);
    logoItem_->setMsaView(ui_->msaView);


    // Make the msa view occupy the majority of the screen
    ui_->verticalSplitter->setStretchFactor(0, 1);


    // Info content label
    totalInfoContentLabel_ = new QLabel;
    totalInfoContentLabel_->setToolTip("Total Information Content");
    totalInfoContentLabel_->setAlignment(Qt::AlignRight);
    statusBar()->addWidget(totalInfoContentLabel_, 1);

    // -----------
    // Final setup
    setWindowTitle(titleString());
}

/**
  */
MsaWindow::~MsaWindow()
{
    if (abstractMsa_ != nullptr)
    {
        msaSubseqTableModel_->setMsa(nullptr, nullptr);
        ui_->msaView->setMsa(nullptr);

        if (abstractMsa_->isNew() == false)
            adoc_->msaRepository(abstractMsa_)->unloadAlignment(abstractMsa_);

        // Once dead, subseqs are always dead :)
        adoc_->msaRepository(abstractMsa_)->unloadDeadSubseqs(abstractMsa_);
    }

    delete ui_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Adoc *
  */
Adoc *MsaWindow::adoc() const
{
    return adoc_;
}

/**
  * @returns bool
  */
bool MsaWindow::isModified() const
{
    return !undoStack_->isClean();
}

/**
  * @returns LiveMsaCharCountDistribution *
  */
LiveMsaCharCountDistribution *MsaWindow::liveMsaCharCountDistribution() const
{
    return liveMsaCharCountDistribution_;
}

/**
  * @returns LogoItem *
  */
LogoItem *MsaWindow::logoItem() const
{
    return logoItem_;
}

/**
  * @returns ObservableMsa *
  */
ObservableMsa *MsaWindow::msa() const
{
    if (!abstractMsa_)
        return nullptr;

    ASSERT(abstractMsa_->msa() != nullptr);

    return abstractMsa_->msa();
}

/**
  * @returns AbstractMsa *
  */
AbstractMsaSPtr MsaWindow::msaEntity() const
{
    return abstractMsa_;
}

/**
  * @param abstractMsa [AbstractMsa *]
  */
void MsaWindow::setMsaEntity(const AbstractMsaSPtr &abstractMsa)
{
    delete liveMsaCharCountDistribution_;
    liveMsaCharCountDistribution_ = nullptr;

    ui_->actionExport->setEnabled(false);
    ui_->actionSaveImage->setEnabled(false);

    // Save a pointer to the old entity so we can unload it after we have finished loading the new msa entity
    // Cannot use a reference here because otherwise when abstractMsa_ is updated, oldMsaEntity will be concurrently
    // updated as well.
    AbstractMsaSPtr oldMsaEntity = abstractMsa_;

    abstractMsa_ = abstractMsa;

    if (abstractMsa_)
    {
        ASSERT(abstractMsa_->msa() != nullptr);
        ASSERT(abstractMsa_->msa()->rowCount() > 0);

        connect(abstractMsa_->msa(), SIGNAL(rowsSorted()), ui_->msaView->viewport(), SLOT(update()));

        enableDisableActions();
        setWindowTitle(titleString());

        liveMsaCharCountDistribution_ = new LiveMsaCharCountDistribution(abstractMsa_->msa(), this);

        msaSubseqTableModel_->setMsa(abstractMsa_->msa(), subseqEntityColumnAdapter());

        // It is vital for proper rendering to assign the observable msa to the view *last*. This is because the view
        // handles painting, which should occur *after* all data updates have been performed. Just like many of the
        // data methods, the view watches the msa for changes and then issues updates in response; however, if these
        // happen before the data models (e.g. symbol string calculator) have been updated, the visualization may be
        // incorrect.
        //
        // This specifically happened (not any more) when an entire column(s) were selected, trimmed, and then undone.
        // Originally, the view was assigned the msa first before the live msa char count distribution. This caused the
        // view to update first, and then the msa char count distribution. Since the view was using clustal colors which
        // which depend upon the char counts, the view was displaying colors based upon erroneous counts.
        ui_->msaView->setMsa(abstractMsa->msa());

        // ------------------------
        // Configure the label view
        ui_->labelView->setMsaView(ui_->msaView);
        ui_->labelView->setModel(msaSubseqTableModel_);

        // Prime the focus on the alignment section (vs the msa subseq table view)
        ui_->msaView->setFocus();

        // Enable the export action
        ui_->actionExport->setEnabled(true);
        ui_->actionSaveImage->setEnabled(true);
    }

    ui_->subseqTableView->resizeColumnToContents(MsaSubseqTableModel::eStartColumn);
    ui_->subseqTableView->resizeColumnToContents(MsaSubseqTableModel::eStopColumn);

    if (oldMsaEntity)
    {
        if (oldMsaEntity->isNew() == false)
            adoc_->msaRepository(oldMsaEntity)->unloadAlignment(oldMsaEntity);
        adoc_->msaRepository(oldMsaEntity)->unloadDeadSubseqs(oldMsaEntity);

        disconnect(oldMsaEntity->msa(), SIGNAL(rowsSorted()), ui_->msaView->viewport(), SLOT(update()));
    }

    setMsaEntityImpl(abstractMsa);
    if (abstractMsa_)
    {
        ASSERT(liveInfoContentDistribution() != nullptr);
        connect(liveInfoContentDistribution(), SIGNAL(dataChanged(ClosedIntRange)), SLOT(updateTotalInfoLabel()));
        connect(liveInfoContentDistribution(), SIGNAL(columnsInserted(ClosedIntRange)), SLOT(updateTotalInfoLabel()));
        connect(liveInfoContentDistribution(), SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(updateTotalInfoLabel()));
    }
    updateTotalInfoLabel();
}

/**
  * Subclasses should return a valid pointer here if they want too.
  *
  * @returns IColumnAdapter *
  */
IColumnAdapter *MsaWindow::subseqEntityColumnAdapter() const
{
    return nullptr;
}

/**
  * @returns TaskManager *
  */
TaskManager *MsaWindow::taskManager() const
{
    return taskManager_;
}

/**
  * @returns QString
  */
QString MsaWindow::titleString() const
{
    if (abstractMsa_ == nullptr)
        return "Alignment Editor";

    QString text = abstractMsa_->name();
    if (abstractMsa_->isDirty() || abstractMsa_->isNew() || isModified())
        text += "*";
    text += " - Alignment Editor";
    return text;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @returns bool
  */
bool MsaWindow::save()
{
    updateEntitiesStartStop();

    // TODO: Check if msa is actually dirty
    abstractMsa_->setDirty(Ag::eCoreDataFlag, true);
    adoc_->adocSource()->begin();
    bool saveSuccessful = adoc_->repository(static_cast<EntityType>(abstractMsa_->type()))->save(abstractMsa_->id());
    adoc_->adocSource()->end();

    if (saveSuccessful)
        undoStack_->setClean();
    else
    {
        QMessageBox::warning(this, "Error saving alignment",
                             "Unable to save alignment. Please try again.",
                             QMessageBox::Ok);
    }

    return saveSuccessful;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected slots
/**
  */
void MsaWindow::updateLogoSceneRect()
{
    ui_->logoGraphicsView->setSceneRect(logoItem_->boundingRect());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param closeEvent [QCloseEvent *]
  */
void MsaWindow::closeEvent(QCloseEvent *closeEvent)
{
    // Use the undo stack to see if this alignment is clean and able to be saved
    if (!undoStack_->isClean())
    {
        activateWindow();
        raise();

        QMessageBox msgBox(this);
        msgBox.setText("The alignment has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.button(QMessageBox::Save)->setText("Save changes");
        switch(msgBox.exec())
        {
        case QMessageBox::Save:
            if (!save())
            {
                closeEvent->ignore();
                return;
            }
            break;
        case QMessageBox::Cancel:
            closeEvent->ignore();
            return;

        default:
            break;
        }
    }

    // Special case: user opens alignment, makes changes to the textual items, closes alignment without saving changes,
    // re-opens the same alignment. All textual changes will not have been undone.
    // ISSUE? What if the undo stack is huge?
    while (undoStack_->canUndo())
        undoStack_->undo();

    // It is vital to clear the stack before emitting the close signal so that all commands may properly clean up.
    // Specifically, this is critical for any RemoveRowsCommand's in the undo stack, because they pass ownership of any
    // dead subseqs to the Msa entity.
    undoStack_->clear();

    emit aboutToClose(this);
    closeEvent->accept();
}

/**
  * @param object [QObject *]
  * @param event [QEvent *]
  * @returns bool
  */
bool MsaWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui_->labelView)
        return labelViewEventFilter(event);

    return QObject::eventFilter(object, event);
}

QList<QKeySequence> MsaWindow::actionShortCuts() const
{
    QList<QKeySequence> shortcuts;
    shortcuts << ui_->actionTrimRowsLeft->shortcut()
              << ui_->actionTrimRowsRight->shortcut()
              << ui_->actionExtendRowsLeft->shortcut()
              << ui_->actionExtendRowsRight->shortcut()
              << ui_->actionLevelRowsLeft->shortcut()
              << ui_->actionLevelRowsRight->shortcut()
              << ui_->actionCollapseLeft->shortcut()
              << ui_->actionCollapseRight->shortcut()
              << ui_->actionSelectAll->shortcut()
              << ui_->actionDeselectAll->shortcut()
              << ui_->actionZoomOut->shortcut()
              << ui_->actionZoomIn->shortcut()
              << ui_->actionZoom1_1->shortcut()
              << ui_->actionZoom1_2->shortcut()
              << selectMsaTool_->trimSequenceAction()->shortcut()
              << selectMsaTool_->extendSequenceAction()->shortcut();
    return shortcuts;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void MsaWindow::onActionSaveImage()
{
    ASSERT(abstractMsa_);

    if (saveImageFileDialog_ == nullptr)
    {
        saveImageFileDialog_ = new QFileDialog(this,
                                               "Select file to save as SVG",
                                               adoc_->documentDirectory().absolutePath());
        saveImageFileDialog_->setNameFilters(imageNameFilters());
        saveImageFileDialog_->setLabelText(QFileDialog::Accept, "Save");
    }
    if (!saveImageFileDialog_->exec())
        return;
    QString selectedNameFilter = saveImageFileDialog_->selectedNameFilter();
    QString defaultSuffix = defaultSuffixForNameFilter(selectedNameFilter);
    ASSERT(!defaultSuffix.isEmpty());
    QString fileName = saveImageFileDialog_->selectedFiles().first();
    QString suffix = filenameSuffix(fileName);
    if (suffix.isEmpty())
        fileName += "." + defaultSuffix;

    if (QFile::exists(fileName))
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Confirm overwrite");
        msgBox.setText(QString("The file, %1, already exists. Continuing will overwrite this file. Are you sure you "
                               "wish to proceed?").arg(fileName));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Yes)->setText("Overwrite");
        msgBox.setDefaultButton(QMessageBox::Cancel);
        if (!msgBox.exec())
            return;
    }

    PosiRect msaRegion = (!ui_->msaView->selection().isNull()) ? ui_->msaView->selection().normalized()
                                                               : PosiRect(1, 1, abstractMsa_->msa()->columnCount(), abstractMsa_->msa()->rowCount());
    if (defaultSuffix == "svg")
    {
        saveMsaRegionAsSvg(fileName, msaRegion);
    }
    else if (defaultSuffix == "png" ||
             defaultSuffix == "bmp" ||
             defaultSuffix == "jpg" ||
             defaultSuffix == "tif")
    {
        saveMsaRegionAsImage(fileName, msaRegion);
    }
}

/**
  */
void MsaWindow::onActionExport()
{
    ASSERT(abstractMsa_);

    if (exportFileDialog_ == nullptr)
    {
        // Setup of the export file dialog
        exportFileDialog_ = new QFileDialog(this,
                                            "Export to File",
                                            adoc_->documentDirectory().absolutePath());

        exportFileDialog_->setLabelText(QFileDialog::Accept, "Export");
        exportFileDialog_->setLabelText(QFileDialog::FileType, "Format");

        exportFileDialog_->setFilters(QStringList()
                                      << "Aligned FASTA (*.afa)"
                                      << "CLUSTAL (*.aln)");
        exportFileDialog_->setDefaultSuffix("afa");

        connect(exportFileDialog_, SIGNAL(filterSelected(QString)), SLOT(onExportFileDialogFilterSelected(QString)));
    }

    // Get a file name from the user
    exportFileDialog_->selectFile(abstractMsa_->name());

    QString exportFileName;
    while (exportFileName.isEmpty())
    {
        if (!exportFileDialog_->exec())
            return;

        exportFileName = exportFileDialog_->selectedFiles().first();
        ASSERT(exportFileName.isEmpty() == false);

        if (!QFile::exists(exportFileName))
            break;

        // Ask user what to do:
        // 1) Cancel
        // 2) Continue
        // 3) Select another file
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Confirm overwrite");
        msgBox.setText(QString("The file, %1, already exists. Continuing will overwrite this file. Are you sure you "
                               "wish to proceed?").arg(exportFileName));
        msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Yes)->setText("Overwrite");
        msgBox.button(QMessageBox::Retry)->setText("Choose different filename");
        msgBox.setDefaultButton(QMessageBox::Retry);
        int retval = msgBox.exec();
        if (retval == QMessageBox::Cancel)
            return;
        if (retval == QMessageBox::Yes)
            break;
        if (retval == QMessageBox::Retry)
            exportFileName.clear();

        // Otherwise, Choose another filename
    }

    // ---------------------------
    // Got a file name, let's export this baby
    QFile exportFile(exportFileName);
    if (!exportFile.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this,
                             "Export Error",
                             QString("Unable to open the file, %1, for writing: %2")
                             .arg(exportFileName)
                             .arg(exportFile.errorString()),
                             QMessageBox::Ok);
        return;
    }

    int index = exportFileDialog_->nameFilters().indexOf(exportFileDialog_->selectedFilter());
    QScopedPointer<IMsaExporter> msaExporter;
    if (index == 0)
        msaExporter.reset(new FastaMsaExporter);
    else
        msaExporter.reset(new ClustalMsaExporter);
    try
    {
        msaExporter->exportMsa(*abstractMsa_, exportFile);
    }
    catch (const char *errorMessage)
    {
        QMessageBox::warning(this,
                             "Export Error",
                             QString("The following exception occurred during the export operation: %1").arg(errorMessage),
                             QMessageBox::Close);
        return;
    }
    catch (...)
    {
        QMessageBox::warning(this,
                             "Export Error",
                             "An unknown exception occurred during the export operation",
                             QMessageBox::Close);
        return;
    }
}

/**
  * Update the default suffix that is applied.
  *
  * @param nameFilter [const QString &]
  */
void MsaWindow::onExportFileDialogFilterSelected(const QString &nameFilter)
{
    // TODO: Refactor this to a more flexible design
    int index = exportFileDialog_->nameFilters().indexOf(nameFilter);
    exportFileDialog_->setDefaultSuffix((index == 0) ? "afa" : "aln");
}

/**
  */
void MsaWindow::onActionTrimRowsLeft()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.right();
    undoStack_->push(new TrimRowsLeftCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionTrimRowsRight()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.left();
    undoStack_->push(new TrimRowsRightCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionExtendRowsLeft()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.left();
    undoStack_->push(new ExtendRowsLeftCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionExtendRowsRight()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.right();
    undoStack_->push(new ExtendRowsRightCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionLevelRowsLeft()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.left();
    undoStack_->push(new LevelRowsLeftCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionLevelRowsRight()
{
    PosiRect selection = ui_->msaView->selection().normalized();
    int msaColumn = selection.right();
    undoStack_->push(new LevelRowsRightCommand(abstractMsa_->msa(), msaColumn, selection.verticalRange()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionRemoveAllGapColumns()
{
    QVector<ClosedIntRange> removedColumns = abstractMsa_->msa()->removeGapColumns();
    if (removedColumns.isEmpty())
        return;

    undoStack_->push(new RemoveAllGapColumnsCommand(abstractMsa_->msa(), removedColumns));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionCollapseLeft()
{
    undoStack_->push(new CollapseMsaRectLeftCommand(abstractMsa_->msa(), ui_->msaView->selection()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionCollapseRight()
{
    undoStack_->push(new CollapseMsaRectRightCommand(abstractMsa_->msa(), ui_->msaView->selection()));
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionSelectAll()
{
    ui_->msaView->selectAll();
    enableDisableActions();
}

/**
  */
void MsaWindow::onActionDeselectAll()
{
    ui_->msaView->clearSelection();
    enableDisableActions();
}

/**
  * Zoom actions!
  */
void MsaWindow::onActionZoomIn()     {    ui_->msaView->setZoomBy(1);   }
void MsaWindow::onActionZoomOut()    {    ui_->msaView->setZoomBy(-1);  }
void MsaWindow::onActionZoom1_1()    {    ui_->msaView->setZoom(1.);    }
void MsaWindow::onActionZoom1_2()    {    ui_->msaView->setZoom(.5);    }
void MsaWindow::onActionZoom2_1()    {    ui_->msaView->setZoom(2.);    }

/**
  */
void MsaWindow::onActionHandTool()
{
    ui_->msaView->setCurrentMsaTool(handMsaTool_);
}

/**
  */
void MsaWindow::onActionSelectTool()
{
    ui_->msaView->setCurrentMsaTool(selectMsaTool_);
}

/**
  */
void MsaWindow::onActionZoomTool()
{
    ui_->msaView->setCurrentMsaTool(zoomMsaTool_);
}

/**
  */
void MsaWindow::onActionGapTool()
{
    ui_->msaView->setCurrentMsaTool(gapMsaTool_);
}

/**
  */
void MsaWindow::enableDisableActions()
{
    ObservableMsa *msa = abstractMsa_->msa();
    PosiRect selection = ui_->msaView->selection().normalized();

    ui_->actionCollapseLeft->setEnabled(msa->canCollapseLeft(selection));
    ui_->actionCollapseRight->setEnabled(msa->canCollapseRight(selection));
    ui_->actionExtendRowsLeft->setEnabled(msa->canExtendLeft(selection.left(), selection.verticalRange()));
    ui_->actionExtendRowsRight->setEnabled(msa->canExtendRight(selection.right(), selection.verticalRange()));
    ui_->actionTrimRowsLeft->setEnabled(msa->canTrimLeft(selection.right(), selection.verticalRange()));
    ui_->actionTrimRowsRight->setEnabled(msa->canTrimRight(selection.left(), selection.verticalRange()));
    ui_->actionLevelRowsLeft->setEnabled(msa->canLevelLeft(selection.left(), selection.verticalRange()));
    ui_->actionLevelRowsRight->setEnabled(msa->canLevelRight(selection.right(), selection.verticalRange()));
}

/**
  * @param columns [const ClosedIntRange &columns]
  * @param normal [bool]
  */
void MsaWindow::onMsaGapColumnsInsertFinished(const ClosedIntRange &columns, bool normal)
{
    Q_UNUSED(normal);

    if (columns.isEmpty())
        return;

    undoStack_->push(new InsertGapColumnsCommand(abstractMsa_->msa(),
                                                 columns.begin_,
                                                 columns.length()));
    enableDisableActions();
}

/**
  * @param msaRect [const PosiRect &]
  */
void MsaWindow::onMsaSlideStarted(const PosiRect &msaRect)
{
    slideStartRect_ = msaRect;
}

/**
  * @param msaRect [const PosiRect &]
  */
void MsaWindow::onMsaSlideFinished(const PosiRect &msaRect)
{
    if (slideStartRect_ == msaRect)
        return;

    undoStack_->push(new SlideMsaRectCommand(ui_->msaView,
                                             abstractMsa_->msa(),
                                             slideStartRect_,
                                             msaRect.left() - slideStartRect_.left()));
    enableDisableActions();
}

void MsaWindow::updateTotalInfoLabel()
{
    totalInfoContentLabel_->setText(QString("Total IC: %1").arg(QString::number(liveInfoContentDistribution()->totalInfo(), 'f', 2)));
}

void MsaWindow::onLogoContextMenuRequested(const QPoint &point)
{
    if (liveInfoContentDistribution() == nullptr)
        return;

    QMenu menu;
    QAction *toggleIcColumnLabelsAction = menu.addAction("Show IC per column");
    toggleIcColumnLabelsAction->setCheckable(true);
    toggleIcColumnLabelsAction->setChecked(logoItem()->logoBarsItem()->areColumnIcLabelsVisible());
    QAction *actualAction = menu.exec(ui_->logoGraphicsView->viewport()->mapToGlobal(point));
    if (!actualAction)
        return;

    if (actualAction == toggleIcColumnLabelsAction)
        logoItem()->logoBarsItem()->setColumnIcLabelsVisible(toggleIcColumnLabelsAction->isChecked());
}

/**
  * @param isClean [bool]
  */
void MsaWindow::onUndoCleanChanged(bool isClean)
{
    ui_->actionSave->setEnabled(!isClean);
    setWindowTitle(titleString());
}

/**
  */
void MsaWindow::onViewportMouseLeft()
{
    locationLabel_->clear();
}

/**
  * @param msaPoint [const QPoint &]
  */
void MsaWindow::onMouseCursorPointMoved(const QPoint &msaPoint)
{
    // If a selection is active, then give it precedence when updating the status bar
    if (selectMsaTool_->isActive())
        return;

    locationLabel_->setText(QString("(%1, %2)").arg(msaPoint.x()).arg(msaPoint.y()));
}

/**
  * This perhaps should belong in the msaView... or perhaps not depending on if there are MsaWindow specific things
  * that should appear in the menu.
  *
  * @param msaViewPoint [const QPoint &]
  */
void MsaWindow::onMsaViewContextMenuRequested(const QPoint &msaViewPoint)
{
    QMenu menu;
    QAction *removeSelectedAction = menu.addAction("Remove selected sequences");

    if (ui_->msaView->selection().isNull() ||
        qAbs(ui_->msaView->selection().height()) == ui_->msaView->msa()->rowCount())
    {
        removeSelectedAction->setEnabled(false);
    }
    QAction *actualAction = menu.exec(ui_->msaView->viewport()->mapToGlobal(msaViewPoint));
    if (!actualAction)
        return;

    if (actualAction == removeSelectedAction)
        undoStack_->push(new RemoveRowsCommand(abstractMsa_, ui_->msaView->selection().normalized().verticalRange()));
}

/**
  * @param selection [const PosiRect &]
  */
void MsaWindow::onMsaViewSelectionChanged(const PosiRect &selection)
{
    if (!selectMsaTool_->isActive())
        return;

    locationLabel_->setText(QString("(%1, %2) -> (%3, %4) [%5 x %6]")
                            .arg(selection.left())
                            .arg(selection.top())
                            .arg(selection.right())
                            .arg(selection.bottom())
                            .arg(qAbs(selection.width()))
                            .arg(qAbs(selection.height())));
}

/**
  */
void MsaWindow::onLogoActionTriggered()
{
    setLogoCollapsed(ui_->verticalSplitter->sizes().at(1) > 0);
}

/**
  */
void MsaWindow::onSequenceLabelsActionTriggered()
{
    setSequenceLabelsCollapsed(ui_->horizontalSplitter->sizes().at(0) > 0);
}

/**
  * Update the checked status of the logo action
  */
void MsaWindow::onViewMenuAboutToShow()
{
    logoAction_->setChecked(ui_->verticalSplitter->sizes().at(1) > 0);
    sequenceLabelsAction_->setChecked(ui_->horizontalSplitter->sizes().at(0) > 0);
}

/**
  * Set the label view's layout's bottom margin to equal the horizontal scroll bar height if visible of the msaview.
  * Additionally, adjust the size of the bottomMsaMarginWidget.
  */
void MsaWindow::onMsaViewMarginsChanged(const QMargins &margins)
{
    int bottomMargin = (ui_->msaView->horizontalScrollBar()->minimum() != ui_->msaView->horizontalScrollBar()->maximum()) ? ui_->msaView->horizontalScrollBar()->height() : 0;
    ui_->holder->setContentsMargins(0, margins.top(), 0, bottomMargin);

    // Now set the height of the bottomMarginWidgetContainer
    ui_->bottomMarginWidgetContainer_->setFixedHeight(margins.bottom());
}

void MsaWindow::updateBottomMargin()
{
    QMargins margins = ui_->holder->contentsMargins();
    int bottomMargin = (ui_->msaView->horizontalScrollBar()->minimum() != ui_->msaView->horizontalScrollBar()->maximum()) ? ui_->msaView->horizontalScrollBar()->height() : 0;
    if (bottomMargin != margins.bottom())
        ui_->holder->setContentsMargins(0, margins.top(), 0, bottomMargin);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param event [QEvent *]
  * @returns bool
  */
bool MsaWindow::labelViewEventFilter(QEvent *event)
{
    if (event->type() == QKeyEvent::KeyPress &&
        ui_->labelView->model() != nullptr)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete)
        {
            // Check for any selected sequences
            QModelIndexList selectedIndexes = ui_->labelView->selectionModel()->selectedIndexes();
            if (selectedIndexes.isEmpty())
                return false;

            ASSERT(selectedIndexes.at(0).row() <= selectedIndexes.last().row());

            // Prevent removing all sequences
            bool allRowsSelected = selectedIndexes.first().row() == 0 &&
                                   selectedIndexes.last().row() == ui_->labelView->model()->rowCount() - 1;
            if (allRowsSelected)
            {
                QMessageBox::warning(this,
                                     "Error removing sequences",
                                     "Removing all sequences is not allowed. "
                                     "Reduce your selection and try again or if you want to delete the alignment "
                                     "exit this editor window and delete this alignment node from the main document "
                                     "interface.",
                                     QMessageBox::Ok);
                return false;
            }

            undoStack_->push(new RemoveRowsCommand(abstractMsa_, ClosedIntRange(selectedIndexes.first().row() + 1,
                                                                                selectedIndexes.last().row() + 1)));
            return true;
        }
    }

    return false;
}

/**
  * @param collapsed [bool]
  */
void MsaWindow::setLogoCollapsed(bool collapsed)
{
    // Note: Currently only the logo is visible in the tabbed widget; so we are simply collapsing and uncollapsing it
    // as a whole here rather than the actual logoGraphicsView widget.
    static int oldHeight = ui_->tabWidget->minimumHeight();
    if (collapsed)
    {
        // Collapse the tab widget containing the logo
        oldHeight = qMax(ui_->tabWidget->minimumHeight(), ui_->tabWidget->height());
        ui_->verticalSplitter->setSizes(QList<int>() << ui_->verticalSplitter->height() << 0);
    }
    else
    {
        ui_->verticalSplitter->setSizes(QList<int>() << ui_->verticalSplitter->height() - oldHeight << oldHeight);
    }
}
/**
  * @param collapsed [bool]
  */
void MsaWindow::setSequenceLabelsCollapsed(bool collapsed)
{
    // Note: Currently only the logo is visible in the tabbed widget; so we are simply collapsing and uncollapsing it
    // as a whole here rather than the actual logoGraphicsView widget.
    static int oldWidth = ui_->labelView->minimumWidth();
    if (collapsed)
    {
        // Collapse the tab widget containing the logo
        oldWidth = qMax(ui_->labelView->minimumWidth(), ui_->labelView->width());
        ui_->horizontalSplitter->setSizes(QList<int>() << 0 << ui_->horizontalSplitter->width());
    }
    else
    {
        ui_->horizontalSplitter->setSizes(QList<int>() << oldWidth << ui_->horizontalSplitter->width());
    }
}

/**
  * Because the start and stop values are kept separately from the subseq start and stop, the entities start and stop
  * values must be updated or else they will not be persisted to the storage layer.
  *
  * This method walks through the non-null entities and updates them.
  */
void MsaWindow::updateEntitiesStartStop()
{
    // Synchronize the start and stop fields of Subseq with its corresponding AbstractSeq pointer.
    for (int i=0, z=abstractMsa_->msa()->rowCount(); i<z; ++i)
    {
        const  Subseq *subseq = abstractMsa_->msa()->at(i+1);
        subseq->seqEntity_->setStart(subseq->start());
        subseq->seqEntity_->setStop(subseq->stop());
    }
}

QString MsaWindow::filenameSuffix(const QString &fileName) const
{
    QRegExp regexp("(\\.\\w+)$");
    if (regexp.indexIn(fileName) != -1)
        return regexp.cap(1);

    return QString();
}

QStringList MsaWindow::imageNameFilters() const
{
    return QStringList()
            << "SVG Vector Image (*.svg)"
            << "PNG Image (*.png)"
            << "Windows BMP Image (*.bmp)"
            << "JPEG image (*.jpeg, *.jpg, *.jpe)"
            << "TIFF image (*.tif, *.tiff)";

}

QString MsaWindow::defaultSuffixForNameFilter(const QString &nameFilter) const
{
    if (nameFilter.startsWith("SVG"))
        return "svg";
    else if (nameFilter.startsWith("PNG"))
        return "png";
    else if (nameFilter.startsWith("Windows BMP"))
        return "bmp";
    else if (nameFilter.startsWith("JPEG"))
        return "jpg";
    else if (nameFilter.startsWith("TIFF"))
        return "tif";

    return QString();
}

void MsaWindow::saveMsaRegionAsSvg(const QString &fileName, const PosiRect &msaRegion)
{
    // Everything is ready! Export the alignment
    ExactTextRenderer renderer(ui_->msaView->font());
    SvgGeneratorEngine engine(&renderer);
    engine.setTitle(abstractMsa_->name());
    engine.setDescription(abstractMsa_->description());
    engine.setResolution(logicalDpiX());

    // In order to determine the actual size, need to calculate the max label width
    // Note: because of potential DPI differences, it is necessary to wonkle the dpi when doing the calculations.
    QFont labelFont = ui_->labelView->font();
    int newLabelFontSize = ::fitPointSizeFromPixels(labelFont.family(),
                                                    engine.abstractTextRenderer()->height(),
                                                    72);
    labelFont.setPointSize(newLabelFontSize);
    QStringList labels = msaLabels(msaRegion.verticalRange());
    qreal maxLabelWidth = maxStringWidth(labelFont, labels);
    newLabelFontSize = engine.mapPointSize(newLabelFontSize);
    labelFont.setPointSize(newLabelFontSize);

    qreal svgWidth = maxLabelWidth + kLabelRightMargin + msaRegion.width() * renderer.width();
    qreal svgHeight = msaRegion.height() * renderer.height();
    engine.setSize(QSizeF(svgWidth, svgHeight));
    if (!engine.open(fileName))
    {
        QMessageBox::warning(this,
                             "File error",
                             QString("Unable to open file, %1. Please try again.").arg(fileName),
                             QMessageBox::Ok);
        return;
    }

    // Render the labels
    QPointF textOrigin(0., engine.abstractTextRenderer()->baseline());
    foreach (const QString &label, labels)
    {
        engine.drawText(textOrigin, label, labelFont, Qt::black, nullptr);
        textOrigin.ry() += engine.abstractTextRenderer()->height();
    }

    ui_->msaView->renderMsaRegion(QPointF(maxLabelWidth + kLabelRightMargin, 0.), msaRegion, &engine, nullptr);
    engine.close();
}

void MsaWindow::saveMsaRegionAsImage(const QString &fileName, const PosiRect &msaRegion)
{
    QStringList labels = msaLabels(msaRegion.verticalRange());
    qreal maxLabelWidth = maxStringWidth(ui_->labelView->font(), labels);

    TextPixmapRenderer textPixmapRenderer(ui_->msaView->font(), 1.);
    // Compute the image width
    int width = maxLabelWidth + kLabelRightMargin + msaRegion.width() * textPixmapRenderer.width();
    int height = msaRegion.height() * textPixmapRenderer.height();
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    image.fill(qRgba(255, 255, 255, 255));

    // Draw the labels
    qreal y = textPixmapRenderer.baseline();
    foreach (const QString &label, labels)
    {
        painter.drawText(0., y, label);
        y += textPixmapRenderer.height();
    }

    // Draw the region
    NativeRenderEngine nativeRenderEngine(&textPixmapRenderer);
    ui_->msaView->renderMsaRegion(QPointF(maxLabelWidth + kLabelRightMargin, 0.), msaRegion, &nativeRenderEngine, &painter);
    image.save(fileName);
}

QStringList MsaWindow::msaLabels(const ClosedIntRange &sequenceRange) const
{
    QStringList labels;
    for (int i=sequenceRange.begin_; i<= sequenceRange.end_; ++i)
    {
        const QModelIndex &index = msaSubseqTableModel_->index(i-1, ui_->labelView->column());
        labels << index.data().toString();
    }

    return labels;
}

qreal MsaWindow::maxStringWidth(const QFont &font, const QStringList &strings) const
{
    qreal maxWidth = 0.;
    QFontMetricsF fontMetrics(font);
    foreach (const QString &string, strings)
    {
        int labelWidth = fontMetrics.width(string);
        if (labelWidth > maxWidth)
            maxWidth = labelWidth;
    }

    return maxWidth;
}
