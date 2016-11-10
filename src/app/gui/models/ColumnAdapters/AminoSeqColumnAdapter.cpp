/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>
#include <QtGui/QIcon>

#include "AminoSeqColumnAdapter.h"
#include "../CustomRoles.h"
#include "../../../core/Entities/AminoSeq.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
AminoSeqColumnAdapter::AminoSeqColumnAdapter(QObject *parent)
    : AbstractColumnAdapter(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int AminoSeqColumnAdapter::columnCount() const
{
    return eNumberOfColumns;
}

/**
  * @param column [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags AminoSeqColumnAdapter::flags(int column) const
{
    Qt::ItemFlags defaultFlags = AbstractColumnAdapter::flags(column);

    switch (column)
    {
    case eNameColumn:
    case eSourceColumn:
    case eStartColumn:
    case eStopColumn:
    case eDescriptionColumn:
    case eNotesColumn:
        defaultFlags |= Qt::ItemIsEditable;

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
QVariant AminoSeqColumnAdapter::data(const IEntitySPtr &entity, int column, int role) const
{
    ASSERT(boost::shared_dynamic_cast<const AminoSeq>(entity));
    const AminoSeqSPtr &aminoSeq = boost::shared_static_cast<AminoSeq>(entity);
    ASSERT(aminoSeq);

    switch (role)
    {
    // ----------------------------------------------
    // Display role
    case Qt::DisplayRole:
        switch (column)
        {
        case eIdColumn:
            return aminoSeq->id();
        case eTypeColumn:
            return "Amino";
        case eNameColumn:
            return aminoSeq->name();
        case eSourceColumn:
            return aminoSeq->source();
        case eStartColumn:
            return aminoSeq->start();
        case eStopColumn:
            return aminoSeq->stop();
        case eLengthColumn:
            return aminoSeq->length();
        case eDescriptionColumn:
            return aminoSeq->description();
        case eNotesColumn:
            return aminoSeq->notes();
        case eSequenceColumn:
            return aminoSeq->bioString().asByteArray();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Edit role
    case Qt::EditRole:
        switch (column)
        {
        case eNameColumn:
            return aminoSeq->name();
        case eSourceColumn:
            return aminoSeq->source();
        case eStartColumn:
            return aminoSeq->start();
        case eStopColumn:
            return aminoSeq->stop();
        case eDescriptionColumn:
            return aminoSeq->description();
        case eNotesColumn:
            return aminoSeq->notes();

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
            return aminoSeq->start();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Max role
    case CustomRoles::kMaxRole:
        switch (column)
        {
        case eStartColumn:
            return aminoSeq->stop();
        case eStopColumn:
            return aminoSeq->seqLength();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Decoration role
    case Qt::DecorationRole:
        if (column == eNameColumn)
            return QIcon(":/aliases/images/icons/amino-seq");
        return QVariant();

    default:
        return QVariant();
    }
}

/**
  * @param column [int]
  * @returns QVariant
  */
QVariant AminoSeqColumnAdapter::headerData(int column) const
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
bool AminoSeqColumnAdapter::setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
{
    if (!entity)
        return false;

    ASSERT(boost::shared_dynamic_cast<const AminoSeq>(entity));
    AminoSeqSPtr aminoSeq = boost::shared_static_cast<AminoSeq>(entity);
    ASSERT(aminoSeq);

    bool success = true;
    switch (column)
    {
    case eNameColumn:
        aminoSeq->setName(value.toString());
        break;
    case eSourceColumn:
        aminoSeq->setSource(value.toString());
        break;
    case eStartColumn:
        {
            bool ok = false;
            int newStart = value.toInt(&ok);
            if (!ok)
                return false;
            success = aminoSeq->setStart(newStart);
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
            success = aminoSeq->setStop(newStop);
            if (success)
                emit dataChanged(entity, eSequenceColumn);
        }
        break;
    case eDescriptionColumn:
        aminoSeq->setDescription(value.toString());
        break;
    case eNotesColumn:
        aminoSeq->setNotes(value.toString());
        break;

    default:
        return false;
    }

    if (!success)
        return false;

    emit dataChanged(entity, column);
    return true;
}
