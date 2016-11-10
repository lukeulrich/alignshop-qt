/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOMSAWINDOW_H
#define AMINOMSAWINDOW_H

#include <QtCore/QHash>
#include <QtCore/QSet>

#include "MsaWindow.h"
#include "../../core/BioSymbolGroup.h"
#include "../../core/global.h"

class QActionGroup;

class AminoSeqColumnAdapter;
class BlastDatabaseModel;
class BlastDatabaseWindow;
class ConsensusLabelTableView;
class ConsensusView;
class FilterColumnAdapter;
class ITask;
class LiveInfoContentDistribution;
class LiveSymbolString;
class MsaConsensusModel;
class PredictSecondaryDialog;
class TaskManager;

class AminoMsaWindow : public MsaWindow
{
    Q_OBJECT

public:
    AminoMsaWindow(Adoc *adoc,
                   TaskManager *taskManager,
                   BlastDatabaseModel *blastDatabaseModel,
                   BlastDatabaseWindow *blastDatabaseWindow,        // Pointer to blast database manager window
                   QWidget *parent = nullptr);
    ~AminoMsaWindow();
    virtual IColumnAdapter *subseqEntityColumnAdapter() const;


protected:
    LiveInfoContentDistribution *liveInfoContentDistribution() const;
    void setMsaEntityImpl(const AbstractMsaSPtr &abstractMsa);


private Q_SLOTS:
    void onBasicColorActionTriggered();
    void onClustalColorActionToggled(bool checked);
    void onZappaColorActionTriggered();
    void onTaylorColorActionTriggered();
    void onHydroColorActionTriggered();
    void onHelixColorActionTriggered();
    void onStrandColorActionTriggered();
    void onTurnColorActionTriggered();
    void onBuriedColorActionTriggered();
    void onSecondaryColorActionTriggered();

    void onPredictSecondaryStructureActionTriggered();

    void onConsensusActionToggled(bool checked);
    void onConsensusThresholdsActionTriggered();
    void onEditConsensusGroupsActionTriggered();

    void onTaskAboutToStart(ITask *task);
    void onTaskDone(ITask *task);
    void onTaskError(ITask *task);

    void updateConsensusLabelFont();

private:
    int mapFromColumnAdapter(int adapterColumn) const;
    void setupColorProviders();
    void updateConsensusModel();


    BlastDatabaseModel *blastDatabaseModel_;
    BlastDatabaseWindow *blastDatabaseWindow_;
    PredictSecondaryDialog *predictSecondaryDialog_;

    AminoSeqColumnAdapter *aminoSeqColumnAdapter_;
    FilterColumnAdapter *filterColumnAdapter_;
    LiveSymbolString *liveClustalSymbolString_;
    LiveInfoContentDistribution *liveInfoContentDistribution_;

    MsaConsensusModel *msaConsensusModel_;
    ConsensusView *consensusView_;
    ConsensusLabelTableView *consensusLabelTableView_;

    QActionGroup *colorProvidersGroup_;
    QAction *defaultColorProviderAction_;
    QAction *predictSecondaryStructureAction_;

    QHash<int, int> taskIdAstringIdHash_;   // Maps a predict secondary task id to its corresponding astring id
    QSet<int> queuedAstringIds_;            // List of astring ids that are already queued for secondary structure prediction;

    bool useCustomConsensusGroups_;
    BioSymbolGroup customConsensusGroup_;
};

#endif // AMINOMSAWINDOW_H
