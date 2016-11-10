/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef PICKER_H
#define PICKER_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPair>

/**
  * Returns the most frequently occurring object(s) of type T from a list of objects of T.
  *
  * This differs from the mathematical mode in that even if all object occurs just once, the list
  * of all objects is returned.
  *
  * Limitations:
  * o T must be an atomic type or have a default constructed value
  * o objects must contain at least one item otherwise the default constructed value is returned.
  *   Assertion is thrown in debug mode.
  *
  * @param objects [const QList<T> &]
  * @returns QList<T>
  */
template <typename T>
QList<T> pickMostFrequent(const QList<T> &objects)
{
    // Catch and prevent an empty-list in debug mode
    Q_ASSERT_X(objects.size() > 0, "pickMostFrequent", "objects must not be an empty list");

    // Build the distribution of
    QHash<T, int> counts;
    for (int i=0, z=objects.size(); i<z; ++i)
    {
        if (counts.contains(objects.at(i)))
            ++counts[objects.at(i)];
        else
            counts[objects.at(i)] = 1;
    }

    QPair<QList<T>, int> most;
    most.second = 0;

    QHashIterator<T, int> i(counts);
    while (i.hasNext())
    {
        i.next();

        if (i.value() > most.second)
        {
            most.first.clear();
            most.first.append(i.key());
            most.second = i.value();
        }
        else if (i.value() == most.second)
            most.first.append(i.key());
    }

    return most.first;
}


/**
  * Returns the least frequently occurring object of type T from a list of objects of T.
  *
  * Limitations:
  * o T must be an atomic type or have a default constructed value
  * o objects must contain at least one item otherwise the default constructed value is returned.
  *   Assertion is thrown in debug mode.
  *
  * @param objects [const QList<T> &]
  * @returns T
  *
  * Unimplemented - left for reference should it be desired
  */
/*
template <typename T>
T pickLeastFrequent(const QList<T> &objects)
{
    T a;
    return a;
}
*/

#endif // PICKER_H
