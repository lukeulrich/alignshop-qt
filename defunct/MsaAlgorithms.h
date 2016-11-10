/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAALGORITHMS_H
#define MSAALGORITHMS_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QRect>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;

// ------------------------------------------------------------------------------------------------
// Public functions
//! Computes and returns the type and number of characters of msa within the area specified by region
ListHashCharInt calculateMsaCharCountDistribution(const Msa &msa, const QRect &region = QRect());

#endif // MSAALGORITHMS_H
