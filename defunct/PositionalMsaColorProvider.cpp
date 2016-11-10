/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "PositionalMsaColorProvider.h"

#include "Msa.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
PositionalMsaColorProvider::~PositionalMsaColorProvider()
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Default implementation always returns black foreground and white background.
  *
  * @param msa [const Msa &]
  * @param row [int]
  * @param column [int]
  * @returns TextColorStyle
  */
TextColorStyle PositionalMsaColorProvider::color(const Msa & /* msa */, int /* row */, int /* column */) const
{
    return TextColorStyle(Qt::black, Qt::white);
}
