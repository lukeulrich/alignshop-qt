/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERDESIGNFILE_H
#define PRIMERDESIGNFILE_H

#include <QtCore>
#include <PrimerDesign/PrimerPairGroup.h>
#include <PrimerDesign/PrimerDesignInput.h>

namespace PrimerDesign
{
    /// Responsible for serializing and deserializing a primer design session.
    class PrimerDesignFile
    {
    public:
        /// Desrializes a session from a string.
        /// @param list ObservablePrimerPairGroupList &
        /// @param data const QString &
        /// @return void
        static void deserialize(ObservablePrimerPairGroupList &list, const QString &data);

        /// Serializes a session to a string.
        /// @param data ObservablePrimerPairGroupList &
        /// @return QString
        static QString serialize(ObservablePrimerPairGroupList &data);
    };
}

#endif // PRIMERDESIGNFILE_H
