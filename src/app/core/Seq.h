/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEQ_H
#define SEQ_H

#include <QtCore/QByteArray>

#include "BioString.h"

/**
  * Full length, source-agnostic, ungapped, fixed representation of a biological sequence.
  */
class Seq : private BioString
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    Seq();                                                                      //!< Construct a completely empty Seq
    Seq(const BioString &bioString);                                            //!< Construct a Seq from bioString
    Seq(const char *str, Grammar grammar = eUnknownGrammar);                    //!< Construct a Seq from str and with grammar
    // **NOTE: Dumb global scope oeperator :: needed here for compilation reasons. Without it gcc complains with the
    //         error: 'class QByteArray' is inaccessible
    //         I believe this is due to it being confused about accessing QByteArray from a privately inherited
    //         perspective vs its actual use as a separate class.
    Seq(const ::QByteArray &byteArray, Grammar grammar = eUnknownGrammar);      //!< Construct a Seq from byteArray and with grammar


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const Seq &other) const       {   return BioString::operator==(static_cast<BioString>(other));    }
    bool operator!=(const Seq &other) const       {   return !operator==(other);                                      }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString toBioString() const;                                              //!< Creates a copy of this Seq as a BioString


    // ------------------------------------------------------------------------------------------------
    // Re-exposed BioString operators
    using BioString::operator==;
    using BioString::operator!=;
    using BioString::operator[];
    using BioString::operator<;
    using BioString::operator<=;
    using BioString::operator>;
    using BioString::operator>=;


    // ------------------------------------------------------------------------------------------------
    // Re-exposed public methods
    using BioString::asByteArray;
    using BioString::at;
    using BioString::backTranscribe;
    using BioString::capacity;
    using BioString::clear;
    using BioString::constData;
    using BioString::contains;
    using BioString::count;
    using BioString::digest;
    using BioString::endsWith;
    using BioString::grammar;
    using BioString::indexOf;
    using BioString::isEmpty;
    using BioString::isEquivalentTo;
    using BioString::isExactMatch;
    using BioString::isNull;
    using BioString::isValidPosition;
    using BioString::lastIndexOf;
    using BioString::left;
    using BioString::length;
    using BioString::mid;
    using BioString::reserve;
    using BioString::reverseComplement;
    using BioString::right;
    using BioString::split;
    using BioString::squeeze;
    using BioString::startsWith;
    using BioString::transcribe;
};

#endif // SEQ_H
