/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "ClustalParser.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
ClustalParser::ClustalParser(QObject *parent) : AbstractSequenceParser(parent)
{
}

/**
  */
ClustalParser *ClustalParser::clone() const
{
    return new ClustalParser();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Rules used to determine if buffer is in the Clustal format:
  * 1. All leading empty lines are ignored
  * 2. First non-whitespace characters should be CLUSTAL and be either the first few characters or the
  *    the appear just after a newline.
  *
  * @param chunk [const QString &]
  * @returns bool
  */
bool ClustalParser::isCompatibleString(const QString &chunk) const
{
    const QChar *s = chunk.constData();
    const QChar *e = s + chunk.length();

    const QChar *x = s - 1;
    while (x != e)
    {
        ++x;
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
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

/**
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns SequenceParseResultPod
  */
SequenceParseResultPod ClustalParser::parseStream(QTextStream &textStream, int totalBytes) const
{
    SequenceParseResultPod clustalPod = AbstractSequenceParser::parseStream(textStream, totalBytes);
    clustalPod.isAlignment_ = eTrue;

    return clustalPod;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns QVector<SimpleSeqPod>
  */
QVector<SimpleSeqPod> ClustalParser::parseSimpleSeqPods(QTextStream &textStream, int totalBytes) const
{
    // The non-regex version benchmarked at roughly 2.5x faster than the regex version. See tests.
    return parseSimpleSeqPodsNoRegex(textStream, totalBytes);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Equivalent to the following regular expression: ^\\s+[.:*](?:\\s|[.:*])+$
  *
  * @param line [const QString &]
  * @returns bool
  */
bool ClustalParser::isConsensusLine(const QString &line) const
{
    const QChar *x = line.constData();

    // Line is empty
    if (x->isNull())
        return false;

    // Does the line begin with at least one space
    if (!x->isSpace())
        return false;

    // Skip the first character since we just checked it
    ++x;

    // Walk through each character and check that it is allowed
    bool hasConsensusChar = false;
    for (; !x->isNull(); ++x)
    {
        if (x->isSpace())
            continue;

        if (*x == '.' || *x == ':' || *x == '*')
        {
            hasConsensusChar = true;
            continue;
        }

        // All other characters indicate this is not a consensus line
        return false;
    }

    return hasConsensusChar;
}

/**
  * Equivalent to the regular expression: ^(\\S+)\\s+(\\S.*)
  *
  * If a match was not found, the first QString in the pair will be empty.
  *
  * @param line [const QString &]
  * @returns QPair<QString, QString>
  */
QPair<QString, QString> ClustalParser::parseAlignmentLine(const QString &line) const
{
    QPair<QString, QString> captures;

    if (line.isEmpty())
        return captures;

    // Must begin with a non-space character
    if (line.at(0).isSpace())
        return captures;

    // Parse out the identifier
    const QChar *x = line.constData() + 1;  // + 1 to skip the character we already checked
    while(!x->isSpace() && !x->isNull())
        ++x;

    captures.first = line.mid(0, x - line.constData());

    // Skip all following whitespace
    while (x->isSpace() && !x->isNull())
        ++x;

    // If at the end of the line, no alignment section is present, return null
    if (x->isNull())
        return QPair<QString, QString>();

    captures.second = line.mid(x - line.constData());

    return captures;
}

/**
  * Does not use a regular expression for parsing.
  *
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns QVector<SimpleSeqPod>
  */
QVector<SimpleSeqPod> ClustalParser::parseSimpleSeqPodsNoRegex(QTextStream &textStream, int totalBytes) const
{
    int totalBytesRead = 0;
    int initialReadPosition = textStream.pos();

    // Ignore all empty/blank header whitespace
    textStream.skipWhiteSpace();

    // Empty file
    if (textStream.atEnd())
        throw "empty file";

    // Read until we find the CLUSTAL header line
    QString line = textStream.readLine();
    ASSERT(line.isNull() == false);

    // Header line must begin with CLUSTAL
    if (line.startsWith("CLUSTAL", Qt::CaseInsensitive) == false)
        throw "missing or invalid CLUSTAL header line";

    line = textStream.readLine();
    if (line.isEmpty() == false)
        throw "blank line must immediately follow the CLUSTAL header line";

    totalBytesRead = textStream.pos() - initialReadPosition;

    QHash<QString, bool> identifier_hash;

    QStringList identifiers;
    QStringList alignments;

    while (!textStream.atEnd())
    {
        if (line.trimmed().isEmpty() ||
            isConsensusLine(line))
        {
            line = textStream.readLine();
            totalBytesRead += line.length();
            continue;
        }

        QStringList blockIdentifiers;
        QStringList blockAlignments;

        bool endOfBlock = false;

        // Attempt to read a block of sequences
        while (!endOfBlock)
        {
            QPair<QString, QString> captures = parseAlignmentLine(line);
            bool isAlignmentLine = !captures.first.isEmpty();

            if (!isAlignmentLine)
            {
                // This line contains some non-Empty text. Either:
                // o Consensus line OR
                // o Junk line OR
                // o Malformed data line
                if (!isConsensusLine(line))
                    throw "malformed alignment line";
            }
            else
            {
                blockIdentifiers.append(captures.first);
                QString alignment = captures.second;
                int digits = 0;
                int leadingSpace = 0;
                int spaces = 0;

                // Remove any terminal numbers that are preceded by a space
                const QChar *x = alignment.constData() + alignment.length() - 1;
                for (int z=alignment.length()-1, i=z; i>= 0; --i, --x)
                {
                    if (x->isDigit())
                        ++digits;
                    else if (x->isSpace())
                    {
                        ++spaces;
                        if (digits)
                        {
                            leadingSpace = 1;
                            break;
                        }
                    }
                }

                if (leadingSpace)
                    alignment.chop(digits + spaces);

                alignment = alignment.simplified().remove(' ');
                if (blockAlignments.count() && alignment.length() != blockAlignments.at(0).length())
                    throw "alignments within block do not all have the same length";
                blockAlignments << alignment;
            }

            line = textStream.readLine();
            totalBytesRead += line.length();
            if (line.trimmed().isEmpty())
                endOfBlock = true;

            emit progressChanged(totalBytesRead, totalBytes);
        }

        ASSERT_X(blockIdentifiers.count() == blockAlignments.count(), "block identifier count != block alignment count");

        // Process this block
        if (identifiers.count())
        {
            if (blockIdentifiers.count() != identifiers.count())
                throw "unequal number of sequences between blocks";

            // Make sure we have the same sequences in this block as in the previous blocks
            for (int i=0, z=identifiers.count(); i<z; ++i)
            {
                if (!identifier_hash.contains(blockIdentifiers.at(i)))
                    throw "found sequence identifiers in current block that are distinct from previous block(s)";

                if (identifiers.at(i) != blockIdentifiers.at(i))
                    throw "sequence identifiers ordered differently from previous blocks";

                alignments[i].append(blockAlignments.at(i));
            }
        }
        else
        {
            identifiers = blockIdentifiers;
            alignments = blockAlignments;

            for (int i=0, z=identifiers.count(); i<z; ++i)
                identifier_hash.insert(identifiers.at(i), true);
        }

        line = textStream.readLine();
        totalBytesRead += line.length();
        emit progressChanged(totalBytesRead, totalBytes);

        // TODO: Implement a cancel exception
        // The following is only relevant for multi-threaded applications, and is set in AbstractSequenceParser
        if (canceled_)
            throw "[ClustalParser] Cancelled! Please implement a proper cancel exception";
    }

    emit progressChanged(totalBytes, totalBytes);

    QVector<SimpleSeqPod> simpleSeqPods;
    simpleSeqPods.reserve(identifiers.count());
    for (int i=0, z=identifiers.count(); i<z; ++i)
        simpleSeqPods << SimpleSeqPod(identifiers.at(i), alignments.at(i).toAscii());

    if (simpleSeqPods.isEmpty())
        throw "no sequences found";
    else if (simpleSeqPods.count() == 1)
        throw "alignment must have more than one sequence";

    return simpleSeqPods;
}
















// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Deprecated
/**
  * Uses a regular expression for parsing.
  *
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns QVector<SimpleSeqPod>
  */
QVector<SimpleSeqPod> ClustalParser::parseSimpleSeqPodsRegex(QTextStream &textStream, int totalBytes) const
{
    int totalBytesRead = 0;
    int initialReadPosition = textStream.pos();

    // Ignore all empty/blank header whitespace
    textStream.skipWhiteSpace();

    // Empty file
    if (textStream.atEnd())
        throw "empty file";

    // Read until we find the CLUSTAL header line
    QString line = textStream.readLine();
    ASSERT(line.isNull() == false);

    // Header line must begin with CLUSTAL
    if (line.startsWith("CLUSTAL", Qt::CaseInsensitive) == false)
        throw "missing or invalid CLUSTAL header line";

    line = textStream.readLine();
    if (line.isEmpty() == false)
        throw "blank line must immediately follow the CLUSTAL header line";

    totalBytesRead = textStream.pos() - initialReadPosition;

    QHash<QString, bool> identifier_hash;

    QStringList identifiers;
    QStringList alignments;

    QRegExp alignmentRegex("^(\\S+)\\s+(\\S.*)");
    QRegExp consensusRegex("^\\s+[.:*](?:\\s|[.:*])+$");

    while (!textStream.atEnd())
    {
        if (line.trimmed().isEmpty() ||
            consensusRegex.indexIn(line) != -1)
        {
            line = textStream.readLine();
            totalBytesRead += line.length();
            continue;
        }

        QStringList blockIdentifiers;
        QStringList blockAlignments;

        bool endOfBlock = false;

        // Attempt to read a block of sequences
        while (!endOfBlock)
        {
            if (alignmentRegex.indexIn(line) == -1)
            {
                // This line contains some non-Empty text. Either:
                // o Consensus line OR
                // o Junk line OR
                // o Malformed data line
                if (consensusRegex.indexIn(line) == -1)
                    throw "malformed alignment line";
            }
            else
            {
                blockIdentifiers.append(alignmentRegex.cap(1));
                QString alignment = alignmentRegex.cap(2);
                int digits = 0;
                int leadingSpace = 0;
                int spaces = 0;

                // Remove any terminal numbers that are preceded by a space
                const QChar *x = alignment.constData() + alignment.length() - 1;
                for (int z=alignment.length()-1, i=z; i>= 0; --i, --x)
                {
                    if (x->isDigit())
                        ++digits;
                    else if (x->isSpace())
                    {
                        ++spaces;
                        if (digits)
                        {
                            leadingSpace = 1;
                            break;
                        }
                    }
                }

                if (leadingSpace)
                    alignment.chop(digits + spaces);

                alignment = alignment.simplified().remove(' ');
                if (blockAlignments.count() && alignment.length() != blockAlignments.at(0).length())
                    throw "alignments within block do not all have the same length";
                blockAlignments << alignment;
            }

            line = textStream.readLine();
            totalBytesRead += line.length();
            if (line.trimmed().isEmpty())
                endOfBlock = true;

            emit progressChanged(totalBytesRead, totalBytes);
        }

        ASSERT_X(blockIdentifiers.count() == blockAlignments.count(), "block identifier count != block alignment count");

        // Process this block
        if (identifiers.count())
        {
            if (blockIdentifiers.count() != identifiers.count())
                throw "unequal number of sequences between blocks";

            // Make sure we have the same sequences in this block as in the previous blocks
            for (int i=0, z=identifiers.count(); i<z; ++i)
            {
                if (!identifier_hash.contains(blockIdentifiers.at(i)))
                    throw "found sequence identifiers in current block that are distinct from previous block(s)";

                if (identifiers.at(i) != blockIdentifiers.at(i))
                    throw "sequence identifiers ordered differently from previous blocks";

                alignments[i].append(blockAlignments.at(i));
            }
        }
        else
        {
            identifiers = blockIdentifiers;
            alignments = blockAlignments;

            for (int i=0, z=identifiers.count(); i<z; ++i)
                identifier_hash.insert(identifiers.at(i), true);
        }

        line = textStream.readLine();
        totalBytesRead += line.length();
        emit progressChanged(totalBytesRead, totalBytes);
    }

    emit progressChanged(totalBytes, totalBytes);

    QVector<SimpleSeqPod> simpleSeqPods;
    simpleSeqPods.reserve(identifiers.count());
    for (int i=0, z=identifiers.count(); i<z; ++i)
        simpleSeqPods << SimpleSeqPod(identifiers.at(i), alignments.at(i).toAscii());

    if (simpleSeqPods.isEmpty())
        throw "no sequences found";
    else if (simpleSeqPods.count() == 1)
        throw "alignment must have more than one sequence";

    return simpleSeqPods;
}
