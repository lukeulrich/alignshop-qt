/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "DnaMsa.h"
#include "../enums.h"

const int DnaMsa::kType = eDnaMsaEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
DnaMsa::DnaMsa(int id, const QString &name, const QString &description, const QString &notes)
    : AbstractMsa(id, name, description, notes)
{
}

/**
  * @param abstractMsaPod [const DnaMsaPod &]
  */
DnaMsa::DnaMsa(const DnaMsaPod &abstractMsaPod)
    : AbstractMsa(abstractMsaPod.id_, abstractMsaPod.name_, abstractMsaPod.description_, abstractMsaPod.notes_)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Grammar
  */
Grammar DnaMsa::grammar() const
{
    return eDnaGrammar;
}

/**
  * @returns int
  */
int DnaMsa::type() const
{
    return eDnaMsaEntity;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
DnaMsa *DnaMsa::createEntity(const QString &name, const QString &description, const QString &notes)
{
    return new DnaMsa(::newEntityId<DnaMsa>(), name, description, notes);
}
