/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

#include <cmath>        // For the floor function
#include <locale>       // For the isspace function

#include "constants.h"
#include "macros.h"
#include "misc.h"


/**
  * Sorts and combines a vector of integers into ranges. Two or more integers that differ by 1 from the previous or
  * next integer in the vector will be combined into a pair with the first number the minimum value and the second
  * number the maximum value.
  *
  * For example:
  * (3, 4, 5, 6) -> (3, 6)
  * (1, 2, 3, 10, 15, 16, 21, 23) -> ((1, 3), (10, 10), (15, 16), (21, 21), (23, 23))
  *
  * Duplicates are treated as a single number:
  * (1, 1, 2, 2, 3) -> (1, 3)
  *
  * @param intVector [const QVector &]
  * @returns QVector<QPair<int, int> >
  */
QVector<QPair<int, int> > convertIntVectorToRanges(QVector<int> intVector)
{
    if (intVector.isEmpty())
        return QVector<QPair<int, int> >();

    qStableSort(intVector);

    QVector<QPair<int, int> > ranges;
    ranges << QPair<int, int>(intVector.at(0), intVector.at(0));
    for (int i=1, z=intVector.size(); i<z; ++i)
    {
        if (ranges.last().second + 1 == intVector.at(i))
            ++ranges.last().second;
        else if (ranges.last().second != intVector.at(i))
            ranges << QPair<int, int>(intVector.at(i), intVector.at(i));
    }

    return ranges;
}

QVector<ClosedIntRange> convertIntVectorToClosedIntRanges(QVector<int> intVector)
{
    if (intVector.isEmpty())
        return QVector<ClosedIntRange>();

    qStableSort(intVector);

    QVector<ClosedIntRange> ranges;
    ranges << ClosedIntRange(intVector.at(0), intVector.at(0));
    for (int i=1, z=intVector.size(); i<z; ++i)
    {
        if (ranges.last().end_ + 1 == intVector.at(i))
            ++ranges.last().end_;
        else if (ranges.last().end_ != intVector.at(i))
            ranges << ClosedIntRange(intVector.at(i), intVector.at(i));
    }

    return ranges;
}


/**
  * @param vectorHashCharInt [const VectorHashCharInt &]
  * @param divisor [int]
  * @returns VectorHashCharDouble
  */
VectorHashCharDouble divideVectorHashCharInt(const VectorHashCharInt &vectorHashCharInt, int divisor)
{
    ASSERT_X(divisor != 0, "divisor may not be zero");

    int columns = vectorHashCharInt.size();
    VectorHashCharDouble dividend(columns);
    for (int i=0; i< columns; ++i)
    {
        const HashCharInt &hashCharInt = vectorHashCharInt.at(i);

        HashCharInt::ConstIterator jt;
        for (jt = hashCharInt.constBegin(); jt != hashCharInt.constEnd(); ++jt)
        {
            // jt == iterator
            // jt.key() == character
            // jt.value() == count of character
            dividend[i].insert(jt.key(), static_cast<qreal>(jt.value()) / static_cast<qreal>(divisor));
        }
    }

    return dividend;
}

/**
  * @param point [const QPointF &]
  * @returns QPoint
  */
QPoint floorPoint(const QPointF &point)
{
    return QPoint(floor(point.x()),
                  floor(point.y()));
}

/**
  * For example:
  * 2 days, 3 hours, 54:32
  *
  * @param seconds [const int]
  * @returns QString
  */
