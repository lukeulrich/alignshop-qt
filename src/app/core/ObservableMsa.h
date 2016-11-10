/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef OBSERVABLEMSA_H
#define OBSERVABLEMSA_H

#include <QtCore/QObject>

#include "Msa.h"

#include "util/ClosedIntRange.h"
#include "util/PosiRect.h"
#include "global.h"

/**
  * ObservableMsa simply extends Msa with signals and slots such that client classes may observe and react to any
  * changes.
  */
class ObservableMsa : public QObject, public Msa
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct an empty ObservableMsa with id, grammar, and parent
    explicit ObservableMsa(Grammar grammar = eUnknownGrammar, QObject *parent = nullptr);


public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isModified() const;                                                    //!< Returns true if this msa has been modified; false otherwise

    // Could not make the sort method a slot because function pointers apparently do not play well with the moc system
    void sort(bool (*lessThan)(const Subseq *a, const Subseq *b));              //!< Sorts the member subseqs using greaterThan; emits subseqOrderAboutToBeChanged before sorting and subseqOrderChanged after sorting is complete
    //! Sorts the subseqs using subseqLessThan in sortOrder
    void sort(const ISubseqLessThan &subseqLessThan, Qt::SortOrder sortOrder = Qt::AscendingOrder);

    // ------------------------------------------------------------------------------------------------
    // Re-implemented public methods
