/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INFOUNIT_H
#define INFOUNIT_H

#include <QtCore/QVector>
#include "../macros.h"

/**
  * Null terminating character is not allowed.
  *
  */
struct InfoUnit
{
    char ch_;               //!< The character for this information
    double percent_;        //!< Percentage this character occurs
    double info_;           //!< Information contributed by this character

    InfoUnit() : ch_('\0'), percent_(0.), info_(0.)
    {
    }

    InfoUnit(char ch, double percent, double info)
        : ch_(ch), percent_(percent), info_(info)
    {
        ASSERT_X(percent >= 0 && percent <= 1., "percent out of range");
        ASSERT_X(info >= 0, "info must be positive");
    }

    bool operator==(const InfoUnit &other) const
    {
        if (this == &other)
            return true;

        return ch_ == other.ch_ &&
                percent_ == other.percent_ &&
                info_ == other.info_;
    }

    bool operator!=(const InfoUnit &other) const
    {
        return !operator==(other);
    }

    bool isNull() const
    {
        return ch_ == '\0';
    }
};
typedef QVector<InfoUnit> VectorInfoUnit;
typedef QVector<QVector<InfoUnit> > VectorVectorInfoUnit;

Q_DECLARE_TYPEINFO(InfoUnit, Q_MOVABLE_TYPE);

#endif // INFOUNIT_H
