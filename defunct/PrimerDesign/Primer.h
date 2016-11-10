/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef PRIMER_H
#define PRIMER_H

#include <QtCore>
#include "PrimerDesign/StrictDnaString.h"
#include "PrimerDesign/PrimerDesignInput.h"

namespace PrimerDesign
{
    /// Represents a single primer.
    class Primer
    {
    public:
        static const double DEFAULT_ENTROPY;
        static const double DEFAULT_ENTHALPY;
        static const double DEFAULT_HOMO_SCORE;

        // ------------------------------------------------------------------------------------------------
        // Constructors
        /// Constructs a primer object.
        Primer();

        /// Constructs a primer object.
        /// @param sequence const QString &
        Primer(const QString &sequence, const PrimerDesignInput &params);

        // ------------------------------------------------------------------------------------------------
        // Properties
        /// Gets the sequence.
        /// @return QString
        QString sequence() const;
        /// Sets the sequence
        /// @param sequence const QString &
        /// @return bool
        bool setSequence(const QString &sequence);

        /// Gets the tm
        /// @return double
        double tm() const;
        /// Sets the tm
        /// @param tm double
        /// @return bool
        bool setTm(double tm);

        /// Gets the entropy
        /// @return double
        double entropy() const;
        /// Sets the entropy
        /// @param entropy double
        /// @return bool
        bool setEntropy(double entropy);

        /// Gets the enthalpy
        /// @return double
        double enthalpy() const;
        /// Sets the enthalpy
        /// @param enthalpy double
        /// @return bool
        bool setEnthalpy(double enthalpy);

        /// Gets the homodimer score
        /// @return int
        int homoDimerScore();
        /// Sets the homodimer score
        /// @param homoDimerScore int
        /// @return bool
        bool setHomoDimerScore(int homoDimerScore);

        /// Gets the sequence position
        /// @return int
        int sequencePosition() const;
        /// Sets the sequence position
        /// @param position int
        /// @return bool
        bool setSequencePosition(int position);

        // ------------------------------------------------------------------------------------------------
    private:
        StrictDnaString sequence_;
        double tm_;
        double entropy_;
        double enthalpy_;
        int homoDimerScore_;
        int sequencePosition_;
    };
}

#endif // PRIMER_H