QString formatTimeRunning(const int seconds)
{
    int remainder = seconds;

    int d = 0;  // days
    int h = 0;  // hours
    int m = 0;  // minutes
    int s = 0;  // seconds

    if (remainder > constants::kSecondsPerDay)
    {
        d = remainder / constants::kSecondsPerDay;
        remainder -= (d * constants::kSecondsPerDay);
    }

    if (remainder > constants::kSecondsPerHour)
    {
        h = remainder / constants::kSecondsPerHour;
        remainder -= (h * constants::kSecondsPerHour);
    }

    if (remainder > constants::kSecondsPerMinute)
    {
        m = remainder / constants::kSecondsPerMinute;
        remainder -= (m * constants::kSecondsPerMinute);
    }

    s = remainder;


    QString secondString = QString::number(s);
    if (m > 0)
        secondString = secondString.rightJustified(2, QChar('0'));
    QString minuteString = QString::number(m);
    QString hourString = QString::number(h) + " hour";
    QString dayString = QString::number(d) + " day";

    if (m == 0)
    {
        secondString += " second";
        if (s > 1 || s == 0)
            secondString += "s";
    }
    else if (m == 1)
        secondString += " minute";
    else
        secondString += " minutes";
    if (h > 1)
        hourString += "s";
    if (d > 1)
        dayString += "s";

    if (d)
        return QString("%1, %2, %3:%4").arg(dayString).arg(hourString).arg(minuteString).arg(secondString);
    else if (h)
        return QString("%1, %2:%3").arg(hourString).arg(minuteString).arg(secondString);
    else if (m == 0)
        return QString("%1").arg(secondString);
    else
        return QString("%1:%2").arg(minuteString).arg(secondString);
}

/**
  * @param ch [char]
  * @returns bool
  */
bool isGapCharacter(char ch)
{
    const char *x = constants::kGapCharacters;
    while (*x)
    {
        if (*x == ch)
            return true;

        ++x;
    }

    return false;
}

/**
  * Returns a random integer between minimum and maximum inclusive.
  *
  * @param minimum [int]
  * @param maximum [int]
  * @returns int
  */
int randomInteger(int minimum, int maximum)
{
    ASSERT(maximum >= minimum);

    return qrand() % ((maximum + 1) - minimum) + minimum;
}

/**
  * @param byteArray [QByteArray &]
  */
void removeWhiteSpace(QByteArray &byteArray)
{
    const char *x = byteArray.constData();
    char *y = byteArray.data();
    int newSize = 0;

    for (; *x; ++x)
    {
        if (!isspace(*x))
        {
            *y = *x;
            ++y;
            ++newSize;
        }
    }
    byteArray.resize(newSize);
}

void removeWhiteSpace(QString &string)
{
    const QChar *x = string.constData();
    QChar *y = string.data();
    int newSize = 0;

    for (; !x->isNull(); ++x)
    {
        if (!x->isSpace())
        {
            *y = *x;
            ++y;
            ++newSize;
        }
    }
    string.resize(newSize);
}

/**
  * @param value [const double]
  * @param decimals [const int]
  * @returns double
  */
double round(const double value, const int decimals)
{
    ASSERT(decimals >= 0);

    double factor = pow(10., decimals);
    return static_cast<double>(static_cast<int>(value * factor + .5)) / factor;
}

/**
  * @param device [QIODevice &]
  * @param data [const char *]
  * @param length [qint64]
  * @returns qint64
  */
qint64 writeAll(QIODevice &device, const char *data, qint64 length)
{
    qint64 totalBytesWritten = 0;
    while (totalBytesWritten != length)
    {
        int bytesWritten = device.write(data + totalBytesWritten, length - totalBytesWritten);
        if (bytesWritten == -1)
            return -1;

        totalBytesWritten += bytesWritten;
    }

    return totalBytesWritten;
}

/**
  * @param device [QIODevice &]
  * @param data [const char *]
  * @returns qint64
  */
qint64 writeAll(QIODevice &device, const char *data)
{
    return writeAll(device, data, qstrlen(data));
}

/**
  * @param device [QIODevice &]
  * @param byteArray [const QByteArray &]
  * @returns qint64
  */
qint64 writeAll(QIODevice &device, const QByteArray &byteArray)
{
    return writeAll(device, byteArray.constData(), byteArray.length());
}


QString readBetaLicense()
{
    QFile betaFile(":/BetaLicense.html");
    if (!betaFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    return betaFile.readAll();
}

QDataStream &operator<<(QDataStream &out, const char ch)
{
    qint8 intCh = ch;
    out << intCh;
    return out;
}

QDataStream &operator>>(QDataStream &in, char &ch)
{
    qint8 intCh;
    in >> intCh;
    ch = intCh;
    return in;
}
