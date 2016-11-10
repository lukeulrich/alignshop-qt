/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PREDICTSECONDARYTASK_H
#define PREDICTSECONDARYTASK_H

#include "Task.h"

#include "../../../core/BioString.h"
#include "../../../core/PODs/Q3Prediction.h"
#include "../../../core/Services/PsiBlastStructureTool.h"
#include "../../../core/util/OptionSet.h"

class Adoc;

class PredictSecondaryTask : public Task
{
    Q_OBJECT

public:
    PredictSecondaryTask(Adoc *adoc, int id, const BioString &sequence, const OptionSet &psiBlastOptions, const QString &name = "Predicting secondary structure");


public Q_SLOTS:
    void start();


private Q_SLOTS:
    void onProgressChanged(int currentStep, int totalSteps);
    void onError(int id, const QString &errorMessage);
    void onFinished(int id, Q3Prediction q3);


private:
    Adoc *adoc_;
    int id_;
    BioString sequence_;
    OptionSet psiBlastOptions_;

    PsiBlastStructureTool *psiBlastStructureTool_;
};

#endif // PREDICTSECONDARYTASK_H
