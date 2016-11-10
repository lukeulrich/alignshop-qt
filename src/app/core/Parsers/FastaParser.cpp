/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QScopedPointer>
#include <QtCore/QTextStream>

#include "FastaParser.h"
#include "../PODs/SequenceParseResultPod.h"
#include "../BioString.h"
#include "../constants.h"
#include "../global.h"
#include "../macros.h"
#include "../misc.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
FastaParser::FastaParser(QObject *parent) : AbstractSequenceParser(parent)
{
}

/**
  * @returns FastaParser *
  */
FastaParser *FastaParser::clone() const
{
    return new FastaParser();
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Rules used to determine if buffer is in the Fasta format:
  * 1. All leading empty lines are ignored
  * 2. First non-whitespace character should be a greater than symbol '>' and be either the first character or the
  *    the first character after a newline.
  *
  * @param chunk [const QString &]
  * @returns bool
  */
bool FastaParser::isCompatibleString(const QString &chunk) const
{
    const QChar *x = chunk.constData();
    const QChar *first = x;
    while (!x->isNull())
    {
        if (!x->isSpace())
        {
            if (*x == '>' && (x == first || *(x-1) == '\n'))
                return true;

            return false;
        }

        ++x;
    }

    return false;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param textStream [QTextStream &]
  * @param totalBytes [int]
  * @returns QVector<SimpleSeqPod>
  */
QVector<SimpleSeqPod> FastaParser::parseSimpleSeqPods(QTextStream &textStream, int totalBytes) const
{
    QVector<SimpleSeqPod> simpleSeqPods;

    // ---------------------------------------------------------------------
    // A. Read in a block of text and make sure we have at least one character
    QString buffer = textStream.read(constants::kParserStreamingBufferSize);
    if (buffer.isNull() || buffer.isEmpty())  // Empty file
        return simpleSeqPods;

    int totalBytesRead = buffer.length();

    // ---------------------------------------------------------------------
    // B. Remove all leading whitespace data until the first >
    while (buffer.at(0) != '>')
    {
        // Search for the "\n>" indicating the first sequence
        // Additionally check that no non-whitespace characters precede these two characters
        // Check for invalid non-whitespace data that precedes "\n>"
        const QChar *x = buffer.constBegin();
        for (int i=0, z= buffer.length()-1; i< z; ++i, ++x)
        {
            if (*x == '\n' && *(x+1) == '>')
            {
                buffer = buffer.mid(i+1);
                goto AFTER_WHILE;           // To break out of nested loop - not worth refactoring at this point :)
            }
            else if (x->isSpace() == false)
            {
                throw "First non-whitespace character must be the > symbol";
            }
        }

        // It is possible that the \n and > are split between reads, thus keep the last character in case it immediately
        // precedes the greater than symbol. The loop should continue
        if (buffer.at(buffer.length()-1) == '\n')
            buffer = "\n";
        else
            // Bogus character to prevent seg faulting on our while loop condition
            buffer = "X";

        QString block = textStream.read(constants::kParserStreamingBufferSize);
        if (!block.isNull())
        {
            buffer += block;
            totalBytesRead += block.length();
        }
        else
        {
            // Read to end of stream and did not find a \n>. File purely contains whitespace data
            return simpleSeqPods;
        }
    }

    // This label is provided for exiting the nested loop above
    AFTER_WHILE:

    emit progressChanged(totalBytesRead, totalBytes);

    // ---------------------------------------------------------------------
    // C. Read stream chunk by chunk and parse all encoded sequences
    ASSERT(buffer.isEmpty() == false);
    ASSERT_X(buffer.at(0) == '>', "First character in buffer should begin with >");
    QString block;
    do
    {
        // Optimization: A chunk of text is fetched each iteration from the input stream. Thus, it is quite possible
        //               that more than one sequence is present within this block. When searching for the FASTA record
        //               separator \n>, we remember the last found position with the variable 'spos' and begin searching
        //               from there again.
        int spos = 1;
        int a = buffer.indexOf("\n>", spos);
        while (a != -1)
        {
            QString header;
            int header_to = buffer.indexOf('\n', spos);
            if (header_to != 1)
                header = buffer.mid(spos, header_to - spos);

            BioString sequence;
            if (a != header_to)
                sequence = buffer.mid(header_to + 1, a - header_to - 1).toAscii();

            simpleSeqPods << SimpleSeqPod(header.trimmed(), sequence);

            // Spos points to the beginning of the \n> search sequence. Thus, we set the next search to begin
            // two characters beyond this point.
            spos = a + 2;

            // Search for the next sequence
            a = buffer.indexOf("\n>", spos);
        }

        // Remove processed sequences from buffer
        buffer = buffer.mid(spos - 1);

        block = textStream.read(constants::kParserStreamingBufferSize);
        buffer += block;
        totalBytesRead += block.length();
        emit progressChanged(totalBytesRead, totalBytes);

        // TODO: Implement a cancel exception
        // The following is only relevant for multi-threaded applications, and is set in AbstractSequenceParser
        if (canceled_)
            throw "[FastaParser] Cancelled! Please implement a proper cancel exception";
    } while (block.isNull() == false);

    // ---------------------------------------------------------------------
    // D. The last sequence in the stream will simply end with a newline or EOF. Since step C only finds those
    //    sequences which have a terminal \n> string, it will not parse the last sequence. We deal with that case
    //    here.
    QString header;
    BioString sequence;
    int header_to = buffer.indexOf('\n');
    if (header_to != -1)    // A newline (and consequently header text is present) was found within the buffer
    {
        header = buffer.mid(1, header_to - 1);

        if (header_to != buffer.length() - 1)
            sequence = buffer.mid(header_to + 1).toAscii();
    }
    else if (buffer.length() > 1)
        // There is no newline in the buffer. Thus, this record consists solely of a header
        header = buffer.mid(1);

    simpleSeqPods << SimpleSeqPod(header.trimmed(), sequence);

    return simpleSeqPods;
}
