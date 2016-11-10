/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "ClustalFormatInspector.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public functions from base class
/**
  * Rules used to determine if buffer is in the Clustal format:
  * 1. All leading empty lines are ignored
  * 2. First non-whitespace characters should be CLUSTAL and be either the first few characters or the
  *    the appear just after a newline.
  *
  * If these conditions are met, then returns eClustalType, otherwise eUnknownType
  *
  * @param buffer [const QString &]
  * @returns eClustalType or eUnknownType
  */
DataFormatType ClustalFormatInspector::inspect(const QString &buffer) const
{
    int length = buffer.length();
    if (length)
    {
        const QChar *s = buffer.constData();
        const QChar *e = s + length;

        const QChar *x = s - 1;
        while (++x && x != e)
        {
            if (!x->isSpace())
            {
                if ((x == s || *(x-1) == '\n') &&
                    x + 6 < e &&
                        *x == 'C' &&
                    *(x+1) == 'L' &&
                    *(x+2) == 'U' &&
                    *(x+3) == 'S' &&
                    *(x+4) == 'T' &&
                    *(x+5) == 'A' &&
                    *(x+6) == 'L')
                    return eClustalType;
                else
                    return eUnknownFormatType;
            }
        }
    }

    return eUnknownFormatType;
}
