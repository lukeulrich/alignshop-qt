/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPAIRNAMER_H
#define PRIMERPAIRNAMER_H

#include <QtCore>
#include "PrimerDesign/PrimerPair.h"

namespace PrimerDesign
{
    /// Names the new primer pairs for a sequence.
    class PrimerPairNamer
    {
    public:
        /// Constructs a new instance.
        /// @param seqName const QString &
        /// @param list ObservablePrimerPairList *
        PrimerPairNamer(const QString &seqName, ObservablePrimerPairList *list);

        /// Generates the next name.
        /// @return QString
        QString nextName();

    private:
        QSet<QString> names_;
        QString prefix_;
        int suffix_;
    };
}

#endif // PRIMERPAIRNAMER_H
