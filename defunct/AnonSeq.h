/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ANONSEQ_H
#define ANONSEQ_H

#include <QtCore/QSharedDataPointer>

#include "BioString.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AnonSeqPrivate;       // Defined in the cpp file

/**
  * Full length, source-agnostic, ungapped, masked representation of a biological sequence.
  *
  * Models all biological sequences regardless of their source and type, and provides a unified
  * interface for accessing the raw sequence data. It is not possible to change the sequence after
  * instantiation. Thus, it represents a static representation of the exact sequence.
  *
  * The sequence data may be accessed via a copy of the underlying BioString object, which is allocated on the
  * heap and cloned in order to properly maintain the source BioString class type.
  *
  * This class is implicitly shared for both convenience and performance reasons.
  */
class AnonSeq
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    AnonSeq();                                                      //!< Construct an empty, invalid AnonSeq
    AnonSeq(int id, const BioString &bioString);                    //!< Construct an AnonSeq initialized with id and bioString
    AnonSeq(int id, BioString *bioString);                          //!< Construct an AnonSeq initialized with id and bioString; takes ownership of bioString
    AnonSeq(const AnonSeq &other);                                  //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Destructor
    ~AnonSeq();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    AnonSeq &operator=(const AnonSeq &other);               //!< Assigns other to this AnonSeq

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const BioString &bioString() const;                     //!< Return a const weak pointer to the internal BioString
    int id() const;                                         //!< Get the internal sequence identifier
    bool isValid() const;                                   //!< Returns whether this sequence is a valid AnonSeq (has a non-zero length)

private:
    QSharedDataPointer<AnonSeqPrivate> d_;
};




#endif // ANONSEQ_H
