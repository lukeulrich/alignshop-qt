/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "utility.h"

/**
  * This function does not modify fileName whatsoever but checks it as is. Thus, any desired formatting
  * should be performed prior to calling this function. fileName should not contain any path information
  * or false will be returned because slashes will be considered invalid characters.
  *
  * Loosely based on the recommendations provided by the Boost library:
  * http://www.boost.org/doc/libs/1_43_0/libs/filesystem/doc/portability_guide.htm
  *
  * @param filename [const QString &]
  * @returns bool
  */
bool portableFileName(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    const QChar *x = fileName.constData();

    // Ensure that first character is not period, hyphen, or space
    if (*x == '.'
        || *x == '-'
        || *x == ' ')
    {
        return false;
    }

    // Ensure that it does not end with a space
    if (*(x + fileName.length() - 1) == ' ')
        return false;

    // Walk through each character and return false if it is invalid
    while (*x != '\0')
    {
        if ((*x >= 'A' && *x <= 'Z')
            || (*x >= 'a' && *x <= 'z')
            || (*x >= '0' && *x <= '9')
            || *x == '.'
            || *x == '_'
            || *x == '-'
            || *x == ' ')
        {
            ++x;
        }
        else
        {
            return false;
        }
    }

    return true;
}
