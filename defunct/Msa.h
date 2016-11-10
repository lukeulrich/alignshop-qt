/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MSA_H
#define MSA_H

#include <QtCore/QObject>

#include "BioString.h"
#include "util/MsaRect.h"
#include "Subseq.h"
#include "global.h"             // For ASSERT

// ------------------------------------------------------------------------------------------------
// Forward declarations
class SubseqLessThan;
class SubseqGreaterThan;

/**
  * Msa is the base class used to model multiple sequence alignments.
  *
  * A Msa consists of an array of aligned Subseqs. Pointers to member Subseqs are stored in a QList and many of the
  * methods for manipulating QList's have been provided here for manipulating the list of Subseqs (e.g. moving, inserting,
  * removing, etc) with respect to maintaining Msa integrity. The set of model operations thus boils down to major
  * categories:
  * 1) Managing the list of Subseqs
  * 2) Alignment specific operations, which primarily encompasses 3 functionalities:
  *    -> inserting gap columns
  *    -> removing gap columns
  *    -> horizontally sliding sequence data
  *    -> TODO: trimming alignment boundaries
  *
  * All subseq members must possess the same type/alphabet which is ultimately determined by their underlying AnonSeqs.
  * Any subseqs that have a different alphabet than that defined by the MSA will be rejected.
  *
  * Msa strictly models the underlying data necessary to adequately represent a multiple sequence alignment. To adhere
  * to a MVC-style approach, no visualization or view parameters are part of this class. Moreover, derived data such as
  * the predicted secondary structure and/or domains, etc. are managed in other classes. The same applies to all annotation
  * data.
  *
  * Another major difference apart from the QList, is that both rows and columns are accessed using a 1-based indices,
  * rather than the standard 0-based. This is to make it easier when dealing with the 1-based nature of sequence data.
  *
  * WARNING: Msa takes ownership and manages all member Subseq instances. Thus, it is vital that no member Subseq pointer
  * is externally deleted and/or stored permanently as this will likely result in program termination.
  *
  * Msa emits a variety of signals pertaining to modifications to its structure. Interested views or objects can attach
  * to these and respond appropriately to changes in the underlying Msa data.
  *
  * Potential OPTIMIZATIONS:
  * - Directly access underlying subseq/biostring data of member sequences. For instance in the removeGapColumns method,
  *   at least four function calls must be made to access the character at a specific row and column. Multiply this times
  *   the number of rows and columns in a big alignment and it could take a very long time.
  *   Possible solution: friend class
  *
  * TODO:
  * >> Enable alignment border modifications which map to Subseq->setStart/setStop; also need to set signals for these
  *    modifications.
  */
