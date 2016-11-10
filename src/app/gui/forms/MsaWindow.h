/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAWINDOW_H
#define MSAWINDOW_H

#include <QtGui/QMainWindow>
#include "../../core/util/PosiRect.h"
#include "../../core/global.h"
#include "../../core/Entities/AbstractMsa.h"

class QAction;
class QActionGroup;
class QFileDialog;
class QGraphicsScene;
class QLabel;
class QUndoStack;

class Adoc;
class GapMsaTool;
class HandMsaTool;
class IColumnAdapter;
class LiveInfoContentDistribution;
class LiveMsaCharCountDistribution;
class LogoItem;
class MsaSubseqTableModel;
class ObservableMsa;
class PercentSpinBox;
class SelectMsaTool;
class TaskManager;
class ZoomMsaTool;


namespace Ui {
    class MsaWindow;
}

class MsaWindow : public QMainWindow
{
    Q_OBJECT

public:
    // -------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit MsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent = nullptr);
    ~MsaWindow();


    // -------------------------------------------------------------------------------------------------
    // Public methods
    Adoc *adoc() const;
    bool isModified() const;
    LiveMsaCharCountDistribution *liveMsaCharCountDistribution() const;
    LogoItem *logoItem() const;
    ObservableMsa *msa() const;
    virtual AbstractMsaSPtr msaEntity() const;
    void setMsaEntity(const AbstractMsaSPtr &abstractMsa);
    virtual IColumnAdapter *subseqEntityColumnAdapter() const;      // The default implementation returns a nullptr
    TaskManager *taskManager() const;
    QString titleString() const;


public Q_SLOTS:
    // -------------------------------------------------------------------------------------------------
    // Public slots
    bool save();


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void aboutToClose(MsaWindow *self);


protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Protected slots
    void updateLogoSceneRect();


protected:
    // -------------------------------------------------------------------------------------------------
    // Protected methods
    virtual LiveInfoContentDistribution *liveInfoContentDistribution() const = 0;
    virtual void setMsaEntityImpl(const AbstractMsaSPtr &abstractMsa) = 0;
    void closeEvent(QCloseEvent *closeEvent);
    bool eventFilter(QObject *object, QEvent *event);
    QList<QKeySequence> actionShortCuts() const;

    QUndoStack *undoStack_;
    Ui::MsaWindow *ui_;
    MsaSubseqTableModel *msaSubseqTableModel_;
    QAction *logoAction_;
    QAction *sequenceLabelsAction_;


private Q_SLOTS:
    // -------------------------------------------------------------------------------------------------
    // Private slots
    // Alignment menu actions
    void onActionSaveImage();
    void onActionExport();
    void onExportFileDialogFilterSelected(const QString &nameFilter);

    // Edit menu actions
    void onActionTrimRowsLeft();
    void onActionTrimRowsRight();
    void onActionExtendRowsLeft();
    void onActionExtendRowsRight();
    void onActionLevelRowsLeft();
    void onActionLevelRowsRight();
    void onActionRemoveAllGapColumns();
    void onActionCollapseLeft();
    void onActionCollapseRight();
    void onActionSelectAll();
    void onActionDeselectAll();

    // Zoom actions
    void onActionZoomIn();
    void onActionZoomOut();
    void onActionZoom1_1();
    void onActionZoom1_2();
    void onActionZoom2_1();

    // Msa editing tool actions
    void onActionHandTool();
    void onActionSelectTool();
    void onActionZoomTool();
    void onActionGapTool();

    void enableDisableActions();

    void onMsaGapColumnsInsertFinished(const ClosedIntRange &columns, bool normal);

    void onMsaSlideStarted(const PosiRect &msaRect);
    void onMsaSlideFinished(const PosiRect &msaRect);

    void onUndoCleanChanged(bool isClean);

    void onViewportMouseLeft();
    void onMouseCursorPointMoved(const QPoint &msaPoint);
    void onMsaViewContextMenuRequested(const QPoint &msaViewPoint);
    void onMsaViewSelectionChanged(const PosiRect &selection);

    void onLogoActionTriggered();
    void onSequenceLabelsActionTriggered();
    void onViewMenuAboutToShow();

    void onMsaViewMarginsChanged(const QMargins &margins);
    void updateBottomMargin();
    void updateTotalInfoLabel();

    void onLogoContextMenuRequested(const QPoint &point);





private:
    // -------------------------------------------------------------------------------------------------
    // Private methods
    bool labelViewEventFilter(QEvent *event);
    void setLogoCollapsed(bool collapsed);      // Slightly more complicated than calling setVisible because we have to work with a tabbed widget
    void setSequenceLabelsCollapsed(bool collapsed);
    void updateEntitiesStartStop();
    QString filenameSuffix(const QString &fileName) const;
    QStringList imageNameFilters() const;
    QString defaultSuffixForNameFilter(const QString &nameFilter) const;
    void saveMsaRegionAsSvg(const QString &fileName, const PosiRect &msaRegion);
    void saveMsaRegionAsImage(const QString &fileName, const PosiRect &msaRegion);
    QStringList msaLabels(const ClosedIntRange &sequenceRange) const;
    qreal maxStringWidth(const QFont &font, const QStringList &strings) const;

    Adoc *adoc_;
    AbstractMsaSPtr abstractMsa_;
    LiveMsaCharCountDistribution *liveMsaCharCountDistribution_;
    PercentSpinBox *zoomSpinBox_;
    QLabel *locationLabel_;

    // Export file dialog
    QFileDialog *saveImageFileDialog_;
    QFileDialog *exportFileDialog_;

    // Logo variables
    QGraphicsScene *logoScene_;
    LogoItem *logoItem_;

    PosiRect slideStartRect_;

    QActionGroup *msaToolGroup_;
    HandMsaTool *handMsaTool_;
    SelectMsaTool *selectMsaTool_;
    ZoomMsaTool *zoomMsaTool_;
    GapMsaTool *gapMsaTool_;

    TaskManager *taskManager_;

    // Info content label
    QLabel *totalInfoContentLabel_;
};

#endif // MSAWINDOW_H
