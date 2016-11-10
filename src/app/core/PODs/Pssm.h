/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSSM_H
#define PSSM_H

#include <QtCore/QVector>

#include "../constants.h"

/**
  * Each row contains an integer score for the 20 amino acids.
  */
struct PssmRow
{
    int scores_[constants::kPssmWidth];
};

Q_DECLARE_TYPEINFO(PssmRow, Q_PRIMITIVE_TYPE);

/**
  */
struct Pssm
{
    double positScaleFactor_;
    QVector<char> mapping_;         // Mapping of characters to their corresponding indices in the scores_ array of
                                    // each PssmRow; should be alphabetical
    QVector<PssmRow> rows_;

    bool isEmpty() const
    {
        return rows_.isEmpty();
    }
};

Q_DECLARE_TYPEINFO(Pssm, Q_PRIMITIVE_TYPE);

#endif // PSSM_H
