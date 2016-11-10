/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef OPTION_H
#define OPTION_H

#include <QtCore/QString>

struct Option
{
    QString name_;
    QString value_;

    Option()
    {
    }

    Option(const QString &name, const QString &value = QString())
        : name_(name.trimmed()),
          value_(value)
    {
    }

    Option(const QString &name, const int value)
        : name_(name.trimmed()),
          value_(QString::number(value))
    {
    }

    Option(const QString &name, const double value)
        : name_(name.trimmed()),
          value_(QString::number(value))
    {
    }

    bool operator==(const Option &other) const
    {
        return name_ == other.name_ &&
               value_ == other.value_;
    }

    bool operator!=(const Option &other) const
    {
        return name_ != other.name_ ||
               value_ != other.value_;
    }
};

Q_DECLARE_TYPEINFO(Option, Q_MOVABLE_TYPE);

#endif // OPTION_H
