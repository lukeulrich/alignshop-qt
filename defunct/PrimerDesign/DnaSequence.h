/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef DNASEQ_H
#define DNASEQ_H

#include <QtCore>
#include "StrictDnaString.h"
#include "PrimerDesign/PrimerPair.h"

namespace PrimerDesign
{
    /**
      * Represents a full, named DNA sequence.
      */
    class DnaSequence
    {
    public:
        // ------------------------------------------------------------------------------------------------
        // Constructors
        /// Constructs a DnaSeq
        DnaSequence();

        // ------------------------------------------------------------------------------------------------
        // Properties
        /// Gets the name of the sequence
        /// @return QString
        QString name() const;

        /// Sets the name of the sequence
        /// @param name const QString &
        /// @return bool
        virtual bool setName(const QString &name);

        /// Gets the notes
        /// @return QString
        QString notes() const;

        /// Sets the notes
        /// @param notes const QString &
        /// @return bool
        virtual bool setNotes(const QString &notes);

        /// Gets the primers
        /// @return QList<PrimerPair>
        QList<PrimerPair> primers() const;

        /// Gets the sequence
        /// @return QString
        QString sequence() const;

        /// Sets the sequence
        /// @param sequence const QString &
        /// @return bool
        virtual bool setSequence(const QString &sequence);

        // ------------------------------------------------------------------------------------------------
    private:
        QString name_;
        QString notes_;
        QList<PrimerPair> primers_;
        StrictDnaString sequence_;
    };
}

#endif // DNASEQ_H
