/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSA_H
#define MSA_H

#include "util/ClosedIntRange.h"
#include "util/PosiRect.h"
#include "Subseq.h"
#include "constants.h"
#include "enums.h"
#include "PODs/SubseqChangePod.h"
#include "Entities/AbstractSeq.h"


class ISubseqLessThan
{
public:
    virtual bool lessThan(const Subseq *a, const Subseq *b) const = 0;
};

/**
  * Msa models multiple sequence alignments.
  *
  * A Msa consists of an array of aligned Subseqs. Pointers to member Subseqs are stored in a QVector and many of the
  * methods for manipulating QVector's have been provided here for manipulating the array of Subseqs (e.g. moving,
  * inserting, removing, etc) with respect to maintaining Msa integrity. The set of model operations thus boils down to
  * major categories:
  * 1) Managing the list of Subseqs
  * 2) Alignment specific operations:
  *    o inserting and removing gap columns
  *    o horizontally sliding character data
  *    o collapsing sequence data
  *    o extending subseqs within the constraints of their parent sequences
  *    o trimming subseqs within the constraints of their parent sequences
  *
  * Previously, the extend and trim methods all called setSubseqStart or setSubseqStop to update the subseqs. These
  * methods in turn, would call the appropriate subseq method and originally insertedGapColumns to accommodate the
  * change if necessary. Now, the extend and trim methods modify the member subseqs directly. SetSubseqStart and
  * setSubseqStop no longer support setting positions beyond their cognate positions (ie. setting a start value greater
  * than its stop value). Rather, they must have their changes fit within the confines of the current Msa boundaries.
  * No gap columns will be automatically inserted. This must be done separately if needed to accommodate additional
  * characters.
  *
  * All subseq members must possess the same grammar as the Msa. Any subseqs with a different grammar that defined by
  * the MSA will be rejected.
  *
  * Msa strictly models the underlying data necessary to adequately represent a multiple sequence alignment. To adhere
  * to a MVC-style approach, no visualization or view parameters are part of this class. Moreover, derived data such as
  * the predicted secondary structure and/or domains, etc. are managed in other classes. The same applies to all annotation
  * data.
  *
  * Another major difference apart from the QVector, is that both rows and columns are accessed using a 1-based indices,
  * rather than the standard 0-based. This is to make it easier when dealing with the 1-based nature of sequence data.
  *
  * NOTE: Msa takes ownership and manages all member Subseq instances. Thus, it is vital that no member Subseq pointer
  * is externally deleted, modified, and/or stored permanently as this will likely result in program termination.
  *
  * Msa is not a QObject class and thus does not emit any signals nor possess any slots. For these capabilities, use
  * ObservableMsa which inherits from Msa and wraps the relevant methods to provide an observable infrastructure.
  */
