/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>
#include <QtGui/QIcon>

#include "DnaSeqColumnAdapter.h"
#include "../CustomRoles.h"
#include "../../../core/Entities/DnaSeq.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
DnaSeqColumnAdapter::DnaSeqColumnAdapter(QObject *parent)
    : AbstractColumnAdapter(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int DnaSeqColumnAdapter::columnCount() const
{
    return eNumberOfColumns;
}

/**
  * @param column [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags DnaSeqColumnAdapter::flags(int column) const
{
    Qt::ItemFlags defaultFlags = AbstractColumnAdapter::flags(column);

    switch (column)
    {
    case eNameColumn:
    case eStartColumn:
    case eStopColumn:
    case eSourceColumn:
    case eDescriptionColumn:
    case eNotesColumn:
        defaultFlags |= Qt::ItemIsEditable;
        break;

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
QVariant DnaSeqColumnAdapter::data(const IEntitySPtr &entity, int column, int role) const
{
    if (!entity)
        return false;

    ASSERT(boost::shared_dynamic_cast<const DnaSeq>(entity));
    const DnaSeqSPtr &dnaSeq = boost::shared_static_cast<DnaSeq>(entity);
    ASSERT(dnaSeq);

    switch (role)
    {
    // ----------------------------------------------
    // Display role
    case Qt::DisplayRole:
        switch (column)
        {
        case eIdColumn:
            return dnaSeq->id();
        case eTypeColumn:
            return "DNA";
        case eNameColumn:
            return dnaSeq->name();
        case eSourceColumn:
            return dnaSeq->source();
        case eStartColumn:
            return dnaSeq->start();
        case eStopColumn:
            return dnaSeq->stop();
        case eLengthColumn:
            return dnaSeq->length();
        case eDescriptionColumn:
            return dnaSeq->description();
        case eNotesColumn:
            return dnaSeq->notes();
        case eSequenceColumn:
            return dnaSeq->bioString().asByteArray();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Edit role
    case Qt::EditRole:
        switch (column)
        {
        case eNameColumn:
            return dnaSeq->name();
        case eSourceColumn:
            return dnaSeq->source();
        case eStartColumn:
            return dnaSeq->start();
        case eStopColumn:
            return dnaSeq->stop();
        case eDescriptionColumn:
            return dnaSeq->description();
        case eNotesColumn:
            return dnaSeq->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Font role
    case Qt::FontRole:
        switch (column)
        {
        case eSourceColumn:
            {
                QFont font;
                font.setItalic(true);
                return font;
            }
            break;

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Min role
    case CustomRoles::kMinRole:
        switch (column)
        {
        case eStartColumn:
            return 1;
        case eStopColumn:
            return dnaSeq->start();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Max role
    case CustomRoles::kMaxRole:
        switch (column)
        {
        case eStartColumn:
            return dnaSeq->stop();
        case eStopColumn:
            return dnaSeq->seqLength();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Decoration role
    case Qt::DecorationRole:
        if (column == eNameColumn)
            return QIcon(":/aliases/images/icons/dna-seq");
        return QVariant();

    default:
        return QVariant();
    }

    return QVariant();
}

/**
  * @param column [int]
  * @returns QVariant
  */
QVariant DnaSeqColumnAdapter::headerData(int column) const
{
    switch (column)
    {
    case eIdColumn:
        return "ID";
    case eTypeColumn:
        return "Type";
    case eNameColumn:
        return "Name";
    case eSourceColumn:
        return "Source";
    case eStartColumn:
        return "Start";
    case eStopColumn:
        return "Stop";
    case eLengthColumn:
        return "Length";
    case eDescriptionColumn:
        return "Description";
    case eNotesColumn:
        return "Notes";
    case eSequenceColumn:
        return "Sequence";

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
bool DnaSeqColumnAdapter::setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
{
    ASSERT(entity);
    ASSERT(boost::shared_dynamic_cast<const DnaSeq>(entity));
    DnaSeqSPtr dnaSeq = boost::shared_static_cast<DnaSeq>(entity);

    bool success = true;
    switch (column)
    {
    case eNameColumn:
        dnaSeq->setName(value.toString());
        break;
    case eSourceColumn:
        dnaSeq->setSource(value.toString());
        break;
    case eStartColumn:
        {
            bool ok = false;
            int newStart = value.toInt(&ok);
            if (!ok)
                return false;
            success = dnaSeq->setStart(newStart);
            if (success)
                emit dataChanged(entity, eSequenceColumn);
        }
        break;
    case eStopColumn:
        {
            bool ok = false;
            int newStop = value.toInt(&ok);
            if (!ok)
                return false;
            success = dnaSeq->setStop(newStop);
            if (success)
                emit dataChanged(entity, eSequenceColumn);
        }
        break;
    case eDescriptionColumn:
        dnaSeq->setDescription(value.toString());
        break;
    case eNotesColumn:
        dnaSeq->setNotes(value.toString());
        break;

    default:
        return false;
    }

    if (!success)
        return false;

    emit dataChanged(entity, column);
    return true;
}
