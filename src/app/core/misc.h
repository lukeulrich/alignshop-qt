/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MISC_H
#define MISC_H

#include <QtCore/QPair>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "util/ClosedIntRange.h"
#include "types.h"

class QByteArray;
class QDataStream;

//! Converts a vector of integers into a vector of pair's of integers describing their ranges
QVector<QPair<int, int> > convertIntVectorToRanges(QVector<int> intVector);
QVector<ClosedIntRange> convertIntVectorToClosedIntRanges(QVector<int> intVector);
//! Iterates through all values in vectorHashCharInt, divides them by divisor, and returns the resulting dividend set; divisor must not be zero
VectorHashCharDouble divideVectorHashCharInt(const VectorHashCharInt &vectorHashCharInt, int divisor);
QPoint floorPoint(const QPointF &point);                //!< Converts the floating point, point, to a QPoint by flooring its x and y values
QString formatTimeRunning(const int seconds);           //!< Converts a number of seconds into a corresponding time string showing days, hours, minutes, and seconds in a human friendly manner
bool isGapCharacter(char ch);                           //!< Returns true if ch is a gap character; false otherwise
int randomInteger(int minimum, int maximum);            //!< Returns a random integer between minimum and maximum inclusive
void removeWhiteSpace(QByteArray &byteArray);           //!< Removes all whitespace from byteArray
void removeWhiteSpace(QString &string);                 //!< Removes all whitespace from string
double round(const double value, const int decimals);   //!< Rounds value with decimals remaining

//! Calls write as necessary to completely write length bytes from data to device; returns the number of bytes successfully written or -1 if an error occurred
qint64 writeAll(QIODevice &device, const char *data, qint64 length);
qint64 writeAll(QIODevice &device, const char *data);   //!< Convenience method that calls writeAll(device, data, qstrlen(data))
//! Calls write as necessary to completely write byteArray to device; returns the number of bytes successfully written or -1 if an error occurred
qint64 writeAll(QIODevice &device, const QByteArray &byteArray);

QString readBetaLicense();
QDataStream &operator<<(QDataStream &out, char ch);
QDataStream &operator>>(QDataStream &in, char &ch);


template<typename T>
bool isBetween(const T &min, const T &value, const T &max)
{
    return value >= min && value <= max;
}

#endif // MISC_H
