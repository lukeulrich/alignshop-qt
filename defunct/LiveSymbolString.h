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

#include "BioSymbolGroup.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class LiveCharCountDistribution;

/**
  * LiveSymbolString provides an observable symbol string computed from a LiveCharCountDistribution and BioSymbolGroup.
  *
  * It is not possible to change the source LiveCharCountDistribution or corresponding BioSymbolGroup after
  * construction; however, "getter" functions are provided to retrieve the current values.
  *
  * LiveSymbolString optimally updates the symbol string in response to changes in the LiveCharCountDistribution. If
  * only a fraction of the CharCountDistribution columns have been updated, then only the symbols for that region will
  * be recomputed.
  */
class LiveSymbolString : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct a LiveSymbolString using liveCharCountDistribution and bioSymbolGroup with parent
    LiveSymbolString(const LiveCharCountDistribution *liveCharCountDistribution, const BioSymbolGroup &bioSymbolGroup, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioSymbolGroup bioSymbolGroup() const;                              //!< Returns the BioSymbolGroup
    const LiveCharCountDistribution *liveCharCountDistribution() const; //!< Returns the LiveCharCountDistribution
    QString symbolString() const;                                       //!< Returns the current symbol string determination

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void symbolsInserted(int from, int to);                             //!< Emitted after the symbols between from and to (inclusive, 1-based) have been inserted
    void symbolsRemoved(int from, int to);                              //!< Emitted after the symbols between from and to (inclusive, 1-based) have been removed)
    void dataChanged(int startColumn, int stopColumn);                  //!< Emitted when any of the symbols between startColumn and stopColumn (inclusive, 1-baed) have changed

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void sourceDistributionColumnsInserted(int from, int to);           //!< Called when the source liveCharCountDistribution has columns inserted
    void sourceDistributionColumnsRemoved(int from, int to);            //!< Called when the source liveCharCountDistribution has columns removed
    void sourceDataChanged(int startColumn, int stopColumn);            //!< Called when the source liveCharCountDistribution data has changed

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString calculateSubSymbolString(int from = 0, int to = 0) const;   //!< Computes and returns the symbol string for the columns between from and to of liveCharCountDistribution_

    const LiveCharCountDistribution *liveCharCountDistribution_;
    BioSymbolGroup bioSymbolGroup_;
    QString symbolString_;
};

#endif // LIVESYMBOLSTRING_H
