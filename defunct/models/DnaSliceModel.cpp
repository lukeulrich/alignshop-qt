/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DnaSliceModel.h"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static member initialization
const QStringList DnaSliceModel::subseqFields_(QStringList() << "id" << "dna_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
const int DnaSliceModel::kSubseqIdColumn = 0;
const int DnaSliceModel::kSubseqDnaSeqIdColumn = 1;
const int DnaSliceModel::kSubseqLabelColumn = 2;
const int DnaSliceModel::kSubseqStartColumn = 3;
const int DnaSliceModel::kSubseqStopColumn = 4;
const int DnaSliceModel::kSubseqSequenceColumn = 5;
const int DnaSliceModel::kSubseqNotesColumn = 6;
const int DnaSliceModel::kSubseqCreatedColumn = 7;

const QStringList DnaSliceModel::seqFields_(QStringList() << "id" << "dstring_id" << "label" << "source" << "gene" << "created");
const int DnaSliceModel::kSeqIdColumn = 8;
const int DnaSliceModel::kSeqDstringIdColumn = 9;
const int DnaSliceModel::kSeqLabelColumn = 10;
const int DnaSliceModel::kSeqSourceColumn = 11;
const int DnaSliceModel::kSeqGeneColumn = 12;
const int DnaSliceModel::kSeqCreatedColumn = 13;

const QStringList DnaSliceModel::friendlySubseqFieldNames_(QStringList() << "DNA Subseq ID" << "DNA Seq ID" << "Subseq Label" << "Start" << "Stop" << "Sequence" << "Notes" << "Created");
const QStringList DnaSliceModel::friendlySeqFieldNames_(QStringList() << "DNA Seq ID" << "Dstring ID" << "Seq Label" << "Source" << "Gene" << "Created");

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
DnaSliceModel::DnaSliceModel(QObject *parent) : SubseqSliceModel(parent)
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
QVariant DnaSliceModel::data(const QModelIndex &index, int role) const
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
bool DnaSliceModel::isEditableColumn(int column) const
{
    return column == kSubseqLabelColumn
           || column == kSubseqNotesColumn
           || column == kSeqLabelColumn
           || column == kSeqSourceColumn
           || column == kSeqGeneColumn;
}

/**
  * @param header [QHeaderView *]
  */
void DnaSliceModel::tweakHorizontalHeader(QHeaderView *header) const
{
    ASSERT_X(header, "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Hide the id columns
    header->hideSection(kSubseqIdColumn);
    header->hideSection(kSubseqDnaSeqIdColumn);
    header->hideSection(kSubseqCreatedColumn);

    header->hideSection(kSeqIdColumn);
    header->hideSection(kSeqDstringIdColumn);
    header->hideSection(kSeqCreatedColumn);

    // Specify sizes of columns
    header->setResizeMode(kSubseqStartColumn, QHeaderView::ResizeToContents);
    header->setResizeMode(kSubseqStopColumn, QHeaderView::ResizeToContents);
//    header->setResizeMode(constants::kColumnAminoSequence, QHeaderView::Stretch);

    // Call parent method
    header->setStretchLastSection(false);
}
