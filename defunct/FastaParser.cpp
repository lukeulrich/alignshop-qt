/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "FastaParser.h"

// ------------------------------------------------------------------------------------------------
// Supporting includes
#include <QTextStream>

#include "ParseError.h"
#include "SimpleSeq.h"

/**
  * Reads all FASTA-formatted sequences from the current position in stream and returns a list of SimpleSeqs. Given the
  * diverse ways and arbitrary definition sequence data has been represented in the FASTA format, the following rules
  * are applied when reading this form of data:
  *
  * o Any data before the first > symbol is ignored
  * o A new SimpleSeq entry is created for every newline that begins with a > symbol regardless of whether it has any
  *   header text or sequence characters
  * o No whitespace is permitted between the newline character and the > symbol
  * o The header text consists of all character data immediately following the > symbol and up to but not including the
  *   next newline character (or end of file)
  * o The sequence consists of all character data immediately following the newline character of the header line and the
  *   start of the next record (or end of file).
  *
  * If the stream status is not ok, a parser error will be set.
  *
  * Will clear listSimpleSeq before adding any new sequences from the stream.
  *
  * @param stream [const QTextStream &]
  * @param listSimpleSeq [QList<SimpleSeq>]
  * @param parseError [ParseError &]
  * @return bool
  */
bool FastaParser::readAll(QTextStream &stream, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError)
{
    Q_ASSERT_X(PARSER_STREAMING_BUFFER_SIZE > 0, "FastaParser::readAll", "PARSER_STREAMING_BUFFER_SIZE must be > 0");
    Q_ASSERT_X(stream.status() == QTextStream::Ok, "FastaParser::readAll", "stream status is not QTextStream::Ok");

    if (stream.status() != QTextStream::Ok)
    {
        parseError.set("Input stream status is not Ok");
        return false;
    }

    listSimpleSeq.clear();

    // ---------------------------------------------------------------------
    // A. Read in a block of text and make sure we have at least one character
    QString buffer = stream.read(PARSER_STREAMING_BUFFER_SIZE);
    if (buffer.isNull() || buffer.isEmpty())  // Empty file
        return true;

    // ---------------------------------------------------------------------
    // B. Remove all leading data until the first >
    while (buffer.at(0) != '>')
    {
        int x = buffer.indexOf("\n>");
        if (x != -1)
        {
            // This operation will terminate the while loop
            buffer = buffer.mid(x+1);
            continue;
        }
        else if (buffer.right(1) == QChar('\n'))
            // It is possible that the \n and > are split between reads, thus keep the last character in case it immediately
            // precedes the greater than symbol. The loop should continue
            buffer = '\n';
        else
            // Bogus character to prevent seg faulting on our while loop condition
            buffer = 'X';

        QString block = stream.read(PARSER_STREAMING_BUFFER_SIZE);
        if (!block.isNull())
            buffer += block;
        else
            // Read to end of stream and did not find a \n>
            return true;
    }

    // ---------------------------------------------------------------------
    // C. Read stream chunk by chunk and parse all encoded sequences
    Q_ASSERT_X(buffer.at(0) == '>', "FastaParser::readAll", "First character in buffer should begin with >");
    QString block;
    do
    {
        // Optimization: A chunk of text up to READ_BUFFER_SIZE is fetched each iteration from the input stream. Thus,
        //               it is quite possible that more than one sequence is present within this block. Thus, when
        //               searching for the FASTA record separator \n>, we remember the last found position with the
        //               variable spos and begin searching from there again.
        int spos = 1;
        int a = buffer.indexOf("\n>", spos);
        while (a != -1)
        {
            QString header;
            int header_to = buffer.indexOf('\n', spos);
            if (header_to != 1)
                header = buffer.mid(spos, header_to - spos);

            QString sequence;
            if (a != header_to)
                sequence = buffer.mid(header_to+1, a - header_to - 1);

            listSimpleSeq.append(SimpleSeq(header, sequence));

            // Spos points to the beginning of the \n> search sequence. Thus, we set the next search to begin
            // two characters beyond this point.
            spos = a+2;

            // Search for the next sequence
            a = buffer.indexOf("\n>", spos);
        }

        // Remove processed sequences from buffer
        buffer = buffer.mid(spos-1);

        block = stream.read(PARSER_STREAMING_BUFFER_SIZE);
        buffer += block;
    } while (!block.isNull());

    // ---------------------------------------------------------------------
    // D. The last sequence in the stream will simply end with a newline or EOF. Since step C only finds those
    //    sequences which have a terminal \n> string, it will not parse the last sequence. We deal with that case
    //    here.
    QString header;
    QString sequence;
    int header_to = buffer.indexOf('\n');
    if (header_to != -1)    // A newline (and consequently header text is present) was found within the buffer
    {
        header = buffer.mid(1, header_to - 1);

        if (header_to != buffer.length() - 1)
        {
            sequence = buffer.mid(header_to+1);

            // Per the spec, the terminal-most newline character in the sequence will not be preserved (similar to that
            // for the header). Thus, we simply check if the last character is a newline and remove it if present.
            if (sequence.right(1) == QChar('\n'))
                sequence.chop(1);
        }
    }
    else if (buffer.length() > 1)
        // There is no newline in the buffer. Thus, this record consists solely of a header
        header = buffer.mid(1);

    listSimpleSeq.append(SimpleSeq(header, sequence));

    return true;
}
