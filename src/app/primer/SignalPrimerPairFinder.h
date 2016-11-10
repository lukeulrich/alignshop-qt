/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SIGNALPRIMERPAIRFINDER_H
#define SIGNALPRIMERPAIRFINDER_H

#include <QtCore/QObject>
#include "PrimerPair.h"         // For PrimerPairVector
#include "../core/global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class PrimerPairFinder;
class PrimerSearchParameters;

/**
  * Wraps PrimerPairFinder with a signal/slot interface.
  */
class SignalPrimerPairFinder : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SignalPrimerPairFinder(QObject *parent = nullptr);
    ~SignalPrimerPairFinder();


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void cancel();
    void findPrimerPairs(const BioString &dnaString, const ClosedIntRange &range, const PrimerSearchParameters &input);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void finished(const PrimerPairVector &primerPairs);


private:
    PrimerPairFinder *primerPairFinder_;
};

#endif // SIGNALPRIMERPAIRFINDER_H
