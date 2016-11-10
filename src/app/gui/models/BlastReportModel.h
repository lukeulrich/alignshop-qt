/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTREPORTMODEL_H
#define BLASTREPORTMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QVector>
#include "../../core/global.h"
#include "../../core/types.h"
#include "../../core/Entities/BlastReport.h"

class BlastReportModel : public QAbstractTableModel
{
public:
    enum Columns
    {
        eIdColumn = 0,
        eAccessionColumn,
        eDefinitionColumn,
        eHitLengthColumn,
        eBitScoreColumn,
        eScoreColumn,
        eEvalueColumn,
        eQueryFromColumn,
        eQueryToColumn,
        eHitFromColumn,
        eHitToColumn,
        eQueryFrameColumn,
        eHitFrameColumn,
        eIdentitiesColumn,
        ePositivesColumn,
        eGapsColumn,
        eAlignLengthColumn,
        eQueryAlignColumn,
        eHitAlignColumn,
        eMidlineColumn,
        eAlignmentColumn,

        eNumberOfColumns
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    BlastReportModel(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Reimplemented virtual methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BlastReportSPtr blastReport() const;
    void setBlastReport(const BlastReportSPtr &blastReport);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void rebuildIndex();


    // ------------------------------------------------------------------------------------------------
    // Private members
    BlastReportSPtr blastReport_;
    QVector<PairInt> hspIndex_;
};

#endif // BLASTREPORTMODEL_H
