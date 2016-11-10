/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef KALIGNCONSTANTS_H
#define KALIGNCONSTANTS_H

class QString;

class OptionProfile;

namespace constants
{
    namespace Kalign
    {
        extern const QString kFriendlyKalignName;

        extern const QString kGapOpenOpt;
        extern const QString kGapExtensionOpt;
        extern const QString kTerminalGapExtensionPenaltyOpt;
        extern const QString kMatrixBonusOpt;
        extern const QString kSortOpt;
        extern const QString kFeatureOpt;
        extern const QString kDistanceOpt;
        extern const QString kGuideTreeOpt;
        extern const QString kZCutOffOpt;
        extern const QString kInFileOpt;
        extern const QString kOutFileOpt;
        extern const QString kFormatOpt;
        extern const QString kQuietOpt;

        // Allowed distance methods
        extern const QString kDistanceWu;
        extern const QString kDistancePair;

        // Allowed guide tree methods
        extern const QString kGuideTreeNJ;
        extern const QString kGuideTreeUPGMA;

        // Allowed sort variables
        extern const QString kSortInput;
        extern const QString kSortTree;
        extern const QString kSortGaps;

        // Allowed output formats
        extern const QString kFormatFasta;
        extern const QString kFormatMsf;
        extern const QString kFormatAln;
        extern const QString kFormatClu;
        extern const QString kFormatMacsim;

        extern const OptionProfile kKalignOptionProfile;

        // Default values
        extern const double kDefaultDnaGapOpenPenalty;
        extern const double kDefaultDnaGapExtendPenalty;
        extern const double kDefaultDnaTerminalGapPenalty;

        extern const double kDefaultAminoGapOpenPenalty;
        extern const double kDefaultAminoGapExtendPenalty;
        extern const double kDefaultAminoTerminalGapPenalty;
    }
}

#endif // KALIGNCONSTANTS_H
