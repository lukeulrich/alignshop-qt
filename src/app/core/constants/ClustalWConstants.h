/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALWCONSTANTS_H
#define CLUSTALWCONSTANTS_H

class QString;

class OptionProfile;

namespace constants
{
    namespace ClustalW
    {
        extern const QString kFriendlyClustalWName;

        extern const QString kOutOrderOpt;

        // Tree parameter options
        extern const QString kGuideTreeOpt;
        extern const QString kTreeExcludeGapPositionsOpt;
        extern const QString kTreeCorrectMultipleSubstitionsOpt;

        // Fast pairwise alignment options
        extern const QString kQuickTreeOpt;
        extern const QString kFastWordSizeOpt;
        extern const QString kFastBestDiagonalsOpt;
        extern const QString kFastWindowBestDiagonalsOpt;
        extern const QString kFastGapPenaltyOpt;
        extern const QString kFastScoreOpt;

        // Slow pairwise alignment options
        extern const QString kSlowProteinWeightMatrixOpt;
        extern const QString kSlowDnaWeightMatrixOpt;
        extern const QString kSlowGapOpenPenaltyOpt;
        extern const QString kSlowGapExtensionPenaltyOpt;

        // Multiple alignment options
        extern const QString kMsaGapOpenPenalty;
        extern const QString kMsaGapExtensionPenalty;
        extern const QString kMsaPercentIdentityForDelay;
        extern const QString kMsaDnaMatrix;
        extern const QString kMsaDnaTransitionsWeighting;
        extern const QString kMsaProteinMatrix;
        extern const QString kMsaProteinNegativeInMatrix;
        extern const QString kMsaIteration;
        extern const QString kMsaNumberofIterations;
        extern const QString kMsaDisableSequenceWeighting;

        extern const QString kInFileOpt;
        extern const QString kFormatOpt;
        extern const QString kOutFileOpt;

        // Allowed order options
        extern const QString kOrderByInput;
        extern const QString kOrderByAligned;

        // Allowed guide tree options
        extern const QString kGuideTreeNJ;
        extern const QString kGuideTreeUPGMA;

        // Allowed fast score option values
        extern const QString kFastScorePercent;
        extern const QString kFastScoreAbsolute;

        // Allowed iteration values
        extern const QString kIterationNone;
        extern const QString kIterationTree;
        extern const QString kIterationAlignment;

        // Allowed weight matrices
        extern const QString kProteinWeightMatrixBlosum;
        extern const QString kProteinWeightMatrixPam;
        extern const QString kProteinWeightMatrixGonnet;
        extern const QString kProteinWeightMatrixId;
        extern const QString kDnaWeightMatrixIub;
        extern const QString kDnaWeightMatrixClustalw;

        // Allowed output formats
        extern const QString kFormatClustal;
        extern const QString kFormatGCG;
        extern const QString kFormatGDE;
        extern const QString kFormatPhylip;
        extern const QString kFormatPIR;
        extern const QString kFormatNexus;
        extern const QString kFormatFasta;

        extern const OptionProfile kClustalWOptionProfile;
    }
}

#endif // CLUSTALWCONSTANTS_H
