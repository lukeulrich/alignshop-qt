/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERPAIRFINDER_H
#define PRIMERPAIRFINDER_H

#include <QtCore>
#include <DnaString.h>
#include <PrimerDesign/PrimerPair.h>
#include <PrimerDesign/PrimerDesignInput.h>

namespace PrimerDesign
{
    /// The result of a primer pair find operation.
    class PrimerPairFinderResult
    {
    public:
        /// Gets whether or not the operation resulted in an error.
        bool isError;

        /// Gets the error message if isError == true.
        QString errorMessage;

        /// Gets the list of found primer pairs, if !isError.
        QList<PrimerPair > value;

        /// Creates and returns an error result.
        /// @param message const QString &
        /// @return PrimerPairFinderResult
        static PrimerPairFinderResult error(const QString &message);

        /// Creates and returns a valid result.
        /// @param value QList<PrimerPair >
        /// @return PrimerPairFinderResult
        static PrimerPairFinderResult valid(QList<PrimerPair > value);
    };

    /// Finds matching primer pairs.
    class PrimerPairFinder
    {
    public:
        /// Constructs a new instance.
        /// @param input PrimerDesignInput *
        PrimerPairFinder(PrimerDesignInput *input);

        /// Finds primer pairs based on the input object provided to the constructor.
        /// @return PrimerPairFinderResult
        PrimerPairFinderResult findPrimerPairs();

        /// Cancels the find operation.
        /// @return void
        void cancel();

    private:
        volatile bool cancelling_;
        PrimerDesignInput input_;
        QString amplicon_;
        QList<Primer > forwardPrimers;
        QList<Primer > reversePrimers;

        /// Performs the actual find operation.
        /// @return PrimerPairFinderResult
        PrimerPairFinderResult performPrimerPairComputation();

        /// Computes forward primers of the specified size.
        /// @param currentPrimerSize int
        /// @return void
        void computeForwardPrimers(int currentPrimerSize);

        /// Computes reverse primers of the specified size.
        /// @param currentPrimerSize int
        /// @return void
        void computeReversePrimers(int currentPrimerSize);

        /// Pairs the forward and reverse primers.
        /// @return PrimerPairFinderResult
        PrimerPairFinderResult getPairedPrimers();

        /// Determines whether or not the specified primer occurs more than once.
        /// @param forwardPrimer const QString &
        /// @return bool
        bool forwardPrimerIsUnique(const QString &forwardPrimer);

        /// Validates the primer meets the input criteria, and stores it into
        /// the specified collection.
        /// @return void
        void validateAndStorePrimer(
            const QString &primerSequence,
            const QString &prefix,
            const QRegExp &suffix,
            int sequencePosition,
            QList<Primer> &collection);
    };
}

#endif // PRIMERPAIRFINDER_H
