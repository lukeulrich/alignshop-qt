/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SIGNALSEQUENCEPARSER_H
#define SIGNALSEQUENCEPARSER_H

#include "ISequenceParser.h"
#include "../global.h"

class SignalSequenceParser : public QObject
{
    Q_OBJECT

public:
    // Takes ownership of sequenceParser
    SignalSequenceParser(ISequenceParser *sequenceParser, QObject *parent = nullptr);

public Q_SLOTS:
    void cancel();
    void parseFile(const QString &fileName);

Q_SIGNALS:
    void parseCanceled();
    void parseError(QString);
    void parseOver();                                               // Emitted at the end of all processing regardless of the path
    void parseSuccess(SequenceParseResultPod);
    void progressChanged(int currentStep, int totalSteps) const;

private:
    ISequenceParser *sequenceParser_;
};

#endif // SIGNALSEQUENCEPARSER_H
