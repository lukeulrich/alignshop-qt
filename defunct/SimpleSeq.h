/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SIMPLESEQ_H
#define SIMPLESEQ_H

#include <QtCore/QString>

/**
  * A SimpleSeq (or "simple" sequence) is simply an unvalidated, data structure containing a raw sequence, any header
  * or labeling information, and any arbitrary attributes.
  *
  * SimpleSeq really serves as an intermediate class for reading user sequence data before transforming it into
  * BioStrings/AnonSeq/Seq/Subseqs. No cleansing of any sort is performed and the sequence is taken directly as provided.
  * Additionally, no constraints are put on either the header or the sequence.
  *
  * Assignment and manipulation of attributes has not yet been implemented; however, some marshaling mechanism will need
  * to be defined before this can be implemented.
  */
class SimpleSeq
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    SimpleSeq(QString header = "", QString sequence = "");  //!< Construct a SimpleSeq from header and sequence
    SimpleSeq(const SimpleSeq &other);                      //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Operators
    SimpleSeq &operator=(const SimpleSeq &other);   //!< Assign other to this SimpleSeq object

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QString header() const;                         //!< Return the header
    void setHeader(const QString header);           //!< Set header_ to header
    void setSequence(const QString sequence);       //!< Set sequence_ to sequence
    QString sequence() const;                       //!< Return the sequence

private:
    QString header_;                                //!< Free-form text comprising the sequence header
    QString sequence_;                              //!< Free-form text representing the actual sequence data
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
inline
SimpleSeq::SimpleSeq(QString header, QString sequence) : header_(header), sequence_(sequence)
{
}

inline
SimpleSeq::SimpleSeq(const SimpleSeq &other) : header_(other.header_), sequence_(other.sequence_)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * Assigns other to the current object
  * @param other reference to SimpleSeq to be assigned
  */
inline
SimpleSeq &SimpleSeq::operator=(const SimpleSeq &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    header_ = other.header_;
    sequence_ = other.sequence_;

    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @return QString
  */
inline
QString SimpleSeq::header() const
{
    return header_;
}

/**
  * @param header [QString]
  */
inline
void SimpleSeq::setHeader(QString header)
{
    header_ = header;
}

/**
  * @param sequence [QString]
  */
inline
void SimpleSeq::setSequence(QString sequence)
{
    sequence_ = sequence;
}

/**
  * @return QString
  */
inline
QString SimpleSeq::sequence() const
{
    return sequence_;
}


#endif // SIMPLESEQ_H