public Q_SLOTS:
    bool append(Subseq *subseq);                                                //!< Add subseq to the Msa and return whether the addition was successful
    void clear();                                                               //!< Remove (and free associated memory) all subseqs
    SubseqChangePodVector collapseLeft(const PosiRect &msaRect);                //!< Collapses all characters in msaRect to the left
    SubseqChangePodVector collapseRight(const PosiRect &msaRect);               //!< Collapses all characters in msaRect to the right
    SubseqChangePodVector extendLeft(int msaColumn, const ClosedIntRange &rows);//!< Extend the start positions of rows to msaColumn as possible
    SubseqChangePod extendLeft(int row, int nCharsToExtend);                    //!< Extend the subseq at row to the left with nCharsToExtend and return a pod describing this change
    //! Extend the subseq at row with extension beginning at msaColumn and return a pod describing this change
    SubseqChangePod extendLeft(int msaColumn, int row, const BioString &extension);
    //! Extend the stop positions of rows to msaColumn as possible
    SubseqChangePodVector extendRight(int msaColumn, const ClosedIntRange &rows);
    //! Extend the subseq at row with extension beginning at msaColumn and return a pod describing this change
    SubseqChangePod extendRight(int msaColumn, int row, const BioString &extension);
    SubseqChangePod extendRight(int row, int nCharsToExtend);                   //!< Extend the subseq at row to the right with nCharsToExtend and return a pod describing this change
    bool insert(int i, Subseq *subseq);                                         //!< Insert subseq at row index i (1-based), and return whether the addition was successful
    //! insert count columns of gaps before column using the gapCharacter symbol
    void insertGapColumns(int column, int count, char gapCharacter = constants::kDefaultGapCharacter);
    bool insertRows(int row, const QVector<Subseq *> subseqs);                  //!< Inserts subseqs before row (1-based), and returns true if all were added successful; false otherwise
    SubseqChangePodVector levelLeft(int msaColumn, const ClosedIntRange &rows); //!< Level the start positions of rows to msaColumn as possible
    SubseqChangePodVector levelRight(int msaColumn, const ClosedIntRange &rows);//!< Level the stop positions of rows to msaColumn as possible
    void moveRow(int from, int to);                                             //!< Moves the subseq at index position, from, to index position, to
    void moveRowRange(const ClosedIntRange &rows, int to);                      //!< Moves rows to the index position, to
    void moveRowRangeRelative(const ClosedIntRange &rows, int delta);           //!< Moves rows delta positions
    void moveRowRelative(int from, int delta);                                  //!< Moves the Subseq at index delta positions (negative indicates upwards, positive indicates downwards)
    bool prepend(Subseq *subseq);                                               //!< Add subseq at the beginning of the Msa and return whether the addition was successful
    void removeAt(int i);                                                       //!< Remove the Subseq at index i
    void removeFirst();                                                         //!< Remove the first Subseq from the Msa
    void removeRows(const ClosedIntRange &rows);                                //!< Removes rows from the Msa
    QVector<ClosedIntRange> removeGapColumns();                                 //!< Remove any columns which consist entirely of gap characters and return the number of columns removed
    QVector<ClosedIntRange> removeGapColumns(const ClosedIntRange &columnRange);//!< Remove any gap columns within columns
    void removeLast();                                                          //!< Remove the last Subseq from the Msa
    void setModified(bool modified);                                            //!< Sets the modified status to modified
    SubseqChangePod setSubseqStart(int row, int newStart);                      //!< Sets the start position of the subseq at row to newStart
    SubseqChangePod setSubseqStop(int row, int newStop);                        //!< Sets the stop position of the subseq at row to newStop
    int slideRect(const PosiRect &msaRect, int delta);                          //!< Horizontally slide the characters in msaRect, delta positions and return the direction (negative to the left, positive to the right) and number of positions successfully moved
    void swap(int i, int j);                                                    //!< Exchange the Subseq at index i with the one at index j
    QVector<Subseq *> takeRows(const ClosedIntRange &rows);                     //!< Extracts and returns a vector of the subseqs between rows
    SubseqChangePodVector trimLeft(int msaColumn, const ClosedIntRange &rows);  //!< Maximally trim the start positions of rows to msaColumn as possible
    SubseqChangePodVector trimRight(int msaColumn, const ClosedIntRange &rows); //!< Maximally trim the stop positions of rows to msaColumn as possible
    //!< Performs the inverse of each change in subseqChangePodVector and returns a equivalently sized vector of the changes that were made
    SubseqChangePodVector undo(const SubseqChangePodVector &subseqChangePodVector);

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void aboutToBeCollapsedLeft(const PosiRect &msaRect);                       //!< Emitted just before msaRect (normalized) is collapsed to the left
    void aboutToBeCollapsedRight(const PosiRect &msaRect);                      //!< Emitted just before msaRect (normalized) is collapsed to the right
    void collapsedLeft(const PosiRect &msaRect);                                //!< Emitted after msaRect has been collapsed to the left
    void collapsedRight(const PosiRect &msaRect);                               //!< Emitted after msaRect has been collapsed to the right
    void gapColumnsAboutToBeInserted(const ClosedIntRange &columns);            //!< Emitted just before gap columns are inserted
    void gapColumnsInserted(const ClosedIntRange &columns);                     //!< Emitted after gap columns have been inserted
    void gapColumnsRemoved(const QVector<ClosedIntRange> &columnRanges);        //!< Emitted after gap columns have been removed
    //! Emitted when the region originally located at msaRect is horizontally moved delta positions and is currently positioned within finalRange
    void msaAboutToBeReset();                                                   //!< Emitted just before the msa is reset and all sequences are removed from the Msa (e.g. via the clear method)
    void msaReset();                                                            //!< Emitted after the msa has been reset and all sequences have been removed from the Msa (e.g. via the clear method)
    void modifiedChanged(bool modified);                                        //!< Emitted when the modified status has changed
    //! Emitted when the rectangle at msaRect has been slid delta positions and is now horizontally located at finalRange (values are valid and not inverted, i.e. begin <= end)
    void rectangleSlid(const PosiRect &msaRect, int delta, const ClosedIntRange &finalRange);
    void rowsAboutToBeInserted(const ClosedIntRange &rows);                     //!< Emitted just before subseqs are inserted; new subseqs will be placed at rows
    void rowsAboutToBeMoved(const ClosedIntRange &rows, int finalRow);          //!< Emitted just before subseqs are moved; the subseqs in rows will be placed at finalRow
    void rowsAboutToBeRemoved(const ClosedIntRange &rows);                      //!< Emitted just before subseqs are removed; subseqs will be removed from rows
    void rowsAboutToBeSorted();                                                 //!< Emitted just before the subseqs are sorted
    void rowsAboutToBeSwapped(int from, int to);                                //!< Emitted just before the subseqs at from and to are swapped
    void rowsInserted(const ClosedIntRange &rows);                              //!< Emitted after subseqs have been inserted in rows
    void rowsMoved(const ClosedIntRange &rows, int finalRow);                   //!< Emitted after subseqs have been moved; the subseqs previously in rows are now located at finalRow
    void rowsRemoved(const ClosedIntRange &rows);                               //!< Emitted after subseqs have been removed; subseqs were previously located at rows
    void rowsSorted();                                                          //!< Emitted after subseqs have been sorted
    void rowsSwapped(int from, int to);                                         //!< Emitted after the subseqs at rows from and to are swapped
    void subseqsChanged(const SubseqChangePodVector &subseqChangePods);         //!< Emitted after the subseqs in subseqChangePods have had their borders modified via either an extension, trim, level, collapse, setSubseqStart, or setSubseqStop

private:
    bool modified_;

    friend class SortMsaCommand;
};

#endif // OBSERVABLEMSA_H
