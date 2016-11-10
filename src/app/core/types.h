/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QVector>

class QColor;

typedef QVector<QHash<char, double> > VectorHashCharDouble;
typedef QHash<char, int> HashCharInt;
typedef QVector<QHash<char, int> > VectorHashCharInt;
typedef QPair<char, double> PairCharDouble;
typedef QPair<double, double> PairDouble;
typedef QPair<int, int> PairInt;
typedef QPair<QColor, QColor> PairQColor;

#endif // TYPES_H
