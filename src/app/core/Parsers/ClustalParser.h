/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALPARSER_H
#define CLUSTALPARSER_H

#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "AbstractSequenceParser.h"

class ClustalParser : public AbstractSequenceParser
{
    Q_OBJECT

public:
    ClustalParser(QObject *parent = 0);

    ClustalParser *clone() const;

    bool isCompatibleString(const QString &chunk) const;

    virtual SequenceParseResultPod parseStream(QTextStream &textStream, int totalBytes) const;

protected:
    QVector<SimpleSeqPod> parseSimpleSeqPods(QTextStream &textStream, int totalBytes) const;

private:
    bool isConsensusLine(const QString &line) const;
    QPair<QString, QString> parseAlignmentLine(const QString &line) const;
    QVector<SimpleSeqPod> parseSimpleSeqPodsNoRegex(QTextStream &textStream, int totalBytes) const;

    // Deprecated
    QVector<SimpleSeqPod> parseSimpleSeqPodsRegex(QTextStream &textStream, int totalBytes) const;

#ifdef TESTING
    friend class TestClustalParser;
#endif
};


#endif // CLUSTALPARSER_H
