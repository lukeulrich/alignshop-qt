/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSLICEMODEL_H
#define AMINOSLICEMODEL_H

#include "SubseqSliceModel.h"

#include <QtCore/QStringList>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

/**
  * AminoSliceModel is a concrete implementation of SubseqSliceModel for managing amino acid data slices.
  *
  * AminoSliceModel contains the following columns of data (each mapped to a static class constant integer):
  * 1. Subseq id
  * 2. Subseq amino sequence id
  * 3. Subseq label
  * 4. Subseq start
  * 5. Subseq stop
  * 6. Subseq sequence
  * 7. Subseq notes
  * 8. Subseq created
  * 9. Sequence id
  * 10. Sequence astring id
  * 11. Sequence label
  * 12. Sequence source
  * 13. Sequence protein
  * 14. Sequence created
  */
class AminoSliceModel : public SubseqSliceModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit AminoSliceModel(QObject *parent = 0);                                          //!< Default constructor

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;              //!< Returns the data associated for the given index and role

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isEditableColumn(int column) const;                                                //!< Returns true if column is an editable column
    void tweakHorizontalHeader(QHeaderView *header) const;                                  //!< Tweaks the appearance of the horizontal headerview

    static const QStringList subseqFields_;
    static const int kSubseqIdColumn;
    static const int kSubseqAminoSeqIdColumn;
    static const int kSubseqLabelColumn;
    static const int kSubseqStartColumn;
    static const int kSubseqStopColumn;
    static const int kSubseqSequenceColumn;
    static const int kSubseqNotesColumn;
    static const int kSubseqCreatedColumn;

    static const QStringList seqFields_;
    static const int kSeqIdColumn;
    static const int kSeqAstringIdColumn;
    static const int kSeqLabelColumn;
    static const int kSeqSourceColumn;
    static const int kSeqProteinColumn;
    static const int kSeqCreatedColumn;

    // Friendly names for each of the columns
    static const QStringList friendlySubseqFieldNames_;
    static const QStringList friendlySeqFieldNames_;
};

#endif // AMINOSLICEMODEL_H
