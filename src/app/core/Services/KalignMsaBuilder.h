/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef KALIGNMSABUILDER_H
#define KALIGNMSABUILDER_H

#include <QtCore/QString>

#include "AbstractMsaBuilder.h"
#include "../global.h"

class KalignMsaBuilder : public AbstractMsaBuilder
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit KalignMsaBuilder(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString friendlyProgramName() const;                //!< Returns "Kalign"


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void align(const QVector<IdBioString> &sequences);


protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Protected slots
    void onProcessStarted();
    void onReadyReadStandardError();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void handleFinished();


private:
    QVector<IdBioString> parseAlignment(const QString &file);

    QVector<IdBioString> sequences_;
    QString outFile_;

    // It is not possible to directly use the output from Kalign as a progress meter because there are two phases
    // where it goes from 0 - 100%. Therefore, it is necessary to tally this ourselves.
    int progress_;
};

#endif // KALIGNMSABUILDER_H
