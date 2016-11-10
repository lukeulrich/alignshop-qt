/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AminoSliceModel.h"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static member initialization
const QStringList AminoSliceModel::subseqFields_(QStringList() << "id" << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
const int AminoSliceModel::kSubseqIdColumn = 0;
const int AminoSliceModel::kSubseqAminoSeqIdColumn = 1;
const int AminoSliceModel::kSubseqLabelColumn = 2;
const int AminoSliceModel::kSubseqStartColumn = 3;
const int AminoSliceModel::kSubseqStopColumn = 4;
const int AminoSliceModel::kSubseqSequenceColumn = 5;
const int AminoSliceModel::kSubseqNotesColumn = 6;
const int AminoSliceModel::kSubseqCreatedColumn = 7;

const QStringList AminoSliceModel::seqFields_(QStringList() << "id" << "astring_id" << "label" << "source" << "protein" << "created");
const int AminoSliceModel::kSeqIdColumn = 8;
const int AminoSliceModel::kSeqAstringIdColumn = 9;
const int AminoSliceModel::kSeqLabelColumn = 10;
const int AminoSliceModel::kSeqSourceColumn = 11;
const int AminoSliceModel::kSeqProteinColumn = 12;
const int AminoSliceModel::kSeqCreatedColumn = 13;

const QStringList AminoSliceModel::friendlySubseqFieldNames_(QStringList() << "Amino Subseq ID" << "Amino Seq ID" << "Subseq Label" << "Start" << "Stop" << "Sequence" << "Notes" << "Created");
const QStringList AminoSliceModel::friendlySeqFieldNames_(QStringList() << "Amino Seq ID" << "Astring ID" << "Seq Label" << "Source" << "Protein" << "Created");

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
AminoSliceModel::AminoSliceModel(QObject *parent) : SubseqSliceModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant AminoSliceModel::data(const QModelIndex &index, int role) const
{
    bool handled = false;
    QVariant parentResponse = SubseqSliceModel::data(index, role, handled);
    if (handled)
        return parentResponse;

    if (role == Qt::FontRole)
    {
        switch(index.column())
        {
        case kSeqSourceColumn:
            {
                QFont font;
                font.setItalic(true);
                return font;
            }
        case kSubseqSequenceColumn:
            return QFont("monospace");
        }
    }

    return QVariant();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param column [int]
  * @returns bool
  */
bool AminoSliceModel::isEditableColumn(int column) const
{
    return column == kSubseqLabelColumn
           || column == kSubseqNotesColumn
           || column == kSeqLabelColumn
           || column == kSeqSourceColumn
           || column == kSeqProteinColumn;
}

/**
  * @param header [QHeaderView *]
  */
void AminoSliceModel::tweakHorizontalHeader(QHeaderView *header) const
{
    ASSERT_X(header, "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Hide the id columns
    header->hideSection(kSubseqIdColumn);
    header->hideSection(kSubseqAminoSeqIdColumn);
    header->hideSection(kSubseqCreatedColumn);

    header->hideSection(kSeqIdColumn);
    header->hideSection(kSeqAstringIdColumn);
    header->hideSection(kSeqCreatedColumn);

    // Specify sizes of columns
    header->setResizeMode(kSubseqStartColumn, QHeaderView::ResizeToContents);
    header->setResizeMode(kSubseqStopColumn, QHeaderView::ResizeToContents);
//    header->setResizeMode(constants::kColumnAminoSequence, QHeaderView::Stretch);

    // Call parent method
    header->setStretchLastSection(false);
}
