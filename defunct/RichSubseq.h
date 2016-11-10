/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RICHSUBSEQ_H
#define RICHSUBSEQ_H

#include "DataRow.h"
#include "Subseq.h"

/**
  * RichSubseq simply extends Subseq with annotation data in the form of a DataRow.
  *
  * For performance reasons, this is stored as a publicly accessible member
  */
class RichSubseq : public Subseq
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct a RichSubseq derived from anonSeq with seqId and annotation
    RichSubseq(const AnonSeq &anonSeq, const QVariant &seqId = QVariant(), const DataRow &annotation = DataRow());

    // ------------------------------------------------------------------------------------------------
    // Public members
    DataRow annotation_;        //!< Publicly accessible annotation
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param anonSeq [const AnonSeq &]
  * @param seqId [const QVariant &]
  * @param annotation [const DataRow &]
  */
inline
RichSubseq::RichSubseq(const AnonSeq &anonSeq, const QVariant &seqId, const DataRow &annotation)
    : Subseq(anonSeq, seqId), annotation_(annotation)
{
}


#endif // RICHSUBSEQ_H
