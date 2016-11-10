/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AnonSeq.h"

#include <QtCore/QSharedData>
#include <QtCore/QScopedPointer>

/**
  * Private class used to make AnonSeq implicitly shared
  */
class AnonSeqPrivate : public QSharedData
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    AnonSeqPrivate(int id, const BioString &bioString);
    AnonSeqPrivate(int id, BioString *bioString);
    AnonSeqPrivate(const AnonSeqPrivate &other);

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    AnonSeqPrivate &operator=(const AnonSeqPrivate &other);

    // Since this is a private class, publicly expose the member variables
    int id_;                                  //!< User-defined identifier
    QScopedPointer<BioString> bioString_;     //!< Scoped pointer to internal BioString object
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * Call the virtual clone method because AnonSeq must own a local copy of a BioString. Since
  * the client may have passed in a derived instance of BioString, use the clone method to ensure
  * the correct polymorphic type.
  *
  * @param id [int]
  * @param bioString [const BioString &]
  */
AnonSeqPrivate::AnonSeqPrivate(int id, const BioString &bioString) : id_(id), bioString_(bioString.clone())
{
    *bioString_ = bioString_->reduced();
}

/**
  * @param id [int]
  * @param bioString [BioString *]
  */
AnonSeqPrivate::AnonSeqPrivate(int id, BioString *bioString) : id_(id), bioString_(bioString)
{
    Q_ASSERT_X(bioString_.isNull() == false, "AnonSeqPrivate::AnonSeqPrivate", "bioString pointer must not be null");

    *bioString_ = bioString_->reduced();
}


/**
  * @param other [const AnonSeqPrivate &]
  */
AnonSeqPrivate::AnonSeqPrivate(const AnonSeqPrivate &other) : QSharedData(other), id_(other.id_)
{
    bioString_.reset(other.bioString_->clone());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Assignment operator
/**
  * @param other [const AnonSeqPrivate &]
  */
AnonSeqPrivate &AnonSeqPrivate::operator=(const AnonSeqPrivate &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    id_ = other.id_;
    bioString_.reset(other.bioString_->clone());

    return *this;
}







// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// AnonSeq methods


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
AnonSeq::AnonSeq() : d_(new AnonSeqPrivate(0, ""))
{
}

/**
  * All AnonSeqs must have an integral identifier and contain a BioString object. The underlying sequence data
  * is purely sequence data and thus all invalid characters are masked.
  *
  * @param id [int] an arbitrary, internal identifier
  * @param sequence [const BioString &] the source sequence
  * @see AnonSeq(int, BioString *)
  */
AnonSeq::AnonSeq(int id, const BioString &bioString) : d_(new AnonSeqPrivate(id, bioString))
{
}

/**
  * All AnonSeqs must have an integral identifier and contain a BioString object. This function differs from its
  * sibling constructor, AnonSeq(id, const BioString &), by taking a pointer rather than a constant reference.
  * In this case, the AnonSeq object takes ownership of bioString and does not perform a clone operation. Thus,
  * it is a more lean operation.
  *
  * @param id [int]
  * @param bioString [BioString *]
  * @see AnonSeq(int, const BioString &)
  */
AnonSeq::AnonSeq(int id, BioString *bioString) : d_(new AnonSeqPrivate(id, bioString))
{
}

/**
  * @param other [const AnonSeq] reference to the AnonSeq to be copied (or rather cloned in this case)
  */
AnonSeq::AnonSeq(const AnonSeq &other) : d_(other.d_)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * Must be defined as out-of-line for the implicit sharing to function properly
  */
AnonSeq::~AnonSeq()
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Assignment operators
/**
  * Assigns other to the current object
  * @param other reference to AnonSeq to be assigned
  */
AnonSeq &AnonSeq::operator=(const AnonSeq &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    d_ = other.d_;

    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Quite frequently it will be necessary to directly access and work with the underlying sequence data, thus
  * this function provides a immutable BioString pointer to the associated BioString. This can then be used to
  * inspect and retrieve associated data.
  *
  * @returns const BioString &
  */
const BioString &AnonSeq::bioString() const
{
    return *(d_->bioString_);
}

/**
  * Returns the internally assigned identifier that was given on construction.
  *
  * @return int
  */
int AnonSeq::id() const
{
    return d_->id_;
}

/**
  * A valid AnonSeq consists of a non-empty sequence.
  *
  * @returns bool
  */
bool AnonSeq::isValid() const
{
    return (d_->bioString_->length() > 0);
}
