/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "LiveSymbolString.h"

#include "LiveCharCountDistribution.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param liveCharCountDistribution [const LiveCharCountDistribution *]
  * @param bioSymbolGroup [const BioSymbolGroup &]
  */
LiveSymbolString::LiveSymbolString(const LiveCharCountDistribution *liveCharCountDistribution, const BioSymbolGroup &bioSymbolGroup, QObject *parent) :
    QObject(parent), liveCharCountDistribution_(liveCharCountDistribution), bioSymbolGroup_(bioSymbolGroup)
{
    if (liveCharCountDistribution_)
    {
        connect(liveCharCountDistribution_, SIGNAL(columnsInserted(int,int)), SLOT(sourceDistributionColumnsInserted(int,int)));
        connect(liveCharCountDistribution_, SIGNAL(columnsRemoved(int,int)), SLOT(sourceDistributionColumnsRemoved(int,int)));
        connect(liveCharCountDistribution_, SIGNAL(dataChanged(int,int)), SLOT(sourceDataChanged(int,int)));

        symbolString_ = calculateSubSymbolString();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioSymbolGroup
  */
BioSymbolGroup LiveSymbolString::bioSymbolGroup() const
{
    return bioSymbolGroup_;
}

/**
  * @returns const LiveCharCountDistribution *
  */
const LiveCharCountDistribution *LiveSymbolString::liveCharCountDistribution() const
{
    return liveCharCountDistribution_;
}

/**
  * Returns an empty QString if the liveCharCountDistribution_ is not defined or it is of length zero. Otherwise,
  * the symbol string is computed using the source live char count distribution and bioSymbolGroup.
  *
  * @returns QString
  */
QString LiveSymbolString::symbolString() const
{
    return symbolString_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  *
  *
  * @param from [int]
  * @param to [int]
  */
void LiveSymbolString::sourceDistributionColumnsInserted(int from, int to)
{
    symbolString_.insert(from-1, calculateSubSymbolString(from, to));
    emit symbolsInserted(from, to);
}

/**
  *
  *
  * @param from [int]
  * @param to [int]
  */
void LiveSymbolString::sourceDistributionColumnsRemoved(int from, int to)
{
    symbolString_.remove(from - 1, to - from + 1);
    emit symbolsRemoved(from, to);
}

/**
  *
  *
  * @param startColumn [int]
  * @param stopColumn [int]
  */
void LiveSymbolString::sourceDataChanged(int startColumn, int stopColumn)
{
    symbolString_.replace(startColumn - 1, stopColumn - startColumn + 1, calculateSubSymbolString(startColumn, stopColumn));
    emit dataChanged(startColumn, stopColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * If no active distribution has been defined, then return an empty QString.
  *
  * @param from [int]
  * @param to [int]
  */
QString LiveSymbolString::calculateSubSymbolString(int from, int to) const
{
    if (!liveCharCountDistribution_
        || liveCharCountDistribution_->charCountDistribution().length() == 0)
    {
        ASSERT(from == 0);
        ASSERT(to == 0);
        return QString();
    }

    // Determine the translated coordinates
    int actualFrom = from;
    int actualTo = to;
    if (from == 0 && to == 0)
    {
        actualFrom = 1;
        actualTo = liveCharCountDistribution_->charCountDistribution().length();
    }

    ASSERT(actualFrom > 0 && actualFrom <= actualTo);
    ASSERT(actualTo <= liveCharCountDistribution_->charCountDistribution().length());

    // Extract the region for which to compute the symbol string
    ListHashCharInt subCharCounts = liveCharCountDistribution_->charCountDistribution().charCounts().mid(actualFrom - 1, actualTo - actualFrom + 1);

    // Use the BioSymbolGroup to determine the symbol string for this region
    return bioSymbolGroup_.calculateSymbolString(::divideListHashCharInt(subCharCounts, liveCharCountDistribution_->divisor()));
}
