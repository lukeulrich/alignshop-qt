/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTSEQUENCEPARSER_H
#define ABSTRACTSEQUENCEPARSER_H

#include "ISequenceParser.h"
#include "../PODs/SequenceParseResultPod.h"

class QObject;
class QString;

class AbstractSequenceParser : public ISequenceParser
{
public:
    virtual void cancel();

    virtual SequenceParseResultPod parseFile(const QString &fileName) const;
    virtual SequenceParseResultPod parseString(QString string) const;
    virtual SequenceParseResultPod parseStream(QTextStream &textStream, int totalBytes) const;

protected:
    AbstractSequenceParser(QObject *parent) : ISequenceParser(parent), canceled_(false) {}

    virtual QVector<SimpleSeqPod> parseSimpleSeqPods(QTextStream &textStream, int totalBytes) const = 0;

    bool canceled_;
};

#endif // ABSTRACTSEQUENCEPARSER_H
