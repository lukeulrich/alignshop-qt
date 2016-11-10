/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FASTAPARSER_H
#define FASTAPARSER_H

#include <QtCore/QVector>

#include "AbstractSequenceParser.h"

/**
  * Potential issues:
  * o Invalid file structure
  * o Leading non-whitespace data before the first character
  * o Solely header and no sequence data
  * o Empty header
  * o Invalid sequence data (handled externally)
  *
  * Empty headers and/or sequences are not handled here
  */
class FastaParser : public AbstractSequenceParser
{
    Q_OBJECT

public:
    FastaParser(QObject *parent = 0);

    FastaParser *clone() const;

    bool isCompatibleString(const QString &chunk) const;

protected:
    QVector<SimpleSeqPod> parseSimpleSeqPods(QTextStream &textStream, int totalBytes) const;
};

#endif // FASTAPARSER_H
