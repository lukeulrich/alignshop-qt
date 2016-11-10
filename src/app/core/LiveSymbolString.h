/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef LIVESYMBOLSTRING_H
#define LIVESYMBOLSTRING_H

#include <QtCore/QObject>

#include "global.h"
#include "Services/SymbolStringCalculator.h"
#include "util/ClosedIntRange.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AbstractLiveCharCountDistribution;
class ClosedIntRange;

/**
  * LiveSymbolString provides an observable symbol string computed from a AbstractLiveCharCountDistribution and
  * BioSymbolGroup.
  *
  * It is not possible to change the source AbstractLiveCharCountDistribution or corresponding BioSymbolGroup after
  * construction; however, "getter" functions are provided to retrieve the current values.
  *
  * LiveSymbolString optimally updates the symbol string in response to changes in the
  * AbstractLiveCharCountDistribution. If only a fraction of the CharCountDistribution columns have been updated, then
  * only the symbols for that region will be recomputed.
  */
class LiveSymbolString : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct a LiveSymbolString using liveCharCountDistribution and symbolStringCalculator with parent
    LiveSymbolString(const AbstractLiveCharCountDistribution *liveCharCountDistribution,
                     const SymbolStringCalculator &symbolStringCalculator,
                     QObject *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns the AbstractLiveCharCountDistribution
    const AbstractLiveCharCountDistribution *liveCharCountDistribution() const;
    QByteArray symbolString() const;                                    //!< Returns the current symbol string determination
    SymbolStringCalculator symbolStringCalculator() const;              //!< Returns the SymbolStringCalculator


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void symbolsInserted(const ClosedIntRange &range);                  //!< Emitted after the symbols in range (inclusive, 1-based) have been inserted
    void symbolsRemoved(const ClosedIntRange &range);                   //!< Emitted after the symbols in range (inclusive, 1-based) have been removed)
    void dataChanged(const ClosedIntRange &range);                      //!< Emitted when any of the symbols in range (inclusive, 1-baed) have changed


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void sourceDistributionColumnsInserted(const ClosedIntRange &range);//!< Called when the source liveCharCountDistribution has columns inserted
    void sourceDistributionColumnsRemoved(const ClosedIntRange &range); //!< Called when the source liveCharCountDistribution has columns removed
    void sourceDataChanged(const ClosedIntRange &range);                //!< Called when the source liveCharCountDistribution data has changed


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Computes and returns the symbol string for the columns in range of liveCharCountDistribution_
    QByteArray calculateSymbolString(const ClosedIntRange &range = ClosedIntRange()) const;

    const AbstractLiveCharCountDistribution *liveCharCountDistribution_;
    SymbolStringCalculator symbolStringCalculator_;
    QByteArray symbolString_;
};

#endif // LIVESYMBOLSTRING_H
