/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AnonSeqFactory.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Convenient, internal function for constructing a new AnonSeq with the prototyped BioString or a generic
  * BioString depending if the prototype has not been set.
  *
  * @param id [int]
  * @param bioString [const BioString &]
  * @returns AnonSeq
  */
AnonSeq AnonSeqFactory::createPrototypedAnonSeq(int id, const BioString &bioString) const
{
    // Empty sequences are not permitted
    Q_ASSERT_X(bioString.reduced().isEmpty() == false, "AnonSeqFactory::create", "reduced form of bioString should not be empty");

    BioString *b;
    if (bioStringPrototype_.isNull() == false)
        b = bioStringPrototype_->create();
    else
        b = new BioString();

    *b = bioString;

    return AnonSeq(id, b);
}
