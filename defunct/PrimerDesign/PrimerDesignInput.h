/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef PRIMERDESIGNINPUT_H
#define PRIMERDESIGNINPUT_H

#include <QtCore>
#include <PrimerDesign/Range.h>
#include <PrimerDesign/PrimerDesignInput.h>

namespace PrimerDesign
{
    /// Represents the input from the primer design wizard.
    class PrimerDesignInput
    {
    public:
        /// The amplicon.
        QString amplicon;

        /// The bounds of the amplicon sub-string being searched.
        Range ampliconBounds;

        /// The acceptable range of resulting amplicons.
        Range ampliconSizeRange;

        /// The foward prefix, restriction enzyme.
        QString forwardPrefix;

        /// The forward suffix, or restriction site.
        QString forwardSuffix;

        /// The primer size range.
        Range primerSizeRange;

        /// The reverse prefix, restriction enzyme.
        QString reversePrefix;

        /// The reverse suffix, or restriction site.
        QString reverseSuffix;

        /// The acceptable tm range.
        RangeF tmRange;

        /// The sodium concentration in molars.
        float sodiumConcentration;

        /// Constructs a new object instance.
        /// @param amplicon const QString & (Defaults to "".)
        PrimerDesignInput(const QString &amplicon = "");

        /// Gets the amplicon region specified by amplicon bounds.
        /// @return QString
        QString getBoundedAmplicon() const;

        /// Gets the value indicating whether or not the input is valid.
        /// @return bool
        bool isValid() const;

        /// Validates the input and returns the error message, or empty if none exists.
        /// @return QString
        QString getErrorMessage() const;

        /// Gets an instance where the ranges use zero-based indices.
        /// @return PrimerDesignInput
        PrimerDesignInput toZeroBased() const;

        /// Gets an instance where the ranges use one-based indices.
        /// @return PrimerDesignInput
        PrimerDesignInput toOneBased() const;
    };

    typedef QSharedPointer<PrimerDesignInput> PrimerDesignInputRef;
}

#endif // PRIMERDESIGNINPUT_H
