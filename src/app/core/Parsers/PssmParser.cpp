/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFileInfo>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "PssmParser.h"
#include "../constants.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param fileName [const QString &]
  * @returns Pssm
  */
Pssm PssmParser::parseFile(const QString &fileName) const
{
    if (fileName.isEmpty())
        throw QString("Unable to parse empty fileName");

    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir())
        throw QString("Filename, %1, is a directory").arg(fileName);

    if (fileInfo.exists() == false)
        throw QString("Pssm file does not exist");

    if (fileInfo.size() == 0)
        throw QString("Empty pssm file");

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw file.errorString();

    QTextStream inputStream(&file);

    return parseStream(inputStream);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param stream [const QTextStream &]
  * @returns Pssm
  */
Pssm PssmParser::parseStream(QTextStream &stream) const
{
    stream.skipWhiteSpace();

    // Empty file
    if (stream.atEnd())
        throw QString("Empty pssm file");

    // First line should begin with "# AG-PSSM"
    QString line = stream.readLine();
    if (line.startsWith("# AG-PSSM") == false)
        throw QString("Missing AG-PSSM header line");

    // Next line should consist of the PSI_SCALE_FACTOR
    QString chunk = stream.readLine(19);
    if (chunk != "# PSI_SCALE_FACTOR:")
        throw QString("Missing PSI_SCALE_FACTOR line");

    Pssm pssm;
    stream >> pssm.positScaleFactor_;
    if (pssm.positScaleFactor_ <= 0)
        throw QString("Invalid PSI_SCALE_FACTOR (must be > 0)");

    // Skip all blank lines
    line = stream.readLine();
    while (line.isEmpty())
        line = stream.readLine();

    // Extract the amino acid character mapping
    pssm.mapping_ = splitPssmTableHeader(line);
    if (pssm.mapping_.size() != constants::kPssmWidth)
        throw QString("Invalid PSSM table header");

    line = stream.readLine();
    if (line.isEmpty())
        throw QString("No PSSM rows found");

    // Read pssm rows
    int last_position = 0;
    bool ok = false;
    while (line.isEmpty() == false)
    {
        QStringList words = splitByWhitespace(line);
        if (words.size() != constants::kPssmWidth + 2)
            throw QString("Invalid PSSM score line");

        // ---------------------------------
        // The first word should be an integer indicating the row position
        // We do not actually use the position, but use it to check for succesful parsing
        int position = words.first().toInt(&ok);
        if (!ok || position != last_position + 1)
            throw QString("Invalid PSSM position");
        last_position = position;

        // ---------------------------------
        // words.at(1) --> character corresponding to this row position; ignored

        // ---------------------------------
        // words.at(2.. 2 + kPssmWidth - 1): individual scores
        pssm.rows_ << PssmRow();
        PssmRow &pssmRow = pssm.rows_.last();
        for (int i=2, z = 2 + constants::kPssmWidth; i<z; ++i)
        {
            int score = words.at(i).toInt(&ok);
            if (!ok)
                throw QString("Invalid PSSM score");

            pssmRow.scores_[i-2] = score;
        }

        line = stream.readLine();
    }

    return pssm;
}

/**
  * Throws a QString error if any duplicate score column label is found.
  *
  * @param thLine [const QString &]
  * @returns QVector<char>
  */
QVector<char> PssmParser::splitPssmTableHeader(const QString &thLine) const
{
    ASSERT(thLine.isEmpty() == false);

    // Check that there are no duplicate header chars
    QSet<char> uniqueChars;
    QVector<char> headerChars;
    const QChar *x = thLine.constData();
    while (!x->isNull())
    {
        if (x->isSpace() == false)
        {
            if (uniqueChars.contains(x->toAscii()))
                throw QString("Duplicate residue/nucleotide column labels for %1").arg(*x);

            headerChars << x->toAscii();
            uniqueChars << x->toAscii();
        }

        ++x;
    }

    return headerChars;
}

/**
  * While it is possible to use the QString split methods, the regular expression version would have to be used, which
  * is significantly slower than simply a character-by-character based approach implemented here.
  *
  * @param string [const QString &]
  * @returns QStringList
  */
QStringList PssmParser::splitByWhitespace(const QString &string) const
{
    QStringList words;

    const QChar *x = string.constData();
    bool in_word = false;
    QPair<int, int> bounds;
    for (int i=0; x->isNull() == false; ++i, ++x)
    {
        if (!x->isSpace())
        {
            if (in_word)
            {
                bounds.second = i;
            }
            else
            {
                bounds.first = i;
                bounds.second = i;
                in_word = true;
            }
        }
        else
        {
            if (in_word)
            {
                // End of our first word
                words << string.mid(bounds.first, bounds.second - bounds.first + 1);

                in_word = false;
            }
        }
    }

    // Capture any terminal word
    if (in_word)
        words << string.mid(bounds.first);

    return words;
}
