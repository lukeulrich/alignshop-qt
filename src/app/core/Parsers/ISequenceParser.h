/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ISEQUENCEPARSER_H
#define ISEQUENCEPARSER_H

#include <QtCore/QObject>

#include "../PODs/SequenceParseResultPod.h"

class QTextStream;

/**
  * A parser's responsibility is two-fold:
  * 1) Actually attempt to parse a data file
  * 2) Inspect a chunk of data for compatibility purposes
  */
class ISequenceParser : public QObject
{
    Q_OBJECT

public:
    virtual ~ISequenceParser() {}
    virtual ISequenceParser *clone() const = 0;

    // Cancel is only relevant in a multi-threaded context
    virtual void cancel() = 0;

    virtual bool isCompatibleString(const QString &chunk) const = 0;

    virtual SequenceParseResultPod parseFile(const QString &fileName) const = 0;
    virtual SequenceParseResultPod parseString(QString string) const = 0;
    virtual SequenceParseResultPod parseStream(QTextStream &textStream, int totalBytes) const = 0;

Q_SIGNALS:
    void progressChanged(int currentStep, int totalSteps) const;

protected:
    ISequenceParser(QObject *parent) : QObject(parent) {}
};

#endif // ISEQUENCEPARSER_H
