/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "FastaFormatInspector.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public functions from base class
/**
  * Rules used to determine if buffer is in the Fasta format:
  * 1. All leading empty lines are ignored
  * 2. First non-whitespace character should be a greater than symbol '>' and be either the first character or the
  *    the first character after a newline.
  *
  * If these conditions are met, then returns FileFormat::FastaFormat, otherwise FileFormat::UnknownFormat
  *
  * @param buffer [const QString &]
  * @returns FileFormat::FastaFormat or FileFormat::UnknownFormat
  */
DataFormatType FastaFormatInspector::inspect(const QString &buffer) const
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
                if ((x == s || *(x-1) == '\n') && *x == '>')
                    return eFastaType;
                else
                    return eUnknownFormatType;
            }
        }
    }

    return eUnknownFormatType;
}
