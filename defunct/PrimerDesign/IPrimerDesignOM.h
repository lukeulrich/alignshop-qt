/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef IPRIMERDESIGNOMPROVIDER_H
#define IPRIMERDESIGNOMPROVIDER_H

#include <QtCore>
#include <QtGui>
#include <PrimerDesign/PrimerPairGroup.h>

namespace PrimerDesign
{
    /// The primer design object model, global to each running session.
    class PrimerDesignOM : QObject
    {
    public:
        /// The observable primer pair groups data collection.
        ObservablePrimerPairGroupList primerPairGroups;
    };

    /// The primer design object model provider interface.
    class IPrimerDesignOMProvider
    {
    public:
        /// Gets the current primer pairs.
        /// @param widget const QWidget *
        /// @return ObservablePrimerPairList *
        static ObservablePrimerPairList *currentPrimerPairs(const QWidget *widget);

        /// Gets the current sequence.
        /// @param widget const QWidget *
        /// @return ObservableSequence *
        static ObservableSequence *currentSequence(const QWidget *widget);

        /// Gets the observable group list.
        /// @param widget const QWidget *
        /// @return ObservablePrimerPairGroupList &
        static ObservablePrimerPairGroupList &groups(const QWidget *widget);

        /// Gets the primer design object model.
        /// @return PrimerDesignOM*
        virtual PrimerDesignOM* primerDesignOM() = 0;
    };
}

#endif // IPRIMERDESIGNOMPROVIDER_H
