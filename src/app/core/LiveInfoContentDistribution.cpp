/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LiveInfoContentDistribution.h"
#include "AbstractLiveCharCountDistribution.h"
#include "util/ClosedIntRange.h"
#include "macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param liveCharCountDistribution [AbstractLiveCharCountDistribution *]
  * @param possibleLetters [int]
  * @param smallSampleErrorCorrection [bool]
  * @param parent [QObject *]
  */
LiveInfoContentDistribution::LiveInfoContentDistribution(AbstractLiveCharCountDistribution *liveCharCountDistribution,
                                                         int possibleLetters,
                                                         bool smallSampleErrorCorrection,
                                                         QObject *parent)
    : QObject(parent),
      InfoContentDistribution(liveCharCountDistribution->charCountDistribution(),
                              possibleLetters,
                              smallSampleErrorCorrection),
      liveCharCountDistribution_(liveCharCountDistribution)
{
    ASSERT(liveCharCountDistribution != nullptr);

    connect(liveCharCountDistribution_, SIGNAL(columnsInserted(ClosedIntRange)), SLOT(onSourceColumnsInserted(ClosedIntRange)));
    connect(liveCharCountDistribution_, SIGNAL(columnsRemoved(ClosedIntRange)), SLOT(onSourceColumnsRemoved(ClosedIntRange)));
    connect(liveCharCountDistribution_, SIGNAL(dataChanged(ClosedIntRange)), SLOT(onSourceDataChanged(ClosedIntRange)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractLiveCharCountDistribution *
  */
AbstractLiveCharCountDistribution *LiveInfoContentDistribution::liveCharCountDistribution() const
{
    return liveCharCountDistribution_;
}

/**
  * @param enabled [bool]
  */
void LiveInfoContentDistribution::setSmallSampleErrorCorrection(bool enabled)
{
    if (smallSampleErrorCorrection_ == enabled)
        return;

    smallSampleErrorCorrection_ = enabled;
    if (liveCharCountDistribution_->charCountDistribution().length() == 0)
        return;

    infoContent_ = computeInfoContent(liveCharCountDistribution_->charCountDistribution());
    emit dataChanged(ClosedIntRange(1, infoContent_.size()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param range [const ClosedIntRange &]
  */
void LiveInfoContentDistribution::onSourceColumnsInserted(const ClosedIntRange &range)
{
    VectorVectorInfoUnit addition = computeInfoContent(liveCharCountDistribution_->charCountDistribution(), range);
    ASSERT(addition.size() == range.length());
    infoContent_.insert(range.begin_ - 1, range.length(), VectorInfoUnit());
    for (int i=range.begin_ - 1, j=0, z= range.end_ - 1; i<= z; ++i, ++j)
        infoContent_[i] = addition.at(j);

    emit columnsInserted(range);
}

/**
  * @param range [const ClosedIntRange &]
  */
void LiveInfoContentDistribution::onSourceColumnsRemoved(const ClosedIntRange &range)
{
    infoContent_.remove(range.begin_ - 1, range.length());
    emit columnsRemoved(range);
}

/**
  * @param range [const ClosedIntRange &]
  */
void LiveInfoContentDistribution::onSourceDataChanged(const ClosedIntRange &range)
{
    VectorVectorInfoUnit changed = computeInfoContent(liveCharCountDistribution_->charCountDistribution(), range);
    ASSERT(changed.size() == range.length());
    for (int i=range.begin_ - 1, j=0, z= range.end_ - 1; i<= z; ++i, ++j)
        infoContent_[i] = changed.at(j);

    emit dataChanged(range);
}
