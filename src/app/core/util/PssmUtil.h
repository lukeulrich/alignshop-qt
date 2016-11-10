/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSSMUTIL_H
#define PSSMUTIL_H

#include "../PODs/NormalizedPssm.h"

struct Pssm;

NormalizedPssm normalizePssm(const Pssm &pssm);     //! Creates an equivalently dimensioned NormalizedPssm by scaling each pssm score by its posit scale factor and mapping this value between 0 and 1 using the logitistic function

#endif // PSSMUTIL_H
