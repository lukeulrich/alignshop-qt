/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPAIRGROUP_H
#define PRIMERPAIRGROUP_H

#include "PrimerDesign/ObservableSequence.h"
#include "PrimerDesign/PrimerPair.h"

namespace PrimerDesign
{
    /// Represents a group of primer pair results.
    class PrimerPairGroup
    {
    public:
        /// The primer pairs.
        ObservablePrimerPairList pairs;

        /// The sequence to which the primer pairs pertain.
        ObservableSequence sequence;
    };

    /// An observable list of PrimerPairGroup objects.
    typedef ObservableList<PrimerPairGroup> ObservablePrimerPairGroupList;
}

#endif // PRIMERPAIRGROUP_H
