/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAALGORITHMS_H
#define MSAALGORITHMS_H

#include "PosiRect.h"
#include "../CharCountDistribution.h"
#include "../types.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;

// ------------------------------------------------------------------------------------------------
// Public functions
//! Computes and returns the character count distribution type of msa within the area specified by msaRects
CharCountDistribution calculateMsaCharCountDistribution(const Msa &msa, const PosiRect &msaRect = PosiRect());

#endif // MSAALGORITHMS_H
