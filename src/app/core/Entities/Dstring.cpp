/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Dstring.h"

#include "../macros.h"

const int Dstring::kType = eDstringEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param seq [const Seq &]
  */
Dstring::Dstring(int id, const Seq &seq) : AbstractAnonSeq(id, seq)
{
    ASSERT(seq.grammar() == eDnaGrammar);
}

/**
  * @param dstringPod [const DstringPod &]
  */
Dstring::Dstring(const DstringPod &dstringPod) : AbstractAnonSeq(dstringPod.id_, dstringPod.seq_)
{
    ASSERT(dstringPod.seq_.grammar() == eDnaGrammar);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param seq [const Seq &]
  * @returns Dstring *
  */
Dstring *Dstring::createEntity(const Seq &seq)
{
    ASSERT(seq.grammar() == eDnaGrammar);
    return new Dstring(::newEntityId<Dstring>(), seq);
}
