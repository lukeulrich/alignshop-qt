/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QIcon>

#include "AminoMsaColumnAdapter.h"
#include "../../../core/Entities/AminoMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
AminoMsaColumnAdapter::AminoMsaColumnAdapter(QObject *parent)
    : AbstractColumnAdapter(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int AminoMsaColumnAdapter::columnCount() const
{
    return eNumberOfColumns;
}

/**
  * @param column [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags AminoMsaColumnAdapter::flags(int column) const
{
    Qt::ItemFlags defaultFlags = AbstractColumnAdapter::flags(column);

    switch (column)
    {
    case eNameColumn:
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
QVariant AminoMsaColumnAdapter::data(const IEntitySPtr &entity, int column, int role) const
{
    ASSERT(boost::shared_dynamic_cast<const AminoMsa>(entity));
    const AminoMsaSPtr &aminoMsa = boost::shared_static_cast<AminoMsa>(entity);
    ASSERT(aminoMsa);

    switch (role)
    {
    // ----------------------------------------------
    // Display role
    case Qt::DisplayRole:
        switch (column)
        {
        case eIdColumn:
            return aminoMsa->id();
        case eTypeColumn:
            return "Amino alignment";
        case eNameColumn:
            return aminoMsa->name();
        case eDescriptionColumn:
            return aminoMsa->description();
        case eNotesColumn:
            return aminoMsa->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Edit role
    case Qt::EditRole:
        switch (column)
        {
        case eNameColumn:
            return aminoMsa->name();
        case eDescriptionColumn:
            return aminoMsa->description();
        case eNotesColumn:
            return aminoMsa->notes();

        default:
            return QVariant();
        }


    // ----------------------------------------------
    // Decoration role
    case Qt::DecorationRole:
        if (column == eNameColumn)
            return QIcon(":/aliases/images/icons/amino-msa");
        return QVariant();

    default:
        return QVariant();
    }
}

/**
  * @param column [int]
  * @returns QVariant
  */
QVariant AminoMsaColumnAdapter::headerData(int column) const
{
    switch (column)
    {
    case eIdColumn:
        return "ID";
    case eTypeColumn:
        return "Type";
    case eNameColumn:
        return "Name";
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
bool AminoMsaColumnAdapter::setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
{
    if (!entity)
        return false;

    ASSERT(boost::shared_dynamic_cast<const AminoMsa>(entity));
    AminoMsaSPtr aminoMsa = boost::shared_static_cast<AminoMsa>(entity);

    switch (column)
    {
    case eNameColumn:
        aminoMsa->setName(value.toString());
        break;
    case eDescriptionColumn:
        aminoMsa->setDescription(value.toString());
        break;
    case eNotesColumn:
        aminoMsa->setNotes(value.toString());
        break;

    default:
        return false;
    }

    emit dataChanged(entity, column);
    return true;
}
