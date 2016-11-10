/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef OPTIONSPEC_H
#define OPTIONSPEC_H

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include "../macros.h"

struct OptionSpec
{
    QString name_;
    bool required_;
    bool emptyValue_;
    bool isDefault_;            // Use option if not provided by user
    QString defaultValue_;
    QRegExp valueRegex_;
    bool allowDuplicate_;

    OptionSpec()
        : required_(false),
          emptyValue_(true),
          isDefault_(false),
          allowDuplicate_(false)
    {
    }

    OptionSpec(const QString &name,
               bool required = false,
               bool emptyValue_ = true,
               bool isDefault = false,
               const QString &defaultValue = QString(),
               const QRegExp &regex = QRegExp("^.*$"),
               bool allowDuplicate = false)
        : name_(name.trimmed()),
          required_(required),
          emptyValue_(emptyValue_),
          isDefault_(isDefault),
          defaultValue_(defaultValue),
          valueRegex_(regex),
          allowDuplicate_(allowDuplicate)
    {
        ASSERT(name_.isEmpty() == false);
        if (defaultValue_.isEmpty() == false)
            ASSERT_X(valueRegex_.exactMatch(defaultValue_), "Default value does not match regex");
    }

    bool operator==(const OptionSpec &other) const
    {
        return name_ == other.name_ &&
               required_ == other.required_ &&
               emptyValue_ == other.emptyValue_ &&
               isDefault_ == other.isDefault_ &&
               defaultValue_ == other.defaultValue_ &&
               valueRegex_ == other.valueRegex_ &&
               allowDuplicate_ == other.allowDuplicate_;
    }

    bool operator!=(const OptionSpec &other) const
    {
        return !operator==(other);
    }

    bool isValid() const
    {
        return !name_.isEmpty();
    }
};

Q_DECLARE_TYPEINFO(OptionSpec, Q_MOVABLE_TYPE);

#endif // OPTIONSPEC_H
