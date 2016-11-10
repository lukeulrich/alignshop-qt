/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RICHMSA_H
#define RICHMSA_H

#include <QtCore/QList>
#include <QtCore/QObject>

#include "DataRow.h"
#include "Msa.h"
#include "RichSubseq.h"

/**
  * RichMsa behaves identically to Msa except it provides a publicly accessible Msa annotation and
  * only accepts RichSubseq members.
  *
  * To prevent client code from adding Subseq instances, the Msa append, insert, and prepend virtual methods
  * perform nothing and always return false. To enable the addition of RichSubseq instances, methods with
  * these same names have been created and simply call the Msa::equivalent.
  *
  * For convenience, the bracket operator[] simply casts all member pointers to their RichSubseq form.
  */
class RichMsa : public Msa
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    RichMsa(Alphabet alphabet, int id, QObject *parent = 0);    //!< Construct a RichMsa with the given alphabet, id, and parent

    // ------------------------------------------------------------------------------------------------
    // Operators
    const RichSubseq *operator()(int i) const;                  //!< Return a const reference to the RichSubseq at index i (1-based)

    // ------------------------------------------------------------------------------------------------
    // Public methods
    // Because a RichMsa may only contain RichSubseq members, the append, insert, and prepend methods function similar to their
    // parent class equivalents except they only accept a RichSubseq pointer. To prevent the user from appending a raw Subseq,
    // the same methods that take Subseq pointers are declared private and do nothing.
    const RichSubseq *at(int i) const;                          //!< Return a const reference to the RichSubseq at index i (1-based); identical to operator()
    bool append(RichSubseq *richSubseq);                        //!< Add richSubseq and return true on success or false otherwise
    bool insert(int i, RichSubseq *richSubseq);                 //!< Insert richSubseq at row index i (1-based), and return whether the addition was successful
    bool prepend(RichSubseq *richSubseq);                       //!< Add richSubseq at the beginning of the Msa and return whether the addition was successful

    // ------------------------------------------------------------------------------------------------
    // Public members
    DataRow annotation_;                                        //!< Specific MSA annotation

private:
    // ------------------------------------------------------------------------------------------------
    // Private reimplmented methods - captured to prevent calling classes from adding Subseq pointers. If this precaution
    // was not taken, it would be possible for a RichMsa to contain both RichSubseqs and Subseqs, which could ultimately
    // lead to a segmentation fault.
    //! Stub function that does nothing when attempting to append subseq; returns false
    bool append(Subseq *subseq)             { Q_UNUSED(subseq); return false; }
    //! Stub function that does nothing when attempting to insert subseq; returns false
    bool insert(int i, Subseq *subseq)      { Q_UNUSED(i); Q_UNUSED(subseq); return false; }
    //! Stub function that does nothing when attempting to prepend subseq; returns false
    bool prepend(Subseq *subseq)            { Q_UNUSED(subseq); return false; }
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param alphabet [Alphabet]
  * @param id [int]
  * @param parent [QObject *]
  */
inline
RichMsa::RichMsa(Alphabet alphabet, int id, QObject *parent) : Msa(alphabet, id, parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * Utility function for accessing the RichSubseq pointer via its index position i (1-based).
  *
  * @param i [int]
  * @returns const RichSubseq *
  * @see at()
  */
inline
const RichSubseq *RichMsa::operator()(int i) const
{
    return static_cast<const RichSubseq *>(Msa::operator()(i));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Utility function for accessing the RichSubseq pointer via its index position i (1-based). Code
  * reproduced from operator() for performance reasons.
  *
  * @param i [int]
  * @returns const RichSubseq *
  * @see operator()
  */
inline
const RichSubseq *RichMsa::at(int i) const
{
    return static_cast<const RichSubseq *>(Msa::at(i));
}

/**
  * @param richSubseq [RichSubseq *]
  * @returns bool
  */
inline
bool RichMsa::append(RichSubseq *richSubseq)
{
    return Msa::append(richSubseq);
}

/**
  * @param i [int]
  * @param richSubseq [RichSubseq *]
  * @returns bool
  */
inline
bool RichMsa::insert(int i, RichSubseq *richSubseq)
{
    return Msa::insert(i, richSubseq);
}

/**
  * @param richSubseq [RichSubseq *]
  * @returns bool
  */
inline
bool RichMsa::prepend(RichSubseq *richSubseq)
{
    return Msa::prepend(richSubseq);
}


#endif // RICHMSA_H
