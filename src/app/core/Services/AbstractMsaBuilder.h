/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTMSABUILDER_H
#define ABSTRACTMSABUILDER_H

#include <QtCore/QVector>

#include "AbstractProcessWrapper.h"
#include "../PODs/IdBioString.h"
#include "../global.h"

class AbstractMsaBuilder : public AbstractProcessWrapper
{
    Q_OBJECT

public:
    explicit AbstractMsaBuilder(QObject *parent = nullptr)
        : AbstractProcessWrapper(parent)
    {
    }


public Q_SLOTS:
    virtual void align(const QVector<IdBioString> &sequences) = 0;


Q_SIGNALS:
    void alignFinished(const QVector<IdBioString> &sequences);
    void progressChanged(qint64 currentStep, qint64 totalSteps);
    void totalSteps(qint64 totalSteps);
};

#endif // ABSTRACTMSABUILDER_H
