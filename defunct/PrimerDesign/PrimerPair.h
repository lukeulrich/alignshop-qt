/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#ifndef PRIMERPAIR_H
#define PRIMERPAIR_H

#include <QtCore>
#include "PrimerDesign/Primer.h"
#include <PrimerDesign/PrimerDesignInput.h>
#include "ObservableList.h"

namespace PrimerDesign
{
    /**
      * Represents a pair of primers with a hetero score.
      */
    class PrimerPair
    {
    public:
        // ------------------------------------------------------------------------------------------------
        // Constructors
        /// @param params const PrimerDesignInputRef &
        PrimerPair(const PrimerDesignInputRef &params);   //!< Constructs a primer pair
        ~PrimerPair();

        // ------------------------------------------------------------------------------------------------
        // Properties
        /// Gets the params used to create this pair.
        /// @return PrimerDesignInput *
        PrimerDesignInput * params() const;

        /// Gets the forward primer.
        /// @return Primer &
        Primer &forwardPrimer();
        /// Sets the forward primer.
        /// @param forward const Primer &
        /// @return bool
        bool setForwardPrimer(const Primer &forward);

        /// Gets the name.
        /// @return QString
        QString name() const;
        /// Sets the name.
        /// @param name const QString &
        /// @return bool
        bool setName(const QString &name);

        /// Gets the reverse primer.
        /// @return Primer &
        Primer &reversePrimer();
        /// Sets the reverse primer.
        /// @param reverse const Primer &
        /// @return bool
        bool setReversePrimer(const Primer &reverse);

        /// Gets the score.
        /// @return double
        double score() const;
        /// Sets the score.
        /// @param value double
        /// @return bool
        bool setScore(double value);

        /// Gets the amplicon length.
        /// @return int
        int ampliconLength() const;

        /// Recomputes the score.
        /// @return void
        void recomputePrimerPairScore();
        // ------------------------------------------------------------------------------------------------
    private:
        PrimerDesignInputRef params_;
        QString name_;
        Primer forward_;
        Primer reverse_;
        double score_;
    };

    /// An observable list of PrimerPair.
    typedef ObservableList<PrimerPair> ObservablePrimerPairList;
}

#endif // PRIMERPAIR_H
