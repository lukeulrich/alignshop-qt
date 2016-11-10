/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "OptionSet.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param option [const Option &]
  * @returns OptionSet &
  */
OptionSet &OptionSet::operator<<(const Option &option)
{
    add(option);

    return *this;
}

OptionSet &OptionSet::operator<<(const OptionSet &otherOptionSet)
{
    options_ << otherOptionSet.options_;

    return *this;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param option [const Option &]
  */
void OptionSet::add(const Option &option)
{
    options_ << option;
}

/**
  */
void OptionSet::clear()
{
    options_.clear();
}

/**
  * @param name [const QString &]
  * @returns bool
  */
bool OptionSet::contains(const QString &name) const
{
    foreach (const Option &option, options_)
        if (option.name_ == name)
            return true;

    return false;
}

/**
  * @param name [const QString &]
  * @param value [const QString &]
  * @returns bool
  */
bool OptionSet::contains(const QString &name, const QString &value) const
{
    return options_.indexOf(Option(name, value)) != -1;
}

/**
  * @returns bool
  */
bool OptionSet::isEmpty() const
{
    return options_.isEmpty();
}

/**
  * @returns QVector<Option>
  */
QVector<Option> OptionSet::asVector() const
{
    return options_;
}

/**
  * @param name [const QString &]
  */
void OptionSet::remove(const QString &name)
{
    for (int i=options_.size()-1; i>=0; --i)
        if (options_.at(i).name_ == name)
            options_.remove(i, 1);
}

/**
  * @param name [const QString &]
  * @param value [const QString &]
  */
void OptionSet::remove(const QString &name, const QString &value)
{
    for (int i=options_.size()-1; i>=0; --i)
        if (options_.at(i).name_ == name && options_.at(i).value_ == value)
            options_.remove(i, 1);
}

/**
  */
void OptionSet::set(const QString &name)
{
    remove(name);
    add(Option(name));
}

/**
  * @param name [const QString &]
  * @returns QString
  */
QString OptionSet::value(const QString &name) const
{
    foreach (const Option &option, options_)
        if (option.name_ == name)
            return option.value_;

    return QString();
}
