/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SUBSEQ_H
#define SUBSEQ_H

#include <QtCore/QByteArray>
#include <QtCore/QVector>

#include "BioString.h"
#include "Seq.h"
#include "UngappedSubseq.h"
#include "global.h"
#include "Entities/AbstractSeq.h"


/**
  * Subseqs are sequence substrings associated with a specific Seq.
  *
  * A Subseq is nothing more than a particular segment that may contain gaps and corresponds to a linear region of a
  * Seq. It is useful for manipulating sequences in a controlled fashion relative to its parent Seq.
  *
  * While Subseq is directly related to a parent Seq object, it may have a different length and contain gap characters.
  *
  * The actual Subseq sequence may be tweaked by adjusting its start and stop positions (relative to its parent
  * Seq object). A Subseq must have at least one non-gap character and have a length of at least 1 (if Seq is not
  * empty). It is important to note that extending the sequence will simply append ungapped character(s) immediately
  * previous (N-terminal) or subsequent (C-terminal) to the terminal-most, non-gap character:
  *
  * ---B-CD--- (add N-terminal 'A') ==> --AB-CD---
  * ---B-CD--- (add C-terminal 'E') ==> ---B-CDE--
  *
  * Similarly, shrinking the sequence will remove 1 or more non-gap characters from the approriate terminus, but neither
  * of these operations will remove the last non gap character remaining in the Subseq. By definition, a subseq must
  * contain at least on non-gap character.
  *
  * On the other hand, the trim and extend methods replace existing characters - gap columns will not be inserted to
  * accomodate a particular extension.
  *
  * Originally, moveStart() and moveStop() were included in this class; however, they simply translated their changes to
  * the startStart() and setStop() functions, which are now virtual. Thus, moveStart() and moveStop() have been removed
  * from this class. Nonetheless, it is useful to note how they function.
  *
  * -----------------------------
  * Method: moveStart(int dstart)
  *
  * Start_ will be moved as many dstart characters as is possible. The final sequence will be similar to that of setStart() and
  * setStop() in terms of how characters are added and removed. Obviously, if dstart equals zero, nothing will be changed.
  *
  * >>> Examples:
  *
  * 123456
  * ABCDEF
  *
  * -C---D-  (start = 3, stop = 4)
  * moveStart(-1) -> BC---D- (start = 2, stop = 4)
  * moveStart(-2..-N) -> ABC---D-
  * moveStart(1) -> -----D-
  * moveStart(2) -> ------E  (since start now exceeds stop, also update stop)
  * moveStart(3..N) -> -------F
  * moveStart(0) -> -C---D-
  *
  * ---------------------------
  * Method: moveStop(int dstop)
  *
  * Stop_ will be moved as many dstop characters as is possible. The final sequence will be similar to that of setStart() and
  * setStop() in terms of how characters are added and removed. Obviously, if dstop equals zero, nothing will be changed.
  *
  * >>> Examples:
  *
  * 123456
  * ABCDEF
  *
  * -C---D-  (start = 3, stop = 4)
  * moveStop(1) -> -C---DE (start = 3, stop = 5)
  * moveStop(2..N) -> -C---DEF
  * moveStop(-1) -> -C-----
  * moveStop(-2) -> B------  (since stop now exceeds start, also update start)
  * moveStop(-3..-N) -> A-------
  * moveStop(0) -> -C---D-
  */
class Subseq : public UngappedSubseq
{
public:
    // ------------------------------------------------------------------------------------------------
    // Forward declarations
    struct SimpleExtension;
    struct Trim;

