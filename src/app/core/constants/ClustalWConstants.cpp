/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ClustalWConstants.h"

#include <QtCore/QString>
#include "../PODs/OptionSpec.h"
#include "../util/OptionProfile.h"

namespace constants
{
    namespace ClustalW
    {
        const QString kFriendlyClustalWName               = "ClustalW";

        const QString kOutOrderOpt                        = "-OUTORDER";

        // Tree parameter options
        const QString kGuideTreeOpt                       = "-CLUSTERING";
        const QString kTreeExcludeGapPositionsOpt         = "-TOSSGAPS";
        const QString kTreeCorrectMultipleSubstitionsOpt  = "-KIMURA";

        // Fast pairwise alignment options
        const QString kQuickTreeOpt                       = "-QUICKTREE";
        const QString kFastWordSizeOpt                    = "-KTUPLE";
        const QString kFastBestDiagonalsOpt               = "-TOPDIAGS";
        const QString kFastWindowBestDiagonalsOpt         = "-WINDOW";
        const QString kFastGapPenaltyOpt                  = "-PAIRGAP";
        const QString kFastScoreOpt                       = "-SCORE";

        // Slow pairwise alignment options
        const QString kSlowProteinWeightMatrixOpt         = "-PWMATRIX";
        const QString kSlowDnaWeightMatrixOpt             = "-PWDNAMATRIX";
        const QString kSlowGapOpenPenaltyOpt              = "-PWGAPOPEN";
        const QString kSlowGapExtensionPenaltyOpt         = "-PWGAPEXT";

        // Multiple alignment options
        const QString kMsaGapOpenPenalty                  = "-GAPOPEN";
        const QString kMsaGapExtensionPenalty             = "-GAPEXT";
        const QString kMsaPercentIdentityForDelay         = "-MAXDIV";
        const QString kMsaDnaMatrix                       = "-DNAMATRIX";
        const QString kMsaDnaTransitionsWeighting         = "-TRANSWEIGHT";
        const QString kMsaProteinMatrix                   = "-MATRIX";
        const QString kMsaProteinNegativeInMatrix         = "-NEGATIVE";
        const QString kMsaIteration                       = "-ITERATION";
        const QString kMsaNumberofIterations              = "-NUMITER";
        const QString kMsaDisableSequenceWeighting        = "-NOWEIGHTS";

        const QString kInFileOpt                          = "-INFILE";
        const QString kFormatOpt                          = "-OUTPUT";
        const QString kOutFileOpt                         = "-OUTFILE";

        const QString kFormatClustal                      = "CLUSTAL";
        const QString kFormatGCG                          = "GCG";
        const QString kFormatGDE                          = "GDE";
        const QString kFormatPhylip                       = "PHYLIP";
        const QString kFormatPIR                          = "PIR";
        const QString kFormatNexus                        = "NEXUS";
        const QString kFormatFasta                        = "FASTA";

        // Allowed order options
        const QString kOrderByInput                       = "INPUT";
        const QString kOrderByAligned                     = "ALIGNED";

        // Allowed guide tree options
        const QString kGuideTreeNJ                        = "NJ";
        const QString kGuideTreeUPGMA                     = "UPGMA";

        // Allowed fast score option values
        const QString kFastScorePercent                   = "PERCENT";
        const QString kFastScoreAbsolute                  = "ABSOLUTE";

        // Allowed iteration values
        const QString kIterationNone                      = "NONE";
        const QString kIterationTree                      = "TREE";
        const QString kIterationAlignment                 = "ALIGNMENT";

        // Allowed weight matrices
        const QString kProteinWeightMatrixBlosum          = "BLOSUM";
        const QString kProteinWeightMatrixPam             = "PAM";
        const QString kProteinWeightMatrixGonnet          = "GONNET";
        const QString kProteinWeightMatrixId              = "ID";
        const QString kDnaWeightMatrixIub                 = "IUB";
        const QString kDnaWeightMatrixClustalw            = "CLUSTALW";

