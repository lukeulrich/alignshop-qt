/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "OptionProfile.h"

#include <QtDebug>


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  */
OptionProfile::OptionProfile()
    : joinEnabled_(false),
      joinDelimiter_(" ")
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param optionSpec [const OptionSpec &]
  * @returns OptionProfile &
  */
OptionProfile &OptionProfile::operator<<(const OptionSpec &optionSpec)
{
    add(optionSpec);

    return *this;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Does not add invalid OptionSpec's (those with an empty name).
  *
  * @param optionSpec [const OptionSpec &]
  */
void OptionProfile::add(const OptionSpec &optionSpec)
{
    if (!optionSpec.isValid())
        return;

    optionSpecs_.insert(optionSpec.name_, optionSpec);

    if (optionSpec.isDefault_)
    {
        defaultOptionNames_ << optionSpec.name_;
    }
    else
    {
        defaultOptionNames_.remove(optionSpec.name_);
        if (optionSpec.required_)
            requiredOptionNames_ << optionSpec.name_;
    }

    // Remove from the required structure regardless of what its default may be
    if (!optionSpec.required_)
        requiredOptionNames_.remove(optionSpec.name_);
}

/**
  * Does not check if the options are valid, but merely produces a QStringList from the options provided and adding
  * defaults where necessary. Only those options not present in the profile option specifications are skipped.
  *
  * @param options [const QVector<Option> &]
  * @returns QStringList
  */
QStringList OptionProfile::argumentList(const QVector<Option> &options) const
{
    QStringList arguments;

    QSet<QString> observedNames;
    foreach (const Option &option, options)
    {
        if (!optionSpecs_.contains(option.name_))
            continue;

        QString value;

        bool ignoreValue = optionSpecs_.value(option.name_).emptyValue_;
        if (!ignoreValue && !option.value_.isEmpty())
            value = option.value_;

        addToArguments(arguments, option.name_, value);

        observedNames << option.name_;
    }

    // Now add in all default arguments
    foreach (const QString &name, defaultOptionNames_)
    {
        if (observedNames.contains(name))
            continue;

        QString value;

        ASSERT(optionSpecs_.contains(name));
        const OptionSpec &optionSpec = optionSpecs_.value(name);
        ASSERT(optionSpec.isDefault_);
        bool ignoreValue = optionSpec.emptyValue_;
        if (!ignoreValue && !optionSpec.defaultValue_.isEmpty())
            value = optionSpec.defaultValue_;

        addToArguments(arguments, optionSpec.name_, value);
    }

    return arguments;
}

/**
  */
void OptionProfile::clear()
{
    optionSpecs_.clear();
    requiredOptionNames_.clear();
    defaultOptionNames_.clear();
}

/**
  * @returns bool
  */
bool OptionProfile::isEmpty() const
{
    return optionSpecs_.isEmpty();
}

/**
  * @returns bool
  */
bool OptionProfile::isJoinEnabled() const
{
    return joinEnabled_;
}

/**
  * @returns QString
  */
QString OptionProfile::joinDelimiter() const
{
    return joinDelimiter_;
}

/**
  * @param delimiter [const QString &]
  */
void OptionProfile::setJoinDelimiter(const QString &delimiter)
{
    joinDelimiter_ = delimiter;
}

/**
  * @param enabled [bool]
  */
void OptionProfile::setJoinEnabled(bool enabled)
{
    joinEnabled_ = enabled;
}

/**
  * @param option [const Option &]
  * @returns bool
  */
bool OptionProfile::validOption(const Option &option) const
{
    if (!optionSpecs_.contains(option.name_))
        return false;

    const OptionSpec &optionSpec = optionSpecs_.value(option.name_);
    if (optionSpec.emptyValue_)
        return true;

    if (option.value_.isEmpty())
        return false;

    return optionSpec.valueRegex_.exactMatch(option.value_);
}

/**
  * @param options [const QVector<Option> &]
  * @returns bool
  */
bool OptionProfile::validOptions(const QVector<Option> &options) const
{
    if (options.isEmpty())
        return false;

    // Keep track of the option names so that if the same name is seen twice and allowDuplicate_
    // is false, we can return false
    QSet<QString> names;
    names.reserve(options.size());  // Slight optimization

    foreach (const Option &option, options)
    {
        if (!validOption(option))
            return false;

        // If we get here, it is a valid option and therefore must exist in the optionSpecs_ hash.
        // Duplicate check
        ASSERT(optionSpecs_.contains(option.name_));
        if (!optionSpecs_.value(option.name_).allowDuplicate_ &&
            names.contains(option.name_))
        {
            return false;
        }

        names << option.name_;
    }

    // Finally, check if any required options have not been fulfilled
    foreach (const QString &requiredName, requiredOptionNames_)
        if (!names.contains(requiredName))
            return false;

    return true;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param arguments [QStringList &]
  * @param name [const QString &]
  * @param value [const QString &]
  */
void OptionProfile::addToArguments(QStringList &arguments, const QString &name, const QString &value) const
{
    if (!joinEnabled_)
    {
        arguments << name;
        if (!value.isEmpty())
            arguments << value;
    }
    else
    {
        if (value.isEmpty())
            arguments << name;
        else
            arguments << QString("%1%2%3").arg(name, joinDelimiter_, value);
    }
}
