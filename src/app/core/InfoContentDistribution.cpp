/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGlobal> // For Q_OS_WIN

// Swine windows does not include the log2 function
#ifndef Q_OS_WIN
#include <cmath>            // For log2
#else
#include <math.h>
double log2(double n)
{
    return log(n) / log(2.);
}
#endif

#include "InfoContentDistribution.h"
#include "macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private helper methods
/**
  * @param a [const InfoUnit &]
  * @param b [const InfoUnit &]
  * @returns bool
  */
bool infoUnitLessThan(const InfoUnit &a, const InfoUnit &b)
{
    return a.info_ < b.info_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param charCountDistribution [const CharCountDistribution &]
  * @param possibleLetters [int]
  * @param smallSampleErrorCorrection [bool]
  */
InfoContentDistribution::InfoContentDistribution(const CharCountDistribution &charCountDistribution,
                                                 int possibleLetters,
                                                 bool smallSampleErrorCorrection)
    : smallSampleErrorCorrection_(smallSampleErrorCorrection),
      possibleLetters_(possibleLetters)
{
    ASSERT_X(possibleLetters > 0, "possibleLetters must be positive");

    maxInfo_ = log2(possibleLetters_);
    smallSampleErrorFactor_ = static_cast<double>(possibleLetters_ - 1) / (2. * log(2.));
    infoContent_ = computeInfoContent(charCountDistribution);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns VectorVectorInfoUnit
  */
VectorVectorInfoUnit InfoContentDistribution::infoContent() const
{
    return infoContent_;
}

double InfoContentDistribution::columnInfo(const int column) const
{
    ASSERT(column > 0 && column <= length());

    const VectorInfoUnit &vectorInfoUnit = infoContent_.at(column - 1);
    double sumInfo = 0.;
    VectorInfoUnit::ConstIterator it = vectorInfoUnit.constBegin();
    for (; it != vectorInfoUnit.constEnd(); ++it)
        sumInfo += (*it).info_;
    return sumInfo;
}

/**
  * @returns int
  */
int InfoContentDistribution::length() const
{
    return infoContent_.size();
}

/**
  * @returns double
  */
double InfoContentDistribution::maxInfo() const
{
    return maxInfo_;
}

/**
  * @returns int
  */
int InfoContentDistribution::possibleLetters() const
{
    return possibleLetters_;
}

/**
  * @returns bool
  */
bool InfoContentDistribution::smallSampleErrorCorrection() const
{
    return smallSampleErrorCorrection_;
}

/**
  * @returns double
  */
double InfoContentDistribution::totalInfo() const
{
    double sumInfo = 0.;
    for (int i=1, z=length(); i<=z; ++i)
        sumInfo += columnInfo(i);
    return sumInfo;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param charCountDistribution [const CharCountDistribution &]
  * @param range [const ClosedIntRange &]
  * @returns VectorVectorInfoUnit
  */
VectorVectorInfoUnit InfoContentDistribution::computeInfoContent(const CharCountDistribution &charCountDistribution,
                                                                 const ClosedIntRange &range) const
{
    if (charCountDistribution.length() == 0)
        return VectorVectorInfoUnit();

    ClosedIntRange actualRange = range;
    if (range.isEmpty())
    {
        actualRange.begin_ = 1;
        actualRange.end_ = charCountDistribution.length();
    }

    ASSERT_X(actualRange.begin_ > 0 && actualRange.begin_ <= actualRange.end_, "invalid range.begin");
    ASSERT_X(actualRange.end_ <= charCountDistribution.length(), "invalid range.end");

    VectorVectorInfoUnit infoContent;
    infoContent.reserve(actualRange.length());

    double divisor = charCountDistribution.divisor();

    // Iterate through each column of the charCountDistribution
    const VectorHashCharInt &charCounts = charCountDistribution.charCounts();
    for (int i=actualRange.begin_; i<= actualRange.end_; ++i)
    {
        // -1 to account for zero-based vector layout
        const HashCharInt &hashCharInt = charCounts.at(i-1);

        infoContent << VectorInfoUnit();
        VectorInfoUnit &vectorInfoUnit = infoContent.last();
        vectorInfoUnit.reserve(hashCharInt.size());

        int nLettersInColumn = 0;
        double entropy = 0.;
        HashCharInt::ConstIterator it = hashCharInt.constBegin();
        for (; it != hashCharInt.constEnd(); ++it)
        {
            // it.key() = char
            // it.value() = count
            nLettersInColumn += it.value();
            double percent = static_cast<double>(it.value()) / divisor;
            vectorInfoUnit << InfoUnit(it.key(), percent, 0.);
            entropy += percent * log2(percent);
        }

        double error = (smallSampleErrorCorrection_) ? smallSampleErrorFactor_ / static_cast<double>(nLettersInColumn)
                                                     : 0;
        double totalColumnInfo = maxInfo_ + entropy - error;

        for (int j=0, y=vectorInfoUnit.size(); j<y; ++j)
        {
            InfoUnit &infoUnit = vectorInfoUnit[j];
            // The error may drop below zero when applying the small sample correction
            // TODO: Build out a test that ensure we avoid allowing the IC to drop below zero
            infoUnit.info_ = qMax(0., infoUnit.percent_ * totalColumnInfo);
        }

        qStableSort(vectorInfoUnit.begin(), vectorInfoUnit.end(), infoUnitLessThan);
    }

    return infoContent;
}
