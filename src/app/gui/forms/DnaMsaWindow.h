/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAMSAWINDOW_H
#define DNAMSAWINDOW_H

#include "MsaWindow.h"
#include "../../core/global.h"

class QActionGroup;
class QAction;

class DnaSeqColumnAdapter;
class FilterColumnAdapter;
class LiveInfoContentDistribution;

class DnaMsaWindow : public MsaWindow
{
    Q_OBJECT

public:
    DnaMsaWindow(Adoc *adoc, TaskManager *taskManager, QWidget *parent = nullptr);

    virtual IColumnAdapter *subseqEntityColumnAdapter() const;


protected:
    LiveInfoContentDistribution *liveInfoContentDistribution() const;
    void setMsaEntityImpl(const AbstractMsaSPtr &abstractMsa);


private Q_SLOTS:
    void onBasicColorActionTriggered();
    void onClustalColorActionToggled();

private:
    int mapFromColumnAdapter(int adapterColumn) const;
    void setupColorProviders();

    DnaSeqColumnAdapter *dnaSeqColumnAdapter_;
    FilterColumnAdapter *filterColumnAdapter_;

    QActionGroup *colorProvidersGroup_;
    QAction *defaultColorProviderAction_;

    // Logo variables
    LiveInfoContentDistribution *liveInfoContentDistribution_;
};

#endif // DNAMSAWINDOW_H
