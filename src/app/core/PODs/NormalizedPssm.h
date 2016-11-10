/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NORMALIZEDPSSM_H
#define NORMALIZEDPSSM_H

#include <QtCore/QVector>

#include "../constants.h"

struct NormalizedPssmRow
{
    double scores_[constants::kPssmWidth];
};

Q_DECLARE_TYPEINFO(NormalizedPssmRow, Q_PRIMITIVE_TYPE);

struct NormalizedPssm
{
    QVector<char> mapping_;
    QVector<NormalizedPssmRow> rows_;

    bool isEmpty() const
    {
        return rows_.isEmpty();
    }
};

Q_DECLARE_TYPEINFO(NormalizedPssm, Q_PRIMITIVE_TYPE);

#endif // NORMALIZEDPSSM_H