class Msa : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //!< Construct an empty Msa with alphabet and parent
    Msa(Alphabet alphabet = eUnknownAlphabet, int id = 0, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Destructor
    ~Msa();                                                     //!< Trivial destructor

    // ------------------------------------------------------------------------------------------------
    // Operators
    const Subseq *operator()(int i) const;                      //!< Return a const reference to the Subseq at index i (1-based)

    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                                  //!< Returns the alphabet of this Msa
    bool append(Subseq *subseq);                                //!< Add subseq to the Msa and return whether the addition was successful
    const Subseq *at(int i) const;                              //!< Return a const reference to the Subseq at index i (1-based)
    void clear();                                               //!< Remove all subseqs from the Msa
    void collapseLeft(const MsaRect &msaRect);                  //!< Collapses all characters in msaRect to the left
    void collapseRight(const MsaRect &msaRect);                 //!< Collapses all characters in msaRect to the right
    const QString &description() const;                         //!< Return the description of this Msa
    void extendSubseqsLeft(int top, int bottom, int msaColumn); //!< Extend the start positions of the subseqs between top and bottom to msaColumn as possible
    void extendSubseqsRight(int top, int bottom, int msaColumn);//!< Extend the stop positions of the subseqs between top and bottom to msaColumn as possible
    int id() const;                                             //!< Returns the Msa id
    bool insert(int i, Subseq *subseq);                         //!< Insert subseq at row index i (1-based), and return whether the addition was successful
    //! insert count columns of gaps before column using the gapCharacter symbol
    void insertGapColumns(int column, int count, char gapCharacter = constants::kDefaultGapCharacter);
    bool isEmpty() const;                                       //!< Returns true if there are no sequences in the Msa; otherwise returns false
    int length() const;                                         //!< Return the number of columns in the alignment
    void levelSubseqsLeft(int top, int bottom, int msaColumn);  //!< Level the start positions of the subseqs between top and bottom to msaColumn as possible
    void levelSubseqsRight(int top, int bottom, int msaColumn); //!< Level the stop positions of the subseqs between top and bottom to msaColumn as possible
    QList<const Subseq *> members() const;                      //!< Returns the list of Subseq pointers comprising this Msa
    void moveRow(int from, int to);                             //!< Moves the subseq at index position, from, to index position, to
    int moveRowRelative(int from, int delta);                   //!< Moves the Subseq at index up to delta positions and returns the direction (negative indicates upwards, positive indicates downwards) and number of positions successfully moved
    void moveRowRange(int start_index, int stop_index, int to); //!< Moves a group of Subseqs between start_index and stop_index to the index position to
    //! Moves a group of Subseqs between start_index and stop_index up to delta positions and returns the number of positions successfully moved
    int moveRowRangeRelative(int start_index, int stop_index, int delta);
    const QString &name() const;                                //!< Return the Msa name
    int negativeColIndex(int i) const;                          //!< Return the negative index of i relative to the number of columns (e.g. length of alignemnt); inverse of positiveColIndex
    int negativeRowIndex(int i) const;                          //!< Return the negative index of i relative to the number of rows (e.g. sequences); inverse of postiveRowIndex
    int positiveColIndex(int i) const;                          //!< Return the absolute positive index of i relative to the number of columns (e.g. length of alignemnt)
    int positiveRowIndex(int i) const;                          //!< Return the absolute positive index of i relative to the number of rows (e.g. sequences)
    bool prepend(Subseq *subseq);                               //!< Add subseq at the beginning of the Msa and return whether the addition was successful
    void removeAt(int i);                                       //!< Remove the Subseq at index i
    void removeFirst();                                         //!< Remove the first Subseq from the Msa
    int removeGapColumns();                                     //!< Remove any columns which consist entirely of gap characters and return the number of columns removed
    void removeLast();                                          //!< Remove the last Subseq from the Msa
    void setDescription(const QString &description);            //!< Set the Msa description
    void setName(const QString &name);                          //!< Set the Msa name
    void setSubseqStart(int subseqIndex, int newStart);         //!< Sets the start position of the subseq at subseqIndex to newStart
    void setSubseqStop(int subseqIndex, int newStop);           //!< Sets the stop position of the subseq at subseqIndex to newStop
    //! Horizontally slide the rectangle of characters specified by left/top, right/bottom, delta positions and return the direction (negative to the left, positive to the right) and number of positions successfully moved
    int slideRegion(int left, int top, int right, int bottom, int delta);
    void sort(const SubseqGreaterThan &greaterThan);            //!< Sorts the member subseqs using greaterThan; emits subseqOrderAboutToBeChanged before sorting and subseqOrderChanged after sorting is complete
    void sort(const SubseqLessThan &lessThan);                  //!< Sorts the member subseqs using lessThan; emits subseqOrderAboutToBeChanged before sorting and subseqOrderChanged after sorting is complete
    int subseqCount() const;                                    //!< Return the number of sequences
    QList<int> subseqIds() const;                               //!< Returns the list of all subseq ids
    void swap(int i, int j);                                    //!< Exchange the Subseq at index i with the one at index j
    void trimSubseqsLeft(int top, int bottom, int msaColumn);   //!< Maximally trim the start positions of the subseqs between top and bottom to msaColumn as possible
    void trimSubseqsRight(int top, int bottom, int msaColumn);  //!< Maximally trim the stop positions of the subseqs between top and bottom to msaColumn as possible

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals - all coordinate data is expressed as positive coordinates
    void extendOrTrimFinished(int start, int end);              //!< Emitted when an extend or trim operation has completed and one or more columns were updated; start and end (1-based) indicate those columns that were affected
    void gapColumnsAboutToBeInserted(int column, int count);    //!< Emitted just before count gap columns are inserted at column (positive)
    void gapColumnsAboutToBeRemoved(int count);                 //!< Emitted just before all (count) gap columns are removed
    void gapColumnsInserted(int column, int count);             //!< Emitted after count gap columns have been inserted at column (positive)
    void gapColumnsRemoved(int count);                          //!< Emitted after all (count) gap columns have been removed
    void msaReset();                                            //!< Emitted when all subseqs have been removed from the Msa via the clear method
    //!< Emitted when the region originally located between (top, left) and (right, bottom) is horizontally moved delta positions and is currently located at finalLeft and finalRight; all coordinates are normalized (ie. left = min(left, right), top = min(top, bottom), etc.)
    void regionSlid(int left, int top, int right, int bottom, int delta, int finalLeft, int finalRight);
    void subseqAboutToBeSwapped(int first, int second);         //!< Emitted just before two subseqs are swapped. The subseq at index first will be swapped with the subseq at index second
    //!< Emitted when the subseq at subseqIndex has been extended extension characters beginning at column
    void subseqExtended(int subseqIndex, int column, const QString &extension);
    //!< Emitted when the subseq at subseqIndex has a new start position, newStart. oldStart is the previous start position
    void subseqStartChanged(int subseqIndex, int newStart, int oldStart);
    //!< Emitted when the subseq at subseqIndex has a new stop position, newStop. oldStop is the previous stop position
    void subseqStopChanged(int subseqIndex, int newStop, int oldStop);
    void subseqSwapped(int first, int second);                  //!< Emitted after two subseqs have been swapped. The subseq originally at index first is now at index second and vice versa
    //!< Emitted when the subseq at subseqIndex has trimmings beginning at column replaced with gap characters
    void subseqTrimmed(int subseqIndex, int column, const QString &trimmings);
    void subseqsAboutToBeInserted(int start, int end);          //!< Emitted just before subseqs are added to the Msa. New subseqs will be placed between start and end inclusive
    //!< Emitted just before subseqs are moved within the Msa. The subseqs between start and end inclusive will be placed at target
    void subseqsAboutToBeMoved(int start, int end, int target);
    void subseqsAboutToBeRemoved(int start, int end);           //!< Emitted just before subseqs are removed from the Msa. The subseqs to be removed are between start and end inclusive
    void subseqsAboutToBeSorted();                              //!< Emitted just before the subseqs are sorted
    void subseqsInserted(int start, int end);                   //!< Emitted after subseqs have been added to the Msa. New subseqs are located between start and end inclusive
    //!< Emitted after subseqs have been moved within the Msa. The subseqs between start and end inclusive have been placed at target
    void subseqsMoved(int start, int end, int target);
    void subseqsRemoved(int start, int end);                    //!< Emitted after subseqs have been removed from the Msa. Removed subseqs were located between start and end inclusive
    void subseqsSorted();                                       //!< Emitted after the subseqs have been sorted

    //!< Emitted when the subseq at subseqIndex has been changed; column indicates the leftmost column of the newSubSequence which replaced oldSubsequence. Currently only emitted in reference to collapse commands
    void subseqInternallyChanged(int subseqIndex, int column, const QString &newSubSequence, const QString &oldSubSequence);
    //!< Emitted when the characters in msaRect have been collapsed to the left; rightMostModifiedColumn indicates the rightmost column in msaRect that was modified
    void collapsedLeft(const MsaRect &msaRect, int rightMostModifiedColumn);
    //!< Emitted when the characters in msaRect have been collapsed to the right; leftMostModifiedColumn indicates the leftmost column in msaRect that was modified
    void collapsedRight(const MsaRect &msaRect, int leftMostModifiedColumn);

    // Obsolete
//    Emitted in conjunction with changes to the start or stop coordinates of the subseq at subseqIndex. The msa region changed begins at column (1-based), newSubSequence contains the final alignment and oldSubSequence contains the previous alignment
//    void subseqBorderChanged(int subseqIndex, int column, const QString &newSubSequence, const QString &oldSubSequence);


protected:
    Alphabet alphabet_;             //!< The alphabet of this Msa and its subseqs
    QString description_;           //!< Short description
    int id_;                        //!< Identifier for this msa
    QString name_;                  //!< Simple descriptive name
    QList<Subseq *> subseqs_;       //!< List of subseqs that comprise the alignment

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //!< Actual method for extending the start positions of the subseqs between top and bottom to msaColumn as possible; return the range of columns affected
    QPair<int, int> __extendSubseqsLeft(int top, int bottom, int msaColumn);
    //!< Actual method for extending the stop positions of the subseqs between top and bottom to msaColumn as possible; return the range of columns affected
    QPair<int, int> __extendSubseqsRight(int top, int bottom, int msaColumn);
    //!< Actual method for maximally trimming the start positions of the subseqs between top and bottom to msaColumn as possible; return the range of columns affected
    QPair<int, int> __trimSubseqsLeft(int top, int bottom, int msaColumn);
    //!< Actual method for maximally trimming the stop positions of the subseqs between top and bottom to msaColumn as possible; return the range of columns affected
    QPair<int, int> __trimSubseqsRight(int top, int bottom, int msaColumn);
    //!< Actual method for setting the start position of the subseq at subseqIndex to newStart; returns the range of columns affected
    QPair<int, int> __setSubseqStart(int subseqIndex, int newStart);
    //!< Actual method for setting the stop position of the subseq at subseqIndex to newStop; returns the range of columns affected
    QPair<int, int> __setSubseqStop(int subseqIndex, int newStop);

#ifdef TESTING
    friend class TestMsa;
#endif
};

Q_DECLARE_METATYPE(Msa *)

/**
  * SubseqLessThan provides a minimalistic functor implementation for comparing two Subseq pointers
  * in a "less than" fashion and serves as the sorting interface used by Msa::Sort().
  *
  * Subclass and override the operator() method to provide specialized sorting. For an example, see
  * MsaSubseqModel.
  */
class SubseqLessThan
{
public:
    //! Returns true if the id of a is less than the id of b; false otherwise
    virtual bool operator()(const Subseq *a, const Subseq *b) const
    {
        ASSERT(a);
        ASSERT(b);
        return a->id() < b->id();
    }
};

/**
  * SubseqGreaterThan provides a minimalistic functor implementation for comparing two Subseq pointers
  * in a "greater than" fashion and serves as the sorting interface used by Msa::Sort().
  *
  * Subclass and override the operator() method to provide specialized sorting. For an example, see
  * MsaSubseqModel.
  */
class SubseqGreaterThan
{
public:
    //! Returns true if the id of a is less than the id of b; false otherwise
    virtual bool operator()(const Subseq *a, const Subseq *b) const
    {
        ASSERT(a);
        ASSERT(b);
        return a->id() > b->id();
    }
};

#endif // MSA_H
