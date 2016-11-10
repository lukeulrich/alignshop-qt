/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "RnaSliceModel.h"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>

#include "global.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Static member initialization
const QStringList RnaSliceModel::subseqFields_(QStringList() << "id" << "rna_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
const int RnaSliceModel::kSubseqIdColumn = 0;
const int RnaSliceModel::kSubseqRnaSeqIdColumn = 1;
const int RnaSliceModel::kSubseqLabelColumn = 2;
const int RnaSliceModel::kSubseqStartColumn = 3;
const int RnaSliceModel::kSubseqStopColumn = 4;
const int RnaSliceModel::kSubseqSequenceColumn = 5;
const int RnaSliceModel::kSubseqNotesColumn = 6;
const int RnaSliceModel::kSubseqCreatedColumn = 7;

const QStringList RnaSliceModel::seqFields_(QStringList() << "id" << "rstring_id" << "label" << "source" << "gene" << "created");
const int RnaSliceModel::kSeqIdColumn = 8;
const int RnaSliceModel::kSeqRstringIdColumn = 9;
const int RnaSliceModel::kSeqLabelColumn = 10;
const int RnaSliceModel::kSeqSourceColumn = 11;
const int RnaSliceModel::kSeqGeneColumn = 12;
const int RnaSliceModel::kSeqCreatedColumn = 13;

const QStringList RnaSliceModel::friendlySubseqFieldNames_(QStringList() << "RNA Subseq ID" << "RNA Seq ID" << "Subseq Label" << "Start" << "Stop" << "Sequence" << "Notes" << "Created");
const QStringList RnaSliceModel::friendlySeqFieldNames_(QStringList() << "RNA Seq ID" << "Rstring ID" << "Seq Label" << "Source" << "Gene" << "Created");

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
RnaSliceModel::RnaSliceModel(QObject *parent) : SubseqSliceModel(parent)
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
QVariant RnaSliceModel::data(const QModelIndex &index, int role) const
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
bool RnaSliceModel::isEditableColumn(int column) const
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
void RnaSliceModel::tweakHorizontalHeader(QHeaderView *header) const
{
    ASSERT_X(header, "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Hide the id columns
    header->hideSection(kSubseqIdColumn);
    header->hideSection(kSubseqRnaSeqIdColumn);
    header->hideSection(kSubseqCreatedColumn);

    header->hideSection(kSeqIdColumn);
    header->hideSection(kSeqRstringIdColumn);
    header->hideSection(kSeqCreatedColumn);

    // Specify sizes of columns
    header->setResizeMode(kSubseqStartColumn, QHeaderView::ResizeToContents);
    header->setResizeMode(kSubseqStopColumn, QHeaderView::ResizeToContents);
//    header->setResizeMode(constants::kColumnAminoSequence, QHeaderView::Stretch);

    // Call parent method
    header->setStretchLastSection(false);
}
