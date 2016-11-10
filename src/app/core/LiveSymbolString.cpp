/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AbstractLiveCharCountDistribution.h"
#include "LiveSymbolString.h"
#include "macros.h"
#include "misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param liveCharCountDistribution [const AbstractLiveCharCountDistribution *]
  * @param bioSymbolGroup [const BioSymbolGroup &]
  */
LiveSymbolString::LiveSymbolString(const AbstractLiveCharCountDistribution *liveCharCountDistribution,
                                   const SymbolStringCalculator &symbolStringCalculator,
                                   QObject *parent) :
    QObject(parent),
    liveCharCountDistribution_(liveCharCountDistribution),
    symbolStringCalculator_(symbolStringCalculator)
{
    if (liveCharCountDistribution_ != nullptr)
    {
        connect(liveCharCountDistribution_,
                SIGNAL(columnsInserted(ClosedIntRange)), SLOT(sourceDistributionColumnsInserted(ClosedIntRange)));
        connect(liveCharCountDistribution_,
                SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(sourceDistributionColumnsRemoved(ClosedIntRange)));
        connect(liveCharCountDistribution_,
                SIGNAL(dataChanged(ClosedIntRange)), SLOT(sourceDataChanged(ClosedIntRange)));

        symbolString_ = calculateSymbolString();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns const AbstractLiveCharCountDistribution *
  */
const AbstractLiveCharCountDistribution *LiveSymbolString::liveCharCountDistribution() const
{
    return liveCharCountDistribution_;
}

/**
  * Returns an empty QByteArray if the liveCharCountDistribution_ is not defined or it is of length zero. Otherwise,
  * the symbol string is computed using the source live char count distribution and bioSymbolGroup.
  *
  * @returns QByteArray
  */
QByteArray LiveSymbolString::symbolString() const
{
    return symbolString_;
}

/**
  * @returns SymbolStringCalculator
  */
SymbolStringCalculator LiveSymbolString::symbolStringCalculator() const
{
    return symbolStringCalculator_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param range [const ClosedIntRange &]
  */
void LiveSymbolString::sourceDistributionColumnsInserted(const ClosedIntRange &range)
{
    symbolString_.insert(range.begin_-1, calculateSymbolString(range));
    emit symbolsInserted(range);
}

/**
  * @param range [const ClosedIntRange &]
  */
void LiveSymbolString::sourceDistributionColumnsRemoved(const ClosedIntRange &range)
{
    symbolString_.remove(range.begin_ - 1, range.length());
    emit symbolsRemoved(range);
}

/**
  * @param range [const ClosedIntRange &]
  */
void LiveSymbolString::sourceDataChanged(const ClosedIntRange &range)
{
    QByteArray oldSymbols = symbolString_.mid(range.begin_ - 1, range.length());
    QByteArray newSymbols = calculateSymbolString(range);
    if (newSymbols == oldSymbols)
        return;

    symbolString_.replace(range.begin_ - 1, range.length(), newSymbols);
    emit dataChanged(range);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * If no active distribution has been defined, then return an empty QByteArray.
  *
  * @param range [const ClosedIntRange &]
  */
QByteArray LiveSymbolString::calculateSymbolString(const ClosedIntRange &range) const
{
    if (!liveCharCountDistribution_
        || liveCharCountDistribution_->charCountDistribution().length() == 0)
    {
        ASSERT(range.isEmpty());
        return QByteArray();
    }

    // Determine the translated coordinates
    ClosedIntRange actualRange = range;
    if (actualRange.isEmpty())
    {
        actualRange.begin_ = 1;
        actualRange.end_ = liveCharCountDistribution_->charCountDistribution().length();
    }

    ASSERT(actualRange.begin_ > 0 && actualRange.begin_ <= actualRange.end_);
    ASSERT(actualRange.end_ <= liveCharCountDistribution_->charCountDistribution().length());

    // Extract the region for which to compute the symbol string
    CharCountDistribution subCharDist = liveCharCountDistribution_->charCountDistribution().mid(actualRange);

    // Use the BioSymbolGroup to determine the symbol string for this region
    return symbolStringCalculator_.computeSymbolString(subCharDist.charPercents());
}
