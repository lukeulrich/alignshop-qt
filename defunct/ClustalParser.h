/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALPARSER_H
#define CLUSTALPARSER_H

#include "SimpleSeqParser.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations (redundant for clarity)
class QTextStream;

class ParseError;

/**
  * Parses Clustal-formatted sequence data from a given text stream.
  *
  * Adheres to the very simplistic interface defined in the abstract base class (ABC), SimpleSeqParser.
  *
  * Clustal sequence files are parsed using the following approach:
  * o Ignore all text up to the line that begins with CLUSTAL
  * o Ignore one or more empty lines
  * o One or more blocks of sequences
  * o Each block optionally may have a consensus line (which is ignored)
  * o Each line in a block consists of sequence identifier followed by at least one space, followed by an equal number
  *   of sequence characters to all other lines in this block. Optionally terminated by total number of characters.
  *
  * Constraints:
  * - Must have CLUSTAL header line at top of file. Any amount of whitespace may appear before this line, but nothing else
  * - After the CLUSTAL header line, there must be at least blank line
  * - Each block must have the same number of sequences/lines
  * - Each block must have the same identifiers for each sequence in the same order
  * - Each alignment in the block must be at least one character and all share the same length
  * - Each block is separated from all other blocks with at least one empty line
  * - Sequence identifiers may not contain spaces
  *
  * Optional:
  * - Arbitrary amount of whitespace between the sequence identifier and the sequence data
  *   REMOVED: , but the actual starting point for the alignment must be the same within a given block
  * - Duplicate identifiers are allowed because a sequence is tied to its identifier and position within each block.
  *   It is not immediately clear why duplicate sequence identifiers would be helpful.
  * - Sequence data may contain spaces
  * - Any numbers terminal to alignment may be present, but there must be at least one space between the alignment end
  *   and the number. Terminal-most numbers will be ignored during parsing.
  * - Consensus lines have leading whitespace and consist solely of spaces and either . or : or *. Regardless of their
  *   placement these are all ignored.
  *
  * Types of errors:
  * 1. "empty file"
  * 2. "missing or invalid CLUSTAL header line"
  * 3. "blank line must immediately follow the CLUSTAL header line"
  * 4. "no sequences found" - has clustal header, but no sequences in the file
  * 5. "alignment must have more than one sequence"
  * 6. "unequal number of sequences between blocks"
  * 7. "found sequence identifiers in current block that are distinct from previous block(s)"
  * 8. "sequence identifiers ordered differently from previous blocks"
  * 9. "alignments within block do not all have the same length"
  * 10. REMOVED: "inconsistent spacing between sequence identifier and start of alignment"
  * 11. "malformed alignment line"
  *
  * If an error is encountered while parsing the file, a ParserExeption will be thrown with one  of the above messages
  *
  * TODO: Add in line numbers, line, and column when throwing ParseException's
  */
class ClustalParser : public SimpleSeqParser
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    ClustalParser();                                        //!< Default constuctor
    ClustalParser(const ClustalParser &other);              //!< Construct a copy of other

    // Virtual constructor idoms:
//    virtual ClustalParser *clone() const;
//    virtual ClustalParser *create() const;

    // ------------------------------------------------------------------------------------------------
    // Operators
    ClustalParser &operator=(const ClustalParser &other);   //!< Assign other to this object

    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~ClustalParser();                               //!< Destructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Process stream for all Clustal-formatted sequences and return them as a list of SimpleSeqs
    virtual bool readAll(QTextStream &stream, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError);

private:
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
inline
ClustalParser::ClustalParser()
{
}

/**
  * @param other [const ClustalParser &]
  */
inline
ClustalParser::ClustalParser(const ClustalParser & /* other */)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
inline
ClustalParser &ClustalParser::operator=(const ClustalParser & /* other */)
{
    // Do nothing because there are no data members or other state variables to manage
    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
inline
ClustalParser::~ClustalParser()
{

}

#endif // CLUSTALPARSER_H
