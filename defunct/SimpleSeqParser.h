/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SIMPLESEQPARSER_H
#define SIMPLESEQPARSER_H

// Unable to forward declare QList. Compile error: template argument required for 'class QList'
#include <QtCore/QList>

// ------------------------------------------------------------------------------------------------
// Constants
const int PARSER_STREAMING_BUFFER_SIZE = 4096;

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QTextStream;

class ParseError;
class SimpleSeq;

/**
  * Abstract base class (ABC) defining the interface for parsing one or more SimpleSeqs from a given text stream.
  *
  * The interface for reading SimpleSeqs consists of one function (pure virtual): readAll(). Each data format to be read
  * must derive from this class and implement the readAll function.
  *
  * TODO: Because this class is an ABC, the virtual constructor functions, clone and create, must also be implemented in
  * derived classes.
  */
class SimpleSeqParser
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    SimpleSeqParser();                                          //!< Default constructor

    // Virtual constructor idoms:
//    virtual SimpleSeqParser *clone() const = 0;                 //!< Abstract virtual method for copy constructing
//    virtual SimpleSeqParser *create() const = 0;                //!< Abstract virtual method for the default constructor

    // ------------------------------------------------------------------------------------------------
    // Operators
    SimpleSeqParser &operator=(const SimpleSeqParser &other);   //!< Assigns other to the current object; Currently meaningless as there is really nothing to assign

    // ------------------------------------------------------------------------------------------------
    // Destructors
    virtual ~SimpleSeqParser();                                 //!< Destructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Abstract virtual method for reading all sequences from source, storing them in listSimpleSeq. Error is stored in genError if provided. Returns true on success or false on failure.
    virtual bool readAll(QTextStream &source, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError) = 0;

private:
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
inline
SimpleSeqParser::SimpleSeqParser()
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Assignment operators
/**
  * @param other [const SimpleSeqParser &]
  */
inline
SimpleSeqParser &SimpleSeqParser::operator=(const SimpleSeqParser & /* other */)
{
    // Do nothing because there are no data members or other state variables to manage
    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructors
inline
SimpleSeqParser::~SimpleSeqParser()
{
}

#endif // SIMPLESEQPARSER_H
