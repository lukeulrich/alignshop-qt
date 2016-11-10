/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERMUTATOR_H
#define PRIMERMUTATOR_H

#include "IPrimerMutator.h"

class PrimerMutator : public IPrimerMutator
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerMutator(QObject *parent = 0);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual bool setPrimerName(Primer &primer, const QString &newName);
};

#endif // PRIMERMUTATOR_H
