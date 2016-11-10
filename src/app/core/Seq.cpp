/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Seq.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
Seq::Seq() : BioString()
{
}

/**
  * @param bioString [const BioString &]
  */
Seq::Seq(const BioString &bioString) : BioString(bioString.ungapped())
{
}

/**
  * @param str [const char *]
  * @param grammar [Grammar]
  */
Seq::Seq(const char *str, Grammar grammar) : BioString(str, grammar)
{
    removeGaps();
}

/**
  * @param byteArray [const QByteArray &]
  * @param grammar [Grammar]
  */
Seq::Seq(const ::QByteArray &byteArray, Grammar grammar) : BioString(byteArray, grammar)
{
    removeGaps();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioString
  */
BioString Seq::toBioString() const
{
    return BioString(asByteArray(), grammar());
}