        OptionProfile initializeClustalWOptionProfile()
        {
            bool notRequired = false;
            bool noEmptyValue = false;
            bool emptyValue = true;
            bool notDefault = false;

            QRegExp positiveIntegerRegexp("^[1-9]\\d*$");
            QRegExp zeroOrPositiveIntegerRegexp("^0|[1-9]\\d*$");
            QRegExp floatingPointRegExp("^[-+]?[0-9]*\\.?[0-9]+$");
            QRegExp zeroTo100RegExp("^0|100|[1-9]\\d?$");
            QRegExp proteinWeightMatricesRegExp("^BLOSUM|PAM|GONNET|ID$");
            QRegExp dnaWeightMatricesRegExp("^IUB|CLUSTALW$");

            OptionProfile profile;

            profile.setJoinEnabled();
            profile.setJoinDelimiter("=");  // ClustalW requires all option name values to be joined with an equal sign

            profile << OptionSpec(kOutOrderOpt,     notRequired, noEmptyValue, notDefault, QString("ALIGNED"), QRegExp("^INPUT|ALIGNED$"));

            // Tree parameter options
            profile << OptionSpec(kGuideTreeOpt,                notRequired, noEmptyValue, notDefault, "NJ",        QRegExp("^NJ|UPGMA$"));
            profile << OptionSpec(kTreeExcludeGapPositionsOpt,  notRequired, emptyValue);
            profile << OptionSpec(kTreeCorrectMultipleSubstitionsOpt, notRequired, emptyValue);

            // Fast pairwise alignment options
            profile << OptionSpec(kQuickTreeOpt,    notRequired, emptyValue);
            profile << OptionSpec(kFastWordSizeOpt,             notRequired, noEmptyValue, notDefault, "1",         positiveIntegerRegexp);
            profile << OptionSpec(kFastBestDiagonalsOpt,        notRequired, noEmptyValue, notDefault, "5",         positiveIntegerRegexp);
            profile << OptionSpec(kFastWindowBestDiagonalsOpt,  notRequired, noEmptyValue, notDefault, "5",         zeroOrPositiveIntegerRegexp);
            profile << OptionSpec(kFastGapPenaltyOpt,           notRequired, noEmptyValue, notDefault, "3",         zeroOrPositiveIntegerRegexp);
            profile << OptionSpec(kFastScoreOpt,                notRequired, noEmptyValue, notDefault, "PERCENT",   QRegExp("^PERCENT|ABSOLUTE$"));

            // Slow pairwise alignment options
            profile << OptionSpec(kSlowProteinWeightMatrixOpt,  notRequired, noEmptyValue, notDefault, "GONNET",    proteinWeightMatricesRegExp);
            profile << OptionSpec(kSlowDnaWeightMatrixOpt,      notRequired, noEmptyValue, notDefault, "IUB",       dnaWeightMatricesRegExp);
            profile << OptionSpec(kSlowGapOpenPenaltyOpt,       notRequired, noEmptyValue, notDefault, "10",        floatingPointRegExp);
            profile << OptionSpec(kSlowGapExtensionPenaltyOpt,  notRequired, noEmptyValue, notDefault, ".1",        floatingPointRegExp);

            // Multiple alignment options
            profile << OptionSpec(kMsaGapOpenPenalty,           notRequired, noEmptyValue, notDefault, "10",        zeroOrPositiveIntegerRegexp);
            profile << OptionSpec(kMsaGapExtensionPenalty,      notRequired, noEmptyValue, notDefault, ".2",        floatingPointRegExp);
            profile << OptionSpec(kMsaPercentIdentityForDelay,  notRequired, noEmptyValue, notDefault, "30",        zeroTo100RegExp);
            profile << OptionSpec(kMsaDnaMatrix,                notRequired, noEmptyValue, notDefault, "IUB",       dnaWeightMatricesRegExp);
            profile << OptionSpec(kMsaDnaTransitionsWeighting,  notRequired, noEmptyValue, notDefault, ".5",        floatingPointRegExp);
            profile << OptionSpec(kMsaProteinMatrix,            notRequired, noEmptyValue, notDefault, "GONNET",    proteinWeightMatricesRegExp);
            profile << OptionSpec(kMsaProteinNegativeInMatrix,  notRequired, emptyValue);
            profile << OptionSpec(kMsaIteration,                notRequired, noEmptyValue, notDefault, "NONE",      QRegExp("^NONE|TREE|ALIGNMENT$"));
            profile << OptionSpec(kMsaNumberofIterations,       notRequired, noEmptyValue, notDefault, "3",         zeroOrPositiveIntegerRegexp);
            profile << OptionSpec(kMsaDisableSequenceWeighting, notRequired, emptyValue);

            // Technically, infile is required
            profile << OptionSpec(kInFileOpt,       notRequired, noEmptyValue);
            profile << OptionSpec(kFormatOpt,       notRequired, noEmptyValue, notDefault, QString("CLUSTAL"), QRegExp("^CLUSTAL|GCG|GDE|PHYLIP|PIR|NEXUS|FASTA$"));
            profile << OptionSpec(kOutFileOpt,      notRequired, noEmptyValue);

            return profile;
        }
        const OptionProfile kClustalWOptionProfile(initializeClustalWOptionProfile());
    }
}