class Msa
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    Msa(Grammar grammar = eUnknownGrammar);                                     //!< Construct an empty Msa with grammar
    ~Msa();                                                                     //!< Releases all memory used by these subseqs


    // ------------------------------------------------------------------------------------------------
    // Operators
    const Subseq *operator[](int i) const;                                      //!< Return a const reference to the Subseq at index i (1-based)


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool append(Subseq *subseq);                                                //!< Add subseq to the Msa and return true if subseq was successfully appended; false otherwise
    const Subseq *at(int i) const;                                              //!< Return a const reference to the Subseq at index i (1-based)
    bool canCollapseLeft(const PosiRect &msaRect) const;                        //!< Returns true if the characters in msaRect can be collapsed to the left by at least one character; false otherwise
    bool canCollapseRight(const PosiRect &msaRect) const;                       //!< Returns true if the characters in msaRect can be collapsed to the right by at least one character; false otherwise
    bool canExtendLeft(int msaColumn, const ClosedIntRange &rows) const;        //!< Returns true if any row between rows can be extended left to msaColumn by at least one character; false otherwise
    bool canExtendRight(int msaColumn, const ClosedIntRange &rows) const;       //!< Returns true if any row between rows can be extended right to msaColumn by at least one character; false otherwise
    bool canLevelLeft(int msaColumn, const ClosedIntRange &rows) const;         //!< Returns true if any row between rows can be leveled left to msaColumn by at least one character; false otherwise
    bool canLevelRight(int msaColumn, const ClosedIntRange &rows) const;        //!< Returns true if any row between rows can be leveled right to msaColumn by at least one character; false otherwise
    bool canTrimLeft(int msaColumn, const ClosedIntRange &rows) const;          //!< Returns trus if any row between rows can be trimmed left to msaColumn by at least one character; false otherwise
    bool canTrimRight(int msaColumn, const ClosedIntRange &rows) const;         //!< Returns trus if any row between rows can be trimmed right to msaColumn by at least one character; false otherwise
    void clear();                                                               //!< Remove (and free associated memory) all subseqs
    SubseqChangePodVector collapseLeft(const PosiRect &msaRect);                //!< Collapses all characters in msaRect to the left
    SubseqChangePodVector collapseRight(const PosiRect &msaRect);               //!< Collapses all characters in msaRect to the right
    int columnCount() const;                                                    //!< Returns the number of columns; effectively the same as length()
    //! Maximally extend the start positions of rows up to and including msaColumn; return a vector of changed subseq pods
    QVector<SubseqChangePod> extendLeft(int msaColumn, const ClosedIntRange &rows);
    SubseqChangePod extendLeft(int row, int nCharsToExtend);                    //!< Extend the subseq at row to the left with nCharsToExtend and return a pod describing this change
    //! Extend the subseq at row with extension beginning at msaColumn and return a pod describing this change
    SubseqChangePod extendLeft(int msaColumn, int row, const BioString &extension);
    //! Maximally extend the stop positions of rows up to and including msaColumn; return a vector changed subseq pods
    QVector<SubseqChangePod> extendRight(int msaColumn, const ClosedIntRange &rows);
    //! Extend the subseq at row with extension beginning at msaColumn and return a pod describing this change
    SubseqChangePod extendRight(int msaColumn, int row, const BioString &extension);
    SubseqChangePod extendRight(int row, int nCharsToExtend);                   //!< Extend the subseq at row to the right with nCharsToExtend and return a pod describing this change
    Grammar grammar() const;                                                    //!< Returns the grammar
    int indexOfAbstractSeq(const AbstractSeqSPtr &abstractSeq) const;           //!< Returns the subseq row that contains abstractSeq or 0 if not found or abstractSeq is null
    bool insert(int i, Subseq *subseq);                                         //!< Insert subseq at row index i (1-based), and return whether the addition was successful
    //! insert count columns of gaps before column using the gapCharacter symbol
    void insertGapColumns(int column, int count, char gapCharacter = constants::kDefaultGapCharacter);
    bool insertRows(int row, const QVector<Subseq *> subseqs);                  //!< Inserts subseqs before row (1-based), and returns true if all were added successful; false otherwise
    bool isCompatibleSubseq(const Subseq *subseq) const;                        //!< Returns true if subseq is compatible with this alignment; false otherwise
    bool isCompatibleSubseqVector(const QVector<Subseq *> subseqs) const;       //!< Returns true if all subseqs are compatible with this alignment; false otherwise
    bool isEmpty() const;                                                       //!< Returns true if there are no sequences in the Msa; otherwise returns false
    bool isValidColumn(int column) const;                                       //!< Returns true if column is a valid column; false otherwise
    bool isValidRect(const PosiRect &msaRect) const;                            //!< Returns true if msaRect is completely within the msa boundaries; false otherwise
    bool isValidRow(int row) const;                                             //!< Returns true if row is a valid row; false otherwise
    bool isValidRowRange(const ClosedIntRange &rows) const;                     //!< Returns true if rows references a valid range within the Msa; false otherwise
    int leftExtendableLength(int msaColumm, int row) const;                     //!< Returns the maximum number of characters that may be extended leftward to msaColumn for the subseq at row
    int leftTrimmableLength(int msaColumn, int row) const;                      //!< Returns the maximum number of characters that may be trimmed leftward to msaColumn for the subseq at row
    int length() const;                                                         //!< Return the number of columns in the alignment
    //! Maximally level the start positions of rows up to and including msaColumn; return a vector changed subseq pods
    QVector<SubseqChangePod> levelLeft(int msaColumn, const ClosedIntRange &rows);
    //! Maximally level the stop positions of rows up to and including msaColumn; return a vector changed subseq pods
    QVector<SubseqChangePod> levelRight(int msaColumn, const ClosedIntRange &rows);
    QVector<const Subseq *> members() const;                                    //!< Returns the vector of Subseq pointers comprising this Msa
    void moveRow(int from, int to);                                             //!< Moves the subseq at index position, from, to index position, to
    void moveRowRange(const ClosedIntRange &rows, int to);                      //!< Moves rows to the index position, to
    void moveRowRangeRelative(const ClosedIntRange &rows, int delta);           //!< Moves rows delta positions
    void moveRowRelative(int from, int delta);                                  //!< Moves the Subseq at index delta positions (negative indicates upwards, positive indicates downwards)
    bool prepend(Subseq *subseq);                                               //!< Add subseq at the beginning of the Msa and return whether the addition was successful
    void removeAt(int i);                                                       //!< Remove the Subseq at index i
    void removeFirst();                                                         //!< Remove the first Subseq from the Msa
    void removeRows(const ClosedIntRange &rows);                                //!< Removes rows from the Msa
    QVector<ClosedIntRange> removeGapColumns();                                 //!< Remove any columns which consist entirely of gap characters and return an ordered vector of the gap ranges removed
    QVector<ClosedIntRange> removeGapColumns(const ClosedIntRange &columnRange);//!< Remove any gap columns within columns and return an ordered vector of the gap ranges removed
    void removeLast();                                                          //!< Remove the last Subseq from the Msa
