/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef POSITIONALMSACOLORPROVIDER_H
#define POSITIONALMSACOLORPROVIDER_H

#include "TextColorStyle.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;

/**
  * PositionalMsaColorProvider defines a abstract and default concrete implementation for returning colors based on a
  * specific position within a user-supplied Msa.
  */
class PositionalMsaColorProvider
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    virtual ~PositionalMsaColorProvider();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual TextColorStyle color(const Msa &msa, int row, int column) const;        //!< Returns the text color style for the row and column position within msa
};

#endif // POSITIONALMSACOLORPROVIDER_H
