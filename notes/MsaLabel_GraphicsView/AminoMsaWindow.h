/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOMSAWINDOW_H
#define AMINOMSAWINDOW_H

#include "MsaWindow.h"
#include "../../core/global.h"

class QActionGroup;
class QGraphicsScene;
class QGraphicsItemGroup;

class AminoSeqColumnAdapter;
class FilterColumnAdapter;
class LiveInfoContentDistribution;
class LiveSymbolString;
class LogoBarsItem;
class MsaColumnItemGroup;
class TaskManager;

class AminoMsaWindow : public MsaWindow
{
    Q_OBJECT

public:
    explicit AminoMsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent = nullptr);
    virtual IColumnAdapter *subseqEntityColumnAdapter() const;
    virtual void setMsaEntity(AbstractMsa *abstractMsa);

private Q_SLOTS:
    void onBasicColorActionTriggered();
    void onClustalColorActionToggled(bool checked);
    void onZappaColorActionTriggered();
    void onTaylorColorActionTriggered();
    void onHydroColorActionTriggered();
    void onHelixColorActionTriggered();
    void onStrandColorActionTriggered();
    void onTurnColorActionTriggered();
    void onSecondaryColorActionTriggered();
    void updateSceneRect();

    void onPredictSecondaryStructureActionTriggered();

    // Reactive slots currently targeting the synchronization of the labels with the alignment widget
    void onLabelViewportResized(const QSize &size);
    void onMsaViewVerticalScroll();
    void onZoomChanged();

    void onRangeChanged(int min, int max);

private:
    int mapFromColumnAdapter(int adapterColumn) const;
    void setupColorProviders();

    AminoSeqColumnAdapter *aminoSeqColumnAdapter_;
    FilterColumnAdapter *filterColumnAdapter_;
    LiveSymbolString *liveSymbolString_;
    LiveInfoContentDistribution *liveInfoContentDistribution_;

    QActionGroup *colorProvidersGroup_;
    QAction *defaultColorProviderAction_;
    QAction *predictSecondaryStructureAction_;

    // Logo variables
    QGraphicsScene *logoScene_;
    LogoBarsItem *logoBarsItem_;

    // Label variables
    QGraphicsScene *labelScene_;
    MsaColumnItemGroup *labelItemGroup_;
};

#endif // AMINOMSAWINDOW_H