//    void restoreSubseqs(const QVector<Subseq *> subseqs, int row);              //!< Essentially equivalent to inserting subseqs at row, yet more relevant in the context of
    int rightExtendableLength(int msaColumm, int row) const;                    //!< Returns the maximum number of characters that may be extended rightward to msaColumn for the subseq at row
    int rightTrimmableLength(int msaColumn, int row) const;                     //!< Returns the maximum number of characters that may be trimmed rightward to msaColumn for the subseq at row
    int rowCount() const;                                                       //!< Returns the number of subseqs; effectively the same as subseqCount()
    SubseqChangePod setSubseqStart(int row, int newStart);                      //!< Sets the start position of the subseq at row to newStart and returns a SubseqChangePod with the relevant changes
    SubseqChangePod setSubseqStop(int row, int newStop);                        //!< Sets the stop position of the subseq at row to newStop and returns a SubseqChangePod with the relevant changes
    int slideRect(const PosiRect &msaRect, int delta);                          //!< Horizontally slide the characters in msaRect, delta positions and return the direction (negative to the left, positive to the right) and number of positions successfully moved
    void sort(bool (*lessThan)(const Subseq *a, const Subseq *b));              //!< Sorts the subseqs using the comparison function lessThan
    //! Sorts the subseqs using subseqLessThan in sortOrder
    void sort(const ISubseqLessThan &subseqLessThan, Qt::SortOrder sortOrder = Qt::AscendingOrder);
    int subseqCount() const;                                                    //!< Return the number of sequences
    void swap(int i, int j);                                                    //!< Exchange the Subseq at index i with the one at index j
    QVector<Subseq *> takeRows(const ClosedIntRange &rows);                     //!< Extracts and returns a vector of the subseqs between rows
    //! Maximally trim the start positions of rows to msaColumn as possible; return a vector of changed subseq pods
    QVector<SubseqChangePod> trimLeft(int msaColumn, const ClosedIntRange &rows);
    SubseqChangePod trimLeft(int row, int nCharsToRemove);                      //!< Trim nCharsToRemove from the left of the subseq at row and return a pod describing this change
    //! Maximally trim the stop positions of rows to msaColumn as possible; return a vector changed subseq pods
    QVector<SubseqChangePod> trimRight(int msaColumn, const ClosedIntRange &rows);
    SubseqChangePod trimRight(int row, int nCharsToRemove);                     //!< Trim nCharsToRemove from the right of the subseq at row and return a pod describing this change
    //!< Performs the inverse of each change in subseqChangePodVector and returns a equivalently sized vector of the changes that were made
    SubseqChangePodVector undo(const SubseqChangePodVector &subseqChangePodVector);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    Subseq *subseqFromRow(int i) const;                                         //!< Returns the subseq at row i (1-based)

    // ------------------------------------------------------------------------------------------------
    // Protected members
    QVector<Subseq *> subseqs_;         //!< Vector of Subseqs that comprise the alignment


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! This method is presumably the better method for finding gaps :) Need to test
    QVector<ClosedIntRange> findGapColumns_iteratorRowBased(const ClosedIntRange &columnRange) const;
    QVector<ClosedIntRange> findGapColumns_nonIteratorColumnBased() const;

    // ------------------------------------------------------------------------------------------------
    // Private members
    Grammar grammar_;                   //!< Grammar of the Msa
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Simply maps the 1-based row index, i, to its 0-based QVector equivalent.
  *
  * @param i [int]
  * @returns Subseq *
  */
inline
Subseq *Msa::subseqFromRow(int i) const
{
    return subseqs_.at(i - 1);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Defunct methods
// SubseqChangePod setSubseqStartHelper(int row, int newStart);                //!< Updates the non-gapped start value of the Subseq at row to newStart and returns a SubseqChangePod
// SubseqChangePod setSubseqStopHelper(int row, int newStop);                  //!< Updates the non-gapped stop value of the Subseq at row to newStop and returns a SubseqChangePod


#endif // MSA_H
