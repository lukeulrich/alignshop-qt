/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include "AbstractSequenceParser.h"
#include "../macros.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void AbstractSequenceParser::cancel()
{
    canceled_ = true;
}

/**
  * @param fileName [const QString &]
  * @returns SequenceParseResultPod
  */
SequenceParseResultPod AbstractSequenceParser::parseFile(const QString &fileName) const
{
    ASSERT(fileName.isEmpty() == false);
    QFileInfo fileInfo(fileName);
    ASSERT(fileInfo.isDir() == false);

    if (fileInfo.exists() == false)
        throw QString("File, %1, does not exist").arg(fileName);

    if (fileInfo.size() == 0)
        throw QString("Empty file");

    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream inputStream(&file);
    return parseStream(inputStream, file.size());
}

/**
  * Cannot use a const QString as argument. Will have to depend upon pure COW for optimization
  *
  * @param string [const QString &]
  * @returns SequenceParseResultPod
  */
SequenceParseResultPod AbstractSequenceParser::parseString(QString string) const
{
    QTextStream inputStream(&string, QIODevice::ReadOnly);
    return parseStream(inputStream, string.length());
}

/**
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns SequenceParseResultPod
  */
SequenceParseResultPod AbstractSequenceParser::parseStream(QTextStream &textStream, int totalBytes) const
{
    ASSERT(textStream.status() == QTextStream::Ok);

    return SequenceParseResultPod(parseSimpleSeqPods(textStream, totalBytes));
}
