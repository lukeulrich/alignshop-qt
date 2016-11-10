/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractSeq.h"
#include "AbstractAnonSeq.h"
#include "EntityFlags.h"
#include "../UngappedSubseq.h"
#include "../global.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractAnonSeqSPtr AbstractSeq::abstractAnonSeq() const
{
    return abstractAnonSeq_;
}

BioString AbstractSeq::parentBioString() const
{
    return abstractAnonSeq_->seq_.toBioString();
}

/**
  * @returns BioString
  */
BioString AbstractSeq::bioString() const
{
    return abstractAnonSeq_->seq_.mid(ClosedIntRange(start_, stop_));
}

/**
  * @returns int
  */
int AbstractSeq::length() const
{
    return stop_ - start_ + 1;
}

/**
  * @returns ClosedIntRange
  */
ClosedIntRange AbstractSeq::range() const
{
    return ClosedIntRange(start_, stop_);
}

/**
  * @returns int
  */
int AbstractSeq::seqLength() const
{
    return abstractAnonSeq_->seq_.length();
}

/**
  * @returns int
  */
int AbstractSeq::start() const
{
    return start_;
}

/**
  * @returns int
  */
int AbstractSeq::stop() const
{
    return stop_;
}

/**
  * @returns QString
  */
QString AbstractSeq::source() const
{
    return source_;
}

/**
  * @param start [int]
  * @returns bool
  */
bool AbstractSeq::setStart(int start)
{
    if (!abstractAnonSeq_->seq_.isValidPosition(start))
        return false;

    if (start_ != start)
    {
        setDirty(Ag::eCoreDataFlag, true);
        start_ = start;
    }

    return true;
}

/**
  * @param stop [int]
  * @returns bool
  */
bool AbstractSeq::setStop(int stop)
{
    if (!abstractAnonSeq_->seq_.isValidPosition(stop))
        return false;

    if (stop_ != stop)
    {
        setDirty(Ag::eCoreDataFlag, true);
        stop_ = stop;
    }

    return true;
}

void AbstractSeq::setSource(const QString &source)
{
    if (source == source_)
        return;

    source_ = source;
    setDirty(Ag::eCoreDataFlag, true);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected constructor
/**
  * @param id [int]
  * @param start [int]
  * @param stop [int]
  * @param name [const QString &]
  * @param source [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
AbstractSeq::AbstractSeq(int id,
                         int start,
                         int stop,
                         const QString &name,
                         const QString &source,
                         const QString &description,
                         const QString &notes,
                         const AbstractAnonSeqSPtr &abstractAnonSeq)
        : AbstractBasicEntity(id, name, description, notes),
          start_(start),
          stop_(stop),
          source_(source),
          abstractAnonSeq_(abstractAnonSeq)
{
    ASSERT(abstractAnonSeq_);
    ASSERT(abstractAnonSeq_->seq_.isValidPosition(start_));
    ASSERT(abstractAnonSeq_->seq_.isValidPosition(stop_));
}

