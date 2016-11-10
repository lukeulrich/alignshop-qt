/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "ClustalParser.h"

// ------------------------------------------------------------------------------------------------
// Supporting includes
#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include "ParseError.h"
#include "SimpleSeq.h"

/**
  * Implemented is a somewhat flexible set of parsing rules for reading Clustal-formatted sequences. See the general
  * class description for the specific constraints and rules that are applied when reading Clustal data.
  *
  * Should an error be encountered during the parsing process, the function will return false. If parseError is a valid
  * object, it will be set with an appropriate ParseError. Currently, only the message is reported - the column and line numbers should all be -1.
  *
  * @param stream [QTextStream &]
  * @param listSimpleSeq [QList<SimpleSeq>]
  * @param parseError [ParseError &]
  * @return bool
  */
bool ClustalParser::readAll(QTextStream &stream, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError)
{
    Q_ASSERT_X(PARSER_STREAMING_BUFFER_SIZE > 0, "ClustalParser::readAll", "PARSER_STREAMING_BUFFER_SIZE must be > 0");
    Q_ASSERT_X(stream.status() == QTextStream::Ok, "ClustalParser::readAll", "stream status is not QTextStream::Ok");

    if (stream.status() != QTextStream::Ok)
    {
        parseError.set("Input stream status is not Ok");
        return false;
    }

    listSimpleSeq.clear();

    // Ignore all empty/blank header whitespace
    stream.skipWhiteSpace();

    // Read until we find the CLUSTAL header line
    QString line = stream.readLine();
    if (line.isNull())
    {
        parseError.set("empty file");
        return false;
    }

    // Header line must begin with CLUSTAL
    if (line.mid(0, 7) != "CLUSTAL")
    {
        parseError.set("missing or invalid CLUSTAL header line");
        return false;
    }

    // Next line must be empty
    line = stream.readLine();
    if (!line.isEmpty())
    {
        parseError.set("blank line must immediately follow the CLUSTAL header line");
        return false;
    }

    QHash<QString, bool> identifier_hash;

    QStringList identifiers;
    QStringList alignments;

    QRegExp alignment_regex("^(\\S+)\\s+(\\S.*)");
    QRegExp consensus_regex("^\\s+[.:*](?:\\s|[.:*])+$");

    while (!stream.atEnd())
    {
        if (line.simplified().trimmed().isEmpty() ||
            consensus_regex.indexIn(line) != -1)
        {
            line = stream.readLine();
            continue;
        }

        QStringList block_identifiers;
        QStringList block_alignments;

        bool end_of_block = false;

        // Attempt to read a block of sequences
        while (!end_of_block)
        {
            int pos = alignment_regex.indexIn(line);
            if (pos == -1)
            {
                // This line contains some non-Empty text. Either:
                // o Consensus line OR
                // o Junk line OR
                // o Malformed data line

                if (consensus_regex.indexIn(line) == -1)
                {
                    parseError.set("malformed alignment line");
                    return false;
                }
            }
            else
            {
                block_identifiers.append(alignment_regex.cap(1));
                QString alignment = alignment_regex.cap(2);
                int digits = 0;
                int leadingSpace = 0;
                int spaces = 0;

                // Remove any terminal numbers that are preceded by a space
                for (int z=alignment.length()-1, i=z; i>= 0; --i)
                {
                    QChar ch = alignment.at(i);
                    if (ch.isDigit())
                        ++digits;
                    else if (ch.isSpace())
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
                if (block_alignments.count() &&
                    alignment.length() != block_alignments.at(0).length())
                {
                    parseError.set("alignments within block do not all have the same length");
                    return false;
                }
                block_alignments.append(alignment);
            }

            line = stream.readLine();
            if (line.simplified().trimmed().isEmpty())
                end_of_block = true;
        }

        Q_ASSERT_X(block_identifiers.count() == block_alignments.count(), "ClustalParser::readAll", "block identifier count != block alignment count");

        // Process this block
        if (identifiers.count())
        {
            if (block_identifiers.count() != identifiers.count())
            {
                parseError.set("unequal number of sequences between blocks");
                return false;
            }

            // Make sure we have the same sequences in this block as in the previous blocks
            for (int i=0, z=identifiers.count(); i<z; ++i)
            {
                if (!identifier_hash.contains(block_identifiers.at(i)))
                {
                    parseError.set("found sequence identifiers in current block that are distinct from previous block(s)");
                    return false;
                }

                if (identifiers.at(i) != block_identifiers.at(i))
                {
                    parseError.set("sequence identifiers ordered differently from previous blocks");
                    return false;
                }

                alignments[i].append(block_alignments.at(i));
            }
        }
        else
        {
            identifiers = block_identifiers;
            alignments = block_alignments;

            for (int i=0, z=identifiers.count(); i<z; ++i)
                identifier_hash.insert(identifiers.at(i), true);
        }

        line = stream.readLine();
    }

    for (int i=0, z=identifiers.count(); i<z; ++i)
        listSimpleSeq.append(SimpleSeq(identifiers.at(i), alignments.at(i)));

    if (listSimpleSeq.count() == 0)
    {
        parseError.set("no sequences found");
        return false;
    }
    else if (listSimpleSeq.count() == 1)
    {
        parseError.set("alignment must have more than one sequence");
        listSimpleSeq.clear();

        return false;
    }

    return true;
}
