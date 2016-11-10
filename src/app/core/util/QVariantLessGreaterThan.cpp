/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QVariant>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QTime>

bool QVariantLessThan(const QVariant &a, const QVariant &b)
{
    int aType = a.userType();
    int bType = b.userType();
    switch (aType)
    {
    case QVariant::Invalid:
        return bType != QVariant::Invalid;
    case QVariant::Bool:
        return a.toBool() < b.toBool();
    case QVariant::ByteArray:
        return a.toByteArray() < b.toByteArray();
    case QVariant::Char:
        return a.toChar() < b.toChar();
    case QVariant::Date:
        return a.toDate() < b.toDate();
    case QVariant::DateTime:
        return a.toDateTime() < b.toDateTime();
    case QVariant::Double:
        return a.toDouble() < b.toDouble();
    case QVariant::Int:
        return a.toInt() < b.toInt();
    case QVariant::LongLong:
        return a.toLongLong() < b.toLongLong();
    case QVariant::String:
        return a.toString() < b.toString();
    case QVariant::Time:
        return a.toTime() < b.toTime();
    case QVariant::UInt:
        return a.toInt() < b.toInt();
    case QVariant::ULongLong:
        return a.toULongLong() < b.toULongLong();

    default:
        return false;
    }
}

bool QVariantGreaterThan(const QVariant &a, const QVariant &b)
{
    return QVariantLessThan(b, a);
}
