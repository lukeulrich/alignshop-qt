/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SUBSEQ_H
#define SUBSEQ_H

#include <QtCore/QHash>
#include <QtCore/QScopedPointer>
#include <QtCore/QVariant>

#include "AnonSeq.h"
#include "BioString.h"

/**
  * Subseqs are user-level, alignment friendly sequence substrings associated with a specific AnonSeq and abstractly
  * through a "foreign key" to Seqs.
  *
  * Ultimately all sequences will be managed as Subseq's. A Subseq is nothing more than a particular segment
  * that may contain gaps and corresponds to a linear region of a AnonSeq. It represents a detailed level for
  * manipulating sequences and also may be arbitrarily decorated with properties.
  *
  * Subseq has its own copy of a BioString because while the ungapped sequence data is equivalent to its source AnonSeq,
  * it may have a different length and contain gap characters. An integral identifier to its parent Seq object
  * provides a means for accessing Seq-level properties. Similarly, it contains an AnonSeq copy for accessing
  * its parent sequence data.
  *
  * WARNING: It is essential that this object be deleted before deleting the associated Seq object otherwise,
  *          methods that depend upon the Seq object will cause a segfault.
  *
  * The actual Subseq sequence may be tweaked by adjusting its start and stop positions (relative to its parent
  * AnonSeq object). A Subseq must have at least one non-gap character and have a length of at least 1. It is
  * important to note that extending the sequence will simply append ungapped character(s) immediately previous
  * (N-terminal) or subsequent (C-terminal) to the terminal-most, non-gap character:
  *
  * ---B-CD--- (add N-terminal 'A') ==> --AB-CD---
  * ---B-CD--- (add C-terminal 'E') ==> ---B-CDE--
  *
  * Similarly, shrinking the sequence will remove 1 or more non-gap characters from the approriate terminus.
  */
class Subseq
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    Subseq(const AnonSeq &anonSeq, int id = 0);     //!< Construct a Subseq from anonSeq (covers entire AnonSeq) with id
    Subseq(const Subseq &other);                    //!< Construct a copy of Subseq from other

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    Subseq &operator=(const Subseq &other);         //!< Assigns other to this Subseq
    const QChar operator[](int i) const;            //!< Return the const character at the specified position, i (1-based)

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const AnonSeq &anonSeq() const;                 //!< Return a const reference to the source anonSeq
    const QChar at(int i) const;                    //!< Return the const character at the specified position, i (1-based); same as operator[]
    const BioString &bioString() const;             //!< Return the raw sequence
    //!< Collapses all characters between start and stop (inclusive, 1-based) to the left and returns the range of columns changed or 0, 0 if none were changed
    QPair<int, int> collapseLeft(int start, int stop);
    //!< Collapses all characters between start and stop (inclusive, 1-based) to the right and returns the range of columns changed or 0, 0 if none were changed
    QPair<int, int> collapseRight(int start, int stop);
    int headGaps() const;                           //!< Return the number of gaps before the first non-gap character
    int id() const;                                 //!< Return id
    //! Insert n gaps at RelAbs position using gapCharacter and return a reference to this object
    Subseq &insertGaps(int position, int n, char gapCharacter = constants::kDefaultGapCharacter);
    const QString &label() const;                   //!< Return the current label
    int moveStart(int dstart);                      //!< Moves start_ dstart characters and returns the number of characters the start position was successfully moved
    int moveStop(int dstop);                        //!< Moves stop_ dstop characters and returns the number of characters the stop position was successfully moved
    Subseq &removeGaps(int position, int n);        //!< Remove up to n contiguous gaps if the character at position is a gap
    bool setBioString(const BioString &bioString);  //!< Sets bioString_ to bioString (which may contain gaps) if the ungapped bioString is a substring of its source AnonSeq
    void setLabel(const QString &label);            //!< Sets the current label
    bool setStart(int newStart);                    //!< Sets the start position to start
    bool setStop(int newStop);                      //!< Sets the stop position to stop

    //!< Slide the characters between start_pos and stop_pos up to delta positions and return the number of positions successfully moved
    int slideSegment(int start_pos, int stop_pos, int delta);
    int start() const;                              //!< Returns the start position (1-based)
    int stop() const;                               //!< Returns the stop position (1-based)
    int tailGaps() const;                           //!< Return the number of gaps occurring after the last non-gap character
    int ungappedLength() const;                     //!< Return the length of the subseq if all gaps were removed. This is equivalent to stop_ - start_ + 1

    // ------------------------------------------------------------------------------------------------
    // Public properties
    bool modified_;                                 //!< Has this Subseq been modified; mostly useful for external purposes

private:
    AnonSeq anonSeq_;                               //!< Source AnonSeq object
    int id_;                                        //!< Identifier of this subseq

    QScopedPointer<BioString> bioString_;           //!< BioString pointer that reflects the actual subsequence
    QString label_;                                 //!< User-assigned display label
    int start_;                                     //!< Start position (1-based) relative to full Seq sequence
    int stop_;                                      //!< Stop position (1-based) relative to full Seq sequence
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @return const BioString &
  */
inline
const BioString &Subseq::bioString() const
{
    return *bioString_;
}

/**
  * @return const QString &
  */
inline
const QString &Subseq::label() const
{
    return label_;
}


/**
  * @param label [const QString &]
  */
inline
void Subseq::setLabel(const QString &label)
{
    label_ = label;
}

/**
  * Simply pass this reqeuest onto the BioString implementation for the underlying BioString object.
  *
  * @param start_pos [int]
  * @param stop_pos [int]
  * @param delta [int]
  * @return int
  * @see BioString::slideSegment()
  */
inline
int Subseq::slideSegment(int start_pos, int stop_pos, int delta)
{
    Q_ASSERT_X(bioString_->positiveIndex(start_pos) >= 1, "Subseq::slideSegment", "start_pos must be >= 1");
    Q_ASSERT_X(bioString_->positiveIndex(start_pos) <= bioString_->length(), "Subseq::slideSegment", "start_pos must be <= length()");

    Q_ASSERT_X(bioString_->positiveIndex(stop_pos) >= 1, "Subseq::slideSegment", "stop_pos must be >= 1");
    Q_ASSERT_X(bioString_->positiveIndex(stop_pos) <= bioString_->length(), "Subseq::slideSegment", "stop_pos must be <= length()");

    Q_ASSERT_X(bioString_->positiveIndex(start_pos) <= bioString_->positiveIndex(stop_pos), "Subseq::slideSegment", "start_pos must be <= stop_pos");

    return bioString_->slideSegment(start_pos, stop_pos, delta);
}

/**
  * @return int
  */
inline
int Subseq::start() const
{
    return start_;
}

/**
  * @return int
  */
inline
int Subseq::stop() const
{
    return stop_;
}

#endif // SUBSEQ_H
