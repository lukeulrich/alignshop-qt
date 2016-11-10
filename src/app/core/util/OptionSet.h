/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef OPTIONSET_H
#define OPTIONSET_H

#include <QtCore/QVector>

#include "../PODs/Option.h"

class OptionSet
{
public:
    // ------------------------------------------------------------------------------------------------
    // Operators
    OptionSet &operator<<(const Option &option);
    OptionSet &operator<<(const OptionSet &otherOptionSet);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void add(const Option &option);
    void clear();
    bool contains(const QString &name) const;
    bool contains(const QString &name, const QString &value) const;
    bool isEmpty() const;
    QVector<Option> asVector() const;
    void remove(const QString &name);
    void remove(const QString &name, const QString &value);
    void set(const QString &name);
    template<typename T> void set(const QString &name, const T &value);
    QString value(const QString &name) const;                   //!< Returns the value for the first option with name or QString() if an option with name is not found


private:
    QVector<Option> options_;
};

Q_DECLARE_TYPEINFO(OptionSet, Q_MOVABLE_TYPE);

template<typename T>
inline
void OptionSet::set(const QString &name, const T &value)
{
    remove(name);
    add(Option(name, value));
}


#endif // OPTIONSET_H
