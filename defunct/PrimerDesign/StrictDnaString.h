/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef STRICTDNASTRING_H
#define STRICTDNASTRING_H

#include "global.h"

namespace PrimerDesign
{
    /// A method which validates a string.
    typedef bool(*StringValidator)(const QString &);

    /// A string which is guaranteed to either be a valid DNA sequence, or blank.
    class StrictDnaString
    {
    public:
        // ------------------------------------------------------------------------------------------------
        // Constructors
        /// Constructs an empty StrictDnaString with a validator
        /// @param validator StringValidator
        StrictDnaString(StringValidator validator);

        /// Constructs a StrictDnaString from a QString
        /// @param value const QString & (Defaults to QString().)
        StrictDnaString(const QString &value = QString());

        /// Constructs a StrictDnaString from a char *
        /// @param value const char *
        StrictDnaString(const char *value);

        // ------------------------------------------------------------------------------------------------
        // Operators
        /// Casts to a QString.
        /// @return operator const
        operator const QString() const;

        // ------------------------------------------------------------------------------------------------
        // Properties
        /// Gets the value.
        /// @return QString
        QString value() const;

        /// Sets the value.
        /// @param value const QString &
        /// @return bool
        bool setValue(const QString &value);

        // ------------------------------------------------------------------------------------------------
        // Static Methods
        /// Gets whether or not the sequence is empty.
        /// @param value const QString &
        /// @return bool
        static bool isNotEmpty(const QString &value);

        // ------------------------------------------------------------------------------------------------
    private:
        QString value_;
        StringValidator customValidator_;
    };
}

#endif // STRICTDNASTRING_H
