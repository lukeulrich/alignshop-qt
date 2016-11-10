/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef RNASLICEMODEL_H
#define RNASLICEMODEL_H

#include "SubseqSliceModel.h"

#include <QtCore/QSet>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

/**
  * RnaSliceModel is a concrete implementation of SubseqSliceModel for managing RNA data slices.
  *
  * RnaSliceModel contains the following columns of data (each mapped to a static class constant integer):
  * 1. Subseq id
  * 2. Subseq RNA sequence id
  * 3. Subseq label
  * 4. Subseq start
  * 5. Subseq stop
  * 6. Subseq sequence
  * 7. Subseq notes
  * 8. Subseq created
  * 9. Sequence id
  * 10. Sequence rstring id
  * 11. Sequence label
  * 12. Sequence source
  * 13. Sequence gene
  * 14. Sequence created
  */
class RnaSliceModel : public SubseqSliceModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit RnaSliceModel(QObject *parent = 0);                                            //!< Default constructor

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;              //!< Returns the data associated for the given index and role

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isEditableColumn(int column) const;                                                //!< Returns true if column is an editable column
    void tweakHorizontalHeader(QHeaderView *header) const;                                  //!< Tweaks the appearance of the horizontal headerview

    static const QStringList subseqFields_;
    static const int kSubseqIdColumn;
    static const int kSubseqRnaSeqIdColumn;
    static const int kSubseqLabelColumn;
    static const int kSubseqStartColumn;
    static const int kSubseqStopColumn;
    static const int kSubseqSequenceColumn;
    static const int kSubseqNotesColumn;
    static const int kSubseqCreatedColumn;

    static const QStringList seqFields_;
    static const int kSeqIdColumn;
    static const int kSeqRstringIdColumn;
    static const int kSeqLabelColumn;
    static const int kSeqSourceColumn;
    static const int kSeqGeneColumn;
    static const int kSeqCreatedColumn;

    // Friendly names for each of the columns
    static const QStringList friendlySubseqFieldNames_;
    static const QStringList friendlySeqFieldNames_;
};

#endif // RNASLICEMODEL_H
