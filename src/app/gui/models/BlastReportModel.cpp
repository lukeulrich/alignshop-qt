/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>

#include "BlastReportModel.h"
#include "../../core/Entities/BlastReport.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastReportModel::BlastReportModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int BlastReportModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant BlastReportModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const HitPod &hitPod = blastReport_->hits().at(hspIndex_.at(index.row()).first);
        const HspPod &hspPod = hitPod.hsps_.at(hspIndex_.at(index.row()).second);

        switch (index.column())
        {
        case eIdColumn:                 return hitPod.id_;                              break;
        case eAccessionColumn:          return hitPod.accession_;                       break;
        case eDefinitionColumn:         return hitPod.definition_;                      break;
        case eHitLengthColumn:          return hitPod.length_;                          break;
        case eBitScoreColumn:           return hspPod.bitScore_;                        break;
        case eScoreColumn:              return hspPod.rawScore_;                        break;
        case eEvalueColumn:             return hspPod.evalue_;                          break;
        case eQueryFromColumn:          return hspPod.queryRange_.begin_;               break;
        case eQueryToColumn:            return hspPod.queryRange_.end_;                 break;
        case eHitFromColumn:            return hspPod.subjectRange_.begin_;             break;
        case eHitToColumn:              return hspPod.subjectRange_.end_;               break;
        case eQueryFrameColumn:         return hspPod.queryFrame_;                      break;
        case eHitFrameColumn:           return hspPod.subjectFrame_;                    break;
        case eIdentitiesColumn:         return hspPod.identities_;                      break;
        case ePositivesColumn:          return hspPod.positives_;                       break;
        case eGapsColumn:               return hspPod.gaps_;                            break;
        case eAlignLengthColumn:        return hspPod.length_;                          break;
        case eQueryAlignColumn:         return hspPod.queryAlignment_.asByteArray();    break;
        case eHitAlignColumn:           return hspPod.subjectAlignment_.asByteArray();  break;
        case eMidlineColumn:            return hspPod.midline_;                         break;
        case eAlignmentColumn:
            return hspPod.queryAlignment_.asByteArray() + "\n" +
                   hspPod.midline_ + "\n" +
                   hspPod.subjectAlignment_.asByteArray();

        default:
            break;
        }
    }
    else if (role == Qt::FontRole)
    {
        switch (index.column())
        {
        case eQueryAlignColumn:
        case eHitAlignColumn:
        case eMidlineColumn:
        case eAlignmentColumn:
            return QFont("Monospace");

        default:
            break;
        }
    }

    return QVariant();
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant BlastReportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section + 1;

    switch (section)
    {
    case eIdColumn:                 return "ID";                break;
    case eAccessionColumn:          return "Accession";         break;
    case eDefinitionColumn:         return "Definition";        break;
    case eHitLengthColumn:          return "Hit Length";        break;
    case eBitScoreColumn:           return "Bit Score";         break;
    case eScoreColumn:              return "Raw Score";         break;
    case eEvalueColumn:             return "E-value";           break;
    case eQueryFromColumn:          return "Query Start";       break;
    case eQueryToColumn:            return "Query Stop";        break;
    case eHitFromColumn:            return "Hit Start";         break;
    case eHitToColumn:              return "Hit Stop";          break;
    case eQueryFrameColumn:         return "Query Frame";       break;
    case eHitFrameColumn:           return "Hit Frame";         break;
    case eIdentitiesColumn:         return "Identities";        break;
    case ePositivesColumn:          return "Positives";         break;
    case eGapsColumn:               return "Gaps";              break;
    case eAlignLengthColumn:        return "Length";            break;
    case eQueryAlignColumn:         return "Query Alignment";   break;
    case eHitAlignColumn:           return "Hit Alignment";     break;
    case eMidlineColumn:            return "Midline";           break;
    case eAlignmentColumn:          return "Alignment";         break;

    default:
        break;
    }

    return QVariant();
}

/**
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex BlastReportModel::index(int row, int column, const QModelIndex & /* parent */) const
{
    if (blastReport_ == nullptr)
        return QModelIndex();

    if (column < 0 || column >= columnCount())
        return QModelIndex();

    if (row < 0 || row >= rowCount())
        return QModelIndex();

    return createIndex(row, column);
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int BlastReportModel::rowCount(const QModelIndex & /* parent */) const
{
    return hspIndex_.size();
}

/**
  * @returns BlastReport *
  */
BlastReportSPtr BlastReportModel::blastReport() const
{
    return blastReport_;
}

/**
  * @param blastReport [BlastReport *]
  */
void BlastReportModel::setBlastReport(const BlastReportSPtr &blastReport)
{
    if (blastReport_ == blastReport)
        return;

    beginResetModel();
    blastReport_ = blastReport;
    rebuildIndex();
    endResetModel();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  */
void BlastReportModel::rebuildIndex()
{
    hspIndex_.clear();

    if (blastReport_ == nullptr)
        return;

    for (int i=0, z=blastReport_->hits().size(); i<z; ++i)
        for (int j=0; j< blastReport_->hits().at(i).hsps_.size(); ++j)
            hspIndex_ << PairInt(i, j);
}
