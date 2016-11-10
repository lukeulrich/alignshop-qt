/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "KalignConstants.h"

#include <QtCore/QString>
#include "../PODs/OptionSpec.h"
#include "../util/OptionProfile.h"

namespace constants
{
    namespace Kalign
    {
        const QString kFriendlyKalignName               = "Kalign";

        // Options
        const QString kGapOpenOpt                       = "-gpo";
        const QString kGapExtensionOpt                  = "-gpe";
        const QString kTerminalGapExtensionPenaltyOpt   = "-tgpe";
        const QString kMatrixBonusOpt                   = "-bonus";
        const QString kSortOpt                          = "-sort";
        const QString kFeatureOpt                       = "-feature";
        const QString kDistanceOpt                      = "-distance";
        const QString kGuideTreeOpt                     = "-tree";
        const QString kZCutOffOpt                       = "-zcutoff";
        const QString kInFileOpt                        = "-infile";
        const QString kOutFileOpt                       = "-outfile";
        const QString kFormatOpt                        = "-format";
        const QString kQuietOpt                         = "-quiet";

        // Allowed distance methods
        const QString kDistanceWu                       = "wu";
        const QString kDistancePair                     = "pair";

        // Allowed guide tree methods
        const QString kGuideTreeNJ                      = "nj";
        const QString kGuideTreeUPGMA                   = "upgma";

        // Allowed sort variables
        const QString kSortInput                        = "input";
        const QString kSortTree                         = "tree";
        const QString kSortGaps                         = "gaps";

        // Allowed output formats
        const QString kFormatFasta                      = "fasta";
        const QString kFormatMsf                        = "msf";
        const QString kFormatAln                        = "aln";
        const QString kFormatClu                        = "clu";
        const QString kFormatMacsim                     = "macsim";

        OptionProfile initializeKalignOptionProfile()
        {
            bool notRequired = false;
            bool noEmptyValue = false;
            bool notDefault = false;
            bool isDefault = true;

            OptionProfile profile;

            profile << OptionSpec(kGapOpenOpt,                      notRequired, noEmptyValue);
            profile << OptionSpec(kGapExtensionOpt,                 notRequired, noEmptyValue);
            profile << OptionSpec(kTerminalGapExtensionPenaltyOpt,  notRequired, noEmptyValue);
            profile << OptionSpec(kMatrixBonusOpt,                  notRequired, noEmptyValue);
            profile << OptionSpec(kSortOpt,                         notRequired, noEmptyValue, isDefault, kSortInput, QRegExp("^input|tree|gaps$"));
            profile << OptionSpec(kFeatureOpt,                      notRequired, noEmptyValue);
            profile << OptionSpec(kDistanceOpt,                     notRequired, noEmptyValue, notDefault, QString(), QRegExp("^wu|pair$"));
            profile << OptionSpec(kGuideTreeOpt,                    notRequired, noEmptyValue, notDefault, QString(), QRegExp("^nj|upgma$"));
            profile << OptionSpec(kZCutOffOpt,                      notRequired, noEmptyValue);
            profile << OptionSpec(kInFileOpt,                       notRequired, noEmptyValue);
            profile << OptionSpec(kOutFileOpt,                      notRequired, noEmptyValue);
            profile << OptionSpec(kFormatOpt,                       notRequired, noEmptyValue, isDefault, kFormatFasta, QRegExp("^fasta|msf|aln|clu|macsim$"));
            profile << OptionSpec(kQuietOpt);

            return profile;
        }

        const OptionProfile kKalignOptionProfile(initializeKalignOptionProfile());

        // Default values (see kalign2_misc.c, line 419 and following)
        // For some reason, any user-defined gap open is multiplied by 5 and the gap extend and terminal penalties
        // multiplied by 10. Therefore, make the defaults match these inputs.
        const double kDefaultDnaGapOpenPenalty = (43.4 * 5.) / 5.;
        const double kDefaultDnaGapExtendPenalty = 39.4 / 10.;
        const double kDefaultDnaTerminalGapPenalty = 292.6 / 10.;

        const double kDefaultAminoGapOpenPenalty = 54.94941 / 5.;
        const double kDefaultAminoGapExtendPenalty = 8.52492 / 10.;
        const double kDefaultAminoTerminalGapPenalty = 4.42410 / 10.;
    }
}
