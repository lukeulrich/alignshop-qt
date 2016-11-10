/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoMsa.h"
#include "../enums.h"

const int AminoMsa::kType = eAminoMsaEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
AminoMsa::AminoMsa(int id, const QString &name, const QString &description, const QString &notes)
    : AbstractMsa(id, name, description, notes)
{
}

/**
  * @param abstractMsaPod [const AminoMsaPod &]
  */
AminoMsa::AminoMsa(const AminoMsaPod &abstractMsaPod)
    : AbstractMsa(abstractMsaPod.id_, abstractMsaPod.name_, abstractMsaPod.description_, abstractMsaPod.notes_)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Grammar
  */
Grammar AminoMsa::grammar() const
{
    return eAminoGrammar;
}

/**
  * @returns int
  */
int AminoMsa::type() const
{
    return eAminoMsaEntity;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
AminoMsa *AminoMsa::createEntity(const QString &name, const QString &description, const QString &notes)
{
    return new AminoMsa(::newEntityId<AminoMsa>(), name, description, notes);
}
