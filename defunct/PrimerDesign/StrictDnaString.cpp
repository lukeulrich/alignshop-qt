/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "StrictDnaString.h"
#include "BioStringValidator.h"

namespace PrimerDesign
{
    /**
      * @param validator [StringValidator]
      */
    StrictDnaString::StrictDnaString(StringValidator validator)
    {
        customValidator_ = validator;
    }

    /**
      * Sets the value to blank, if it is not a valid DNA sequeence.
      *
      * @param value [QString]
      */
    StrictDnaString::StrictDnaString(const QString &value)
        : customValidator_(0)
    {
        setValue(value);
    }

    /**
      * Sets the value to blank, if it is not a valid DNA sequeence.
      *
      * @param value [char*]
      */
    StrictDnaString::StrictDnaString(const char *value)
        : customValidator_(0)
    {
        setValue(value);
    }

    /**
      * @return QString
      */
    StrictDnaString::operator const QString() const
    {
        return value();
    }

    /**
      * @return QString
      */
    QString StrictDnaString::value() const
    {
        return value_;
    }

    /**
      * Returns true if the value was valid and was set, otherwise false.
      *
      * @return bool
      */
    bool StrictDnaString::setValue(const QString &value)
    {
        static BioStringValidator validator(constants::kDnaCharacters);

        QString correctedValue = value.trimmed().toUpper();
        if (validator.isValid(correctedValue) && (customValidator_ == 0 || customValidator_(correctedValue)))
        {
            value_ = correctedValue;
            return true;
        }

        return false;
    }

    /**
      * @param value [QString]
      * @return bool
      */
    bool StrictDnaString::isNotEmpty(const QString &value)
    {
        return !value.isEmpty();
    }
}
