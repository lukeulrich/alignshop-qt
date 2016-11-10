/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef PARSEDBIOSTRING_H
#define PARSEDBIOSTRING_H

#include "BioString.h"

/**
  * ParsedBioString associates several pieces of information common to a BioString that has been parsed
  * and while it may be used elsewhere, it's primary function is for importing sequence data.
  *
  * Generally speaking, ParsedBioString is a glorified SimpleSeq that additionally contains valid and
  * checked flags, which denote whether it is valid and/or selected, respectively.
  */
struct ParsedBioString
{
    BioString bioString_;   //!< Parsed bioString
    QString header_;        //!< Arbitrary header data
    bool valid_;            //!< Flag denoting if this sequence is valid
    bool checked_;          //!< Flag denoting if user has selected this sequence

    /**
      * Constructs a a ParsedBioString with bioString, header, valid, and checked status
      */
    ParsedBioString(const BioString &bioString, const QString &header = QString(), bool valid = false, bool checked = false) :
            bioString_(bioString), header_(header), valid_(valid), checked_(checked)
    {}
};

#endif // PARSEDBIOSTRING_H