    // ------------------------------------------------------------------------------------------------
    // Constructors
    Subseq(const Seq &parentSeq);                                               //!< Construct a Subseq from parentSeq


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const Subseq &other) const;                                 //!< Returns true if other is equivalent to this Subseq; false otherwise
    bool operator!=(const Subseq &other) const;                                 //!< Returns true if other is not identical to this Subseq; false otherwise

    bool operator<(const Subseq &other) const                                {   return UngappedSubseq::operator<(other); }


    // ------------------------------------------------------------------------------------------------
    // Re-exposed BioString operators
    using BioString::operator==;    // For some reason, the UngappedSubseq::operator== does not inherit
    using BioString::operator[];

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void extendLeft(const SimpleExtension &simpleExtension);                    //!< Extends the Subseq to the left using the data in simpleExtension
    void extendLeft(int position, const BioString &bioString);                  //!< Extends the Subseq to the left by replacing the characters beginning at position with bioString
    void extendLeft(int position, const ClosedIntRange &parentSeqRange);        //!< Extends the Subseq to the left by replacing the characters beginning at position with the characters specified by parentSeqRange
    void extendRight(const SimpleExtension &simpleExtension);                   //!< Extends the Subseq to the right using the data in simpleExtension
    void extendRight(int position, const BioString &bioString);                 //!< Extends the Subseq to the right by replacing the characters beginning at position with bioString
    void extendRight(int position, const ClosedIntRange &parentSeqRange);       //!< Extends the Subseq to the right by replacing the characters beginning at position with the characters specified by parentSeqRange
    ClosedIntRange leftTrimRange(int position) const;                           //!< Returns the ClosedIntRange that may be trimmed left of position (inclusive) or an empty ClosedIntRange if none may be trimmed
    int leftUnusedLength() const;                                               //!< Returns the number of characters in the parent Seq to the left of start (or start_ - 1)
    int mapToSeq(int position) const;                                           //!< Maps position in subseq space to its corresponding position in the parent Seq object; returns -1 if position corresponds to a gap character
    void rearrange(const ClosedIntRange &range, const BioString &bioString);    //!< A memory efficient version of replace that substitutes bioString for the characters in range. Requires that range and bioString have equivalent lengths and that the non-gap characters in range are equivalent in order and number to the non-gap characters in bioString
    // Override replace to ensure that the biostring remains a true substring of parentSeq_
    bool replace(int position, int amount, const BioString &bioString);         //!< Replace amount character starting from position (1-based) with bioString and return true if successful, false otherwise
    bool replace(const ClosedIntRange &range, const BioString &bioString);      //!< Replace the characters in range with bioString and return true if successful, false otherwise
    ClosedIntRange rightTrimRange(int position) const;                          //!< Returns the ClosedIntRange that may be trimmed right of position (inclusive) or an empty ClosedIntRange if none may be trimmed
    int rightUnusedLength() const;                                              //!< Returns the number of characters in the parent Seq to the right of stop (or parentSeq_.length() - stop_)
    bool setBioString(const BioString &bioString);                              //!< Sets the substring to bioString (which may contain gaps) if the ungapped bioString is a substring of parentSeq; returns whether this operation was successful
    bool setBioString(const ::QByteArray &byteArray);                           //!< Sets the substring to the characters in byteArray if the ungapped representation of byteArray is a substring of parentSeq; returns whether this operation was successful
    bool setBioString(const char *str);                                         //!< Sets the substring to the characters in str if the ungapped representation of str is a substring of parentSeq; returns whether this operation was successful
    void setStart(int newStart);                                                //!< Sets the start position to newStart
    void setStop(int newStop);                                                  //!< Sets the stop position to stop
    void trimLeft(const Trim &trim);                                            //!< Trims from the left using the data in trim
    void trimLeft(const ClosedIntRange &range, int nNonGaps = 0);               //!< Trims range from the left end of the subseq replacing the trimmed characters with gap characters; the nNonGaps is for optimization purposes and avoids the need to calculate the number of non-gaps that will be replaced. If nNonGaps is zero, then this value is auto-calculated
    void trimRight(const Trim &trim);                                           //!< Trims to the right using the data in trim
    void trimRight(const ClosedIntRange &range, int nNonGaps = 0);              //!< Trims range from the right end of the subseq replacing the trimmed characters with gap characters; the nNonGaps is for optimization purposes and avoids the need to calculate the number of non-gaps that will be replaced. If nNonGaps is zero, then this value is auto-calculated

    // ------------------------------------------------------------------------------------------------
    // Public static methods
    //! Returns a vector of AbstractSeq pointers associated with subseqs


    // ------------------------------------------------------------------------------------------------
    // Re-exposed public methods
    using BioString::collapseLeft;
    using BioString::collapseRight;
    using BioString::hasGaps;
    using BioString::hasNonGaps;
    using BioString::headGaps;
    using BioString::insertGaps;
    using BioString::leftSlidablePositions;
    using BioString::nonGapsBetween;
    using BioString::removeGaps;
    using BioString::rightSlidablePositions;
    using BioString::slide;
    using BioString::tailGaps;
    using BioString::translateGaps;
    using BioString::ungapped;
    using BioString::ungappedLength;


    // ------------------------------------------------------------------------------------------------
    // Public members
    boost::shared_ptr<AbstractSeq> seqEntity_;

    // ------------------------------------------------------------------------------------------------
    // Public structs - deprecated!
    struct SimpleExtension
    {
        ClosedIntRange seqRange_;
        int subseqPosition_;

        SimpleExtension() : subseqPosition_(0)
        {
        }
    };

    struct Trim
    {
        ClosedIntRange subseqRange_;
        int nNonGaps_;

        Trim() : nNonGaps_(0)
        {
        }
    };

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    static const ::QByteArray gapBuffer_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Inline methods
/**
  * @returns int
  */
inline
int Subseq::leftUnusedLength() const
{
    return start_ - 1;
}

/**
  * @returns int
  */
inline
int Subseq::rightUnusedLength() const
{
    return parentSeq_.length() - stop_;
}


template<typename T>
QVector<int> seqEntityIdVector(const QVector<Subseq *> subseqs)
{
    QVector<int> seqEntityIds(subseqs.size());
    for (int i=0, z=subseqs.size(); i<z; ++i)
        seqEntityIds[i] = subseqs.at(i)->seqEntity_->id();

    return seqEntityIds;
}


#endif // SUBSEQ_H


