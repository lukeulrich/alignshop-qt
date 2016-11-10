/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "PsiBlastConstants.h"
#include "../PODs/OptionSpec.h"
#include "../util/OptionProfile.h"

namespace constants
{
    namespace PsiBlast
    {
        const QString kFriendlyPsiBlastName     = "PSI-BLAST";

        const QString kAgPssmFileOpt            = "-out_ag_pssm";
        const QString kNumAlignmentsOpt         = "-num_alignments";
        const QString kDatabaseOpt              = "-db";
        const QString kDatabaseSizeOpt          = "-dbsize";
        const QString kNumDescriptionsOpt       = "-num_descriptions";
        const QString kEvalueOpt                = "-evalue";
        const QString kInclusionEThreshOpt      = "-inclusion_ethresh";
        const QString kIterationsOpt            = "-num_iterations";
        const QString kMatrixOpt                = "-matrix";
        const QString kMaxTargetSeqsOpt         = "-max_target_seqs";
        const QString kOutputFormatOpt          = "-outfmt";
        const QString kSearchSpaceOpt           = "-searchsp";
        const QString kThreadsOpt               = "-num_threads";

        const QString kBLOSUM45                 = "BLOSUM45";
        const QString kBLOSUM50                 = "BLOSUM50";
        const QString kBLOSUM62                 = "BLOSUM62";
        const QString kBLOSUM80                 = "BLOSUM80";
        const QString kBLOSUM90                 = "BLOSUM90";
        const QString kPAM30                    = "PAM30";
        const QString kPAM70                    = "PAM70";
        const QString kPAM250                   = "PAM250";

        const QStringList kMatrixList(QStringList()
                                      << kBLOSUM45 << kBLOSUM50 << kBLOSUM62 << kBLOSUM80 << kBLOSUM90
                                      << kPAM30 << kPAM70 << kPAM250);

        OptionProfile initializePsiBlastOptionProfile()
        {
            OptionProfile profile;

            bool required = true;
            bool notRequired = false;
            bool noEmptyValue = false;
            bool notDefault = false;
            //    bool allowEmptyValue = true;
            //    bool isDefault = true;

            profile << OptionSpec(kAgPssmFileOpt,       notRequired, noEmptyValue);
            profile << OptionSpec(kNumAlignmentsOpt,    notRequired, noEmptyValue, notDefault, QString(), QRegExp("^[1-9][0-9]*$"));
            profile << OptionSpec(kDatabaseOpt,         required,    noEmptyValue);
            profile << OptionSpec(kNumDescriptionsOpt,  notRequired, noEmptyValue, notDefault, QString(), QRegExp("^[1-9][0-9]*$"));
            profile << OptionSpec(kEvalueOpt,           notRequired, noEmptyValue);
            profile << OptionSpec(kInclusionEThreshOpt, notRequired, noEmptyValue);
            profile << OptionSpec(kIterationsOpt,       notRequired, noEmptyValue, notDefault, QString(), QRegExp("^[1-9][0-9]*$"));
            profile << OptionSpec(kMatrixOpt,           notRequired, noEmptyValue, notDefault, QString(), QRegExp("^(?:BLOSUM(?:45|50|62|80|90))|(?:PAM(?:30|70|250))$"));
            profile << OptionSpec(kMaxTargetSeqsOpt,    notRequired, noEmptyValue, notDefault, QString(), QRegExp("^[1-9][0-9]*$"));
            profile << OptionSpec(kDatabaseSizeOpt,     notRequired, noEmptyValue, notDefault, QString(), QRegExp("^0|[1-9][0-9]*$"));
            profile << OptionSpec(kSearchSpaceOpt,      notRequired, noEmptyValue, notDefault, QString(), QRegExp("^0|[1-9][0-9]*$"));
            profile << OptionSpec(kOutputFormatOpt,     notRequired, noEmptyValue);
            profile << OptionSpec(kThreadsOpt,          notRequired, noEmptyValue, notDefault, QString(), QRegExp("^[1-9][0-9]*$"));

            return profile;
        }
        const OptionProfile kPsiBlastOptionProfile(initializePsiBlastOptionProfile());
    }
}
