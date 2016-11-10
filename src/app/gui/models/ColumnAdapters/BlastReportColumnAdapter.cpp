/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QIcon>

#include "BlastReportColumnAdapter.h"
#include "../../../core/Entities/BlastReport.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastReportColumnAdapter::BlastReportColumnAdapter(QObject *parent)
    : AbstractColumnAdapter(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int BlastReportColumnAdapter::columnCount() const
{
    return eNumberOfColumns;
}

/**
  * @param column [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags BlastReportColumnAdapter::flags(int column) const
{
    Qt::ItemFlags defaultFlags = AbstractColumnAdapter::flags(column);

    switch (column)
    {
    case eNameColumn:
    case eDescriptionColumn:
    case eNotesColumn:
        defaultFlags |= Qt::ItemIsEditable;

    // TODO: Make this column return the actual subsequence and not the full sequence
    default:
        break;
    }

    return defaultFlags;
}

/**
  * @param entity [const IEntitySPtr &]
  * @param column [int]
  * @param role [int]
  * @returns QVariant
  */
QVariant BlastReportColumnAdapter::data(const IEntitySPtr &entity, int column, int role) const
{
    ASSERT(boost::shared_dynamic_cast<BlastReport>(entity));
    const BlastReportSPtr &blastReport = boost::shared_static_cast<BlastReport>(entity);
    ASSERT(blastReport);

    switch (role)
    {
    // ----------------------------------------------
    // Display role
    case Qt::DisplayRole:
        switch (column)
        {
        case eIdColumn:
            return blastReport->id();
        case eTypeColumn:
            return "BLAST";
        case eNameColumn:
            return blastReport->name();
        case eQuerySeqId:
            return blastReport->querySeqId();
        case eQueryStartColumn:
            return blastReport->queryRange().begin_;
        case eQueryStopColumn:
            return blastReport->queryRange().end_;
        case eQuerySequenceColumn:
            return blastReport->query().asByteArray();
        case eHitsColumn:
            return blastReport->hits().size();
        case eDescriptionColumn:
            return blastReport->description();
        case eNotesColumn:
            return blastReport->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Edit role
    case Qt::EditRole:
        switch (column)
        {
        case eNameColumn:
            return blastReport->name();
        case eDescriptionColumn:
            return blastReport->description();
        case eNotesColumn:
            return blastReport->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Decoration role
    case Qt::DecorationRole:
        if (column == eNameColumn)
            return QIcon(":/aliases/images/icons/blast");
        return QVariant();

    default:
        return QVariant();
    }
}

/**
  * @param column [int]
  * @returns QVariant
  */
QVariant BlastReportColumnAdapter::headerData(int column) const
{
    switch (column)
    {
    case eIdColumn:
        return "ID";
    case eTypeColumn:
        return "Type";
    case eNameColumn:
        return "Name";
    case eQuerySeqId:
        return "Query ID";
    case eQueryStartColumn:
        return "Query Start";
    case eQueryStopColumn:
        return "Query Stop";
    case eQuerySequenceColumn:
        return "Query Sequence";
    case eHitsColumn:
        return "Hits";
    case eDescriptionColumn:
        return "Description";
    case eNotesColumn:
        return "Notes";

    default:
        return QVariant();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param entity [const IEntitySPtr &]
  * @param column [int]
  * @param value [const QVariant &]
  * @returns bool
  */
bool BlastReportColumnAdapter::setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
{
    if (!entity)
        return false;

    ASSERT(boost::shared_dynamic_cast<BlastReport>(entity));
    BlastReportSPtr blastReport = boost::shared_static_cast<BlastReport>(entity);

    switch (column)
    {
    case eNameColumn:
        blastReport->setName(value.toString());
        break;
    case eDescriptionColumn:
        blastReport->setDescription(value.toString());
        break;
    case eNotesColumn:
        blastReport->setNotes(value.toString());
        break;

    default:
        return false;
    }

    emit dataChanged(entity, column);
    return true;
}
