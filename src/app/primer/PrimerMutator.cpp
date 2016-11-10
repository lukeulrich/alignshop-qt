/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerMutator.h"
#include "Primer.h"
#include "../core/Entities/EntityFlags.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
PrimerMutator::PrimerMutator(QObject *parent)
    : IPrimerMutator(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
bool PrimerMutator::setPrimerName(Primer &primer, const QString &newName)
{
    if (newName != primer.name())
    {
        primer.setDirty(Ag::eCoreDataFlag, true);
        primer.setName(newName);
        emit primerNameChanged(primer.id());
    }

    return true;
}
