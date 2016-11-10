/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QIcon>

#include "DnaMsaColumnAdapter.h"
#include "../../../core/Entities/DnaMsa.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
DnaMsaColumnAdapter::DnaMsaColumnAdapter(QObject *parent)
    : AbstractColumnAdapter(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns int
  */
int DnaMsaColumnAdapter::columnCount() const
{
    return eNumberOfColumns;
}

/**
  * @param column [int]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags DnaMsaColumnAdapter::flags(int column) const
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
QVariant DnaMsaColumnAdapter::data(const IEntitySPtr &entity, int column, int role) const
{
    ASSERT(boost::shared_dynamic_cast<DnaMsa>(entity));
    const DnaMsaSPtr &dnaMsa = boost::shared_static_cast<DnaMsa>(entity);
    ASSERT(dnaMsa);

    switch (role)
    {
    // ----------------------------------------------
    // Display role
    case Qt::DisplayRole:
        switch (column)
        {
        case eIdColumn:
            return dnaMsa->id();
        case eTypeColumn:
            return "Dna alignment";
        case eNameColumn:
            return dnaMsa->name();
        case eDescriptionColumn:
            return dnaMsa->description();
        case eNotesColumn:
            return dnaMsa->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Display role
    case Qt::EditRole:
        switch (column)
        {
        case eNameColumn:
            return dnaMsa->name();
        case eDescriptionColumn:
            return dnaMsa->description();
        case eNotesColumn:
            return dnaMsa->notes();

        default:
            return QVariant();
        }

    // ----------------------------------------------
    // Decoration role
    case Qt::DecorationRole:
        if (column == eNameColumn)
            return QIcon(":/aliases/images/icons/dna-msa");
        return QVariant();

    default:
        return QVariant();
    }
}

/**
  * @param column [int]
  * @returns QVariant
  */
QVariant DnaMsaColumnAdapter::headerData(int column) const
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
bool DnaMsaColumnAdapter::setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
{
    if (!entity)
        return false;

    ASSERT(boost::shared_dynamic_cast<DnaMsa>(entity));
    DnaMsaSPtr dnaMsa = boost::shared_static_cast<DnaMsa>(entity);

    switch (column)
    {
    case eNameColumn:
        dnaMsa->setName(value.toString());
        break;
    case eDescriptionColumn:
        dnaMsa->setDescription(value.toString());
        break;
    case eNotesColumn:
        dnaMsa->setNotes(value.toString());
        break;

    default:
        return false;
    }

    emit dataChanged(entity, column);
    return true;
}