/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef FASTAPARSER_H
#define FASTAPARSER_H

#include "SimpleSeqParser.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations (redundant for clarity)
class QTextStream;

class ParseError;

/**
  * Parses FASTA-formatted sequence data from a given text stream.
  *
  * Adheres to the very simplistic interface defined in the abstract base class (ABC), SimpleSeqParser.
  *
  * FASTA sequence files are parsed using the following approach:
  * o Ignore all text data until the first line that begins with a > symbol
  * o All text following the > symbol until the newline character is considered free-form header text
  * o All character data until the next line that begins with a > symbol (or the end of file) is considered to be
  *   sequence data.
  * o The terminal newline of the header of sequence is not considered part of the sequence.
  *
  * TODO: Some form of error processing and logging. Various cases might include:
  * o No sequences in file
  * o Extra data before first sequence
  * o Sequences without header information
  * o Sequences with header but no sequence data
  * o Would say extra looong sequences, but DNA contigs or genomes could very easily be quite long. So this is not a
  *   good error condition to process
  * o Out of memory...
  * o Possibly look for potentially erroneous > symbols (e.g. [newline][whitespace]>) and warn about this
  *
  * o Any actual issues with the sequence itself should be caught during the conversion to {Bio,Amino,Dna,Rna}String's
  */
class FastaParser : public SimpleSeqParser
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    FastaParser();                                      //!< Default constuctor
    FastaParser(const FastaParser &other);              //!< Construct a copy of other

    // Virtual constructor idoms:
//    virtual FastaParser *clone() const;
//    virtual FastaParser *create() const;

    // ------------------------------------------------------------------------------------------------
    // Operators
    FastaParser &operator=(const FastaParser &other);   //!< Assign other to this object

    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~FastaParser();                             //!< Destructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Process stream for all FASTA-formatted sequences and return them as a list of SimpleSeqs
    virtual bool readAll(QTextStream &stream, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError);

private:
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
inline
FastaParser::FastaParser()
{
}

/**
  * @param other [const FastaParser &]
  */
inline
FastaParser::FastaParser(const FastaParser & /* other */)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
inline
FastaParser &FastaParser::operator=(const FastaParser & /* other */)
{
    // Do nothing because there are no data members or other state variables to manage
    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
inline
FastaParser::~FastaParser()
{

}


#endif // FASTAPARSER_H
