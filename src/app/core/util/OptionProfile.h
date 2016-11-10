/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef OPTIONPROFILE_H
#define OPTIONPROFILE_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "../PODs/OptionSpec.h"
#include "../PODs/Option.h"

class OptionProfile
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    OptionProfile();


    // ------------------------------------------------------------------------------------------------
    // Operators
    OptionProfile &operator<<(const OptionSpec &optionSpec);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void add(const OptionSpec &optionSpec);
    QStringList argumentList(const QVector<Option> &options) const;
    void clear();
    bool isEmpty() const;
    bool isJoinEnabled() const;
    QString joinDelimiter() const;
    void setJoinDelimiter(const QString &delimiter);
    void setJoinEnabled(bool enabled = true);
    bool validOption(const Option &option) const;
    bool validOptions(const QVector<Option> &options) const;


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void addToArguments(QStringList &arguments, const QString &name, const QString &value) const;

    QHash<QString, OptionSpec> optionSpecs_;
    QSet<QString> requiredOptionNames_;
    QSet<QString> defaultOptionNames_;
    bool joinEnabled_;
    QString joinDelimiter_;
};

Q_DECLARE_TYPEINFO(OptionProfile, Q_MOVABLE_TYPE);

#endif // OPTIONPROFILE_H
