/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IPARSER_H
#define IPARSER_H

#include <QtCore/QObject>

class QString;
class QTextStream;

class ParseResultPod;

class IParser : public QObject
{
    Q_OBJECT

public:
    virtual ~IParser() {}
    virtual ParseResultPod *parseFile(const QString &fileName) const = 0;
    virtual ParseResultPod *parseStream(const QTextStream &textStream, int totalBytes = 0) const = 0;

Q_SIGNALS:
    void progressChanged(int currentStep, int totalSteps) const;

protected:
    IParser(QObject *parent) : QObject(parent) {}
};

#endif // IPARSER_H
