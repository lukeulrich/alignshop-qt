/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ANONSEQFACTORY_H
#define ANONSEQFACTORY_H

#include <QtCore/QCryptographicHash>
#include <QtCore/QScopedPointer>

#include "AnonSeq.h"

/**
  * AnonSeqFactory provides an abstract interface to managing a collection of AnonSeqs to avoid unnecessarily
  * duplicating sequence data and results. This is particularly significant when computing and storing any kind of
  * derived data that is strictly sequence dependent such as secondary structure, and domain predictions.
  *
  * In essence, the collection of AnonSeqs consists of managing three major properties:
  * o id
  * o digest (must be plain text and not binary)
  * o sequence (BioString)
  *
  * The actual implementation is handled with by derived classes.
  *
  * Only the actual sequence data is needed at this level, thus each new sequence is first reduced (BioString::reduced)
  * - removing all gaps and masking invalid characters. Concrete factories must subclass this class and implement the
  * actual mechanism for storing, retrieving, and deleting data.
  *
  * A side benefit of centralizing the storage of AnonSeqs is the generation of unique identifiers for each new AnonSeq.
  * An AnonSeq may be retrieved by either inputting an identifier or passing in an identical sequence.
  */
class AnonSeqFactory
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //< Create an AnonSeqFactory that uses digestAlgorithm as the hash function
    AnonSeqFactory(QCryptographicHash::Algorithm digestAlgorithm);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual AnonSeq add(const BioString &bioString) = 0;            //!< Create (if it does not already exist) or add an AnonSeq via a BioString object
    virtual BioString *bioStringPrototype() const;                  //!< Returns the BioString prototype used by the factory when it needs to construct BioStrings on demand from the associated data store (for instance when fetching a sequence)
    virtual AnonSeq fetch(const BioString &bioString) const = 0;    //!< Retrieve any AnonSeq with an identical reduced sequence as bioString; returns a valid AnonSeq on success or invalid AnonSeq if not found
    virtual AnonSeq fetch(int id) const = 0;                        //!< Retrieve any AnonSeq with the identifier id; returns a valid AnonSeq on success or invalid AnonSeq if id is not found
    virtual int remove(const BioString &bioString) = 0;             //!< Remove the AnonSeq with an identical reduced sequence as bioString and return the number of AnonSeqs removed
    virtual int remove(int id) = 0;                                 //!< Remove any AnonSeq identified by id and return the number of AnonSeqs removed
    virtual void setBioStringPrototype(BioString *bioString);       //!< Sets the BioString prototype to the specified bioString. Takes ownership of the prototype
    virtual int size() const = 0;                                   //!< Returns the number of AnonSeqs in this factory

protected:
    QByteArray base64Digest(const BioString &bioString) const;      //!< Return the base64 encoded digest of bioString
    //!< Utility function for creating a prototyped AnonSeq (if bioStringPrototype_ is set) from id and bioString
    virtual AnonSeq createPrototypedAnonSeq(int id, const BioString &bioString) const;

    QScopedPointer<BioString> bioStringPrototype_;                  //!< BioString prototype to clone when constructing BioStrings on demand
    QCryptographicHash::Algorithm digestAlgorithm_;                 //!< Algorithm to utilize when digesting sequence data
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param digestAlgorithm [QCryptographicHash::Algorithm]
  */
inline
AnonSeqFactory::AnonSeqFactory(QCryptographicHash::Algorithm digestAlgorithm) : digestAlgorithm_(digestAlgorithm)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * AnonSeqFactory maintains ownership of this prototype and thus it is vitally important that client
  * classes do not attempt to delete the pointer returned by this method.
  *
  * @returns BioString *
  * @see setBioStringPrototype()
  */
inline
BioString *AnonSeqFactory::bioStringPrototype() const
{
    return bioStringPrototype_.data();
}

/**
  * @param bioString [BioString *]
  * @see bioStringPrototype()
  */
inline
void AnonSeqFactory::setBioStringPrototype(BioString *bioString)
{
    bioStringPrototype_.reset(bioString);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
inline
QByteArray AnonSeqFactory::base64Digest(const BioString &bioString) const
{
    return QCryptographicHash::hash(bioString.sequence().toAscii(), digestAlgorithm_).toBase64();
}



#endif // ANONSEQFACTORY_H
