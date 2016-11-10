/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVector>
#include <cmath>            // For exp

#include "PssmUtil.h"
#include "../PODs/Pssm.h"
#include "../constants.h"

/**
  * @param pssm [const Pssm &]
  * @returns NormalizedPssm
  */
NormalizedPssm normalizePssm(const Pssm &pssm)
{
    NormalizedPssm nPssm;
    nPssm.mapping_ = pssm.mapping_;

    // Because multiplication is faster than division, convert what would normally be a division operation to a
    // multiplication factor.
    double multFactor = 1. / pssm.positScaleFactor_;

    int nRows = pssm.rows_.size();
    nPssm.rows_.resize(nRows);          // Pre-allocate the required space
    for (int i=0; i< nRows; ++i)
    {
        const PssmRow &pssmRow = pssm.rows_.at(i);
        NormalizedPssmRow &nPssmRow = nPssm.rows_[i];
        for (int j=0; j< constants::kPssmWidth; ++j)
        {
            double scaledScore = pssmRow.scores_[j] * multFactor;
            nPssmRow.scores_[j] = 1. / (1. + exp(-.5 * scaledScore));
        }
    }

    return nPssm;
}
