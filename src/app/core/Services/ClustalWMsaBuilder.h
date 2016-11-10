/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALWMSABUILDER_H
#define CLUSTALWMSABUILDER_H

#include <QtCore/QString>

#include "AbstractMsaBuilder.h"
#include "../global.h"

class ClustalWMsaBuilder : public AbstractMsaBuilder
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit ClustalWMsaBuilder(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString friendlyProgramName() const;                //!< Returns "ClustalW"


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void align(const QVector<IdBioString> &sequences);


protected Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Protected slots
    void onReadyReadStandardOutput();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void handleFinished();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int currentStep(int first, int second) const;
    QVector<IdBioString> parseAlignment(const QString &file);
    int halfSquareTotal(int rows) const;

    QVector<IdBioString> sequences_;
    int totalPairwise_;
    QString inFile_;
    QString outFile_;
};


#endif // CLUSTALWMSABUILDER_H
