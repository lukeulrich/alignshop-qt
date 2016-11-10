/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractBasicEntity.h"
#include "EntityFlags.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
AbstractBasicEntity::AbstractBasicEntity(int id, const QString &name, const QString &description, const QString &notes)
    : AbstractEntity(id), name_(name), description_(description), notes_(notes)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString AbstractBasicEntity::description() const
{
    return description_;
}

/**
  * @returns QString
  */
QString AbstractBasicEntity::name() const
{
    return name_;
}

/**
  * @returns QString
  */
QString AbstractBasicEntity::notes() const
{
    return notes_;
}

/**
  * @param description [const QString &]
  */
void AbstractBasicEntity::setDescription(const QString &description)
{
    if (description == description_)
        return;

    description_ = description;
    setDirty(Ag::eCoreDataFlag, true);
}

/**
  * @param name [const QString &]
  */
void AbstractBasicEntity::setName(const QString &name)
{
    if (name == name_)
        return;

    name_ = name;
    setDirty(Ag::eCoreDataFlag, true);
}

/**
  * @param notes [const QString &]
  */
void AbstractBasicEntity::setNotes(const QString &notes)
{
    if (notes == notes_)
        return;

    notes_ = notes;
    setDirty(Ag::eCoreDataFlag, true);
}
