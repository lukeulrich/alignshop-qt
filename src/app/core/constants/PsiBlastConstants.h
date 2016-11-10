/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSIBLASTCONSTANTS_H
#define PSIBLASTCONSTANTS_H

class QString;
class QStringList;

class OptionProfile;

namespace constants
{
    namespace PsiBlast
    {
        extern const QString kFriendlyPsiBlastName;

        extern const QString kAgPssmFileOpt;
        extern const QString kNumAlignmentsOpt;
        extern const QString kDatabaseOpt;
        extern const QString kDatabaseSizeOpt;
        extern const QString kNumDescriptionsOpt;
        extern const QString kEvalueOpt;
        extern const QString kInclusionEThreshOpt;
        extern const QString kIterationsOpt;
        extern const QString kMatrixOpt;
        extern const QString kMaxTargetSeqsOpt;
        extern const QString kOutputFormatOpt;
        extern const QString kSearchSpaceOpt;
        extern const QString kThreadsOpt;

        enum OutputFormat {
            ePairwiseOutput = 0,
            eQueryAnchoredWithIdentitiesOutput,
            eQueryAnchoredNoIdentitiesOutput,
            eFlatQueryAnchoredWithIdentitiesOutput,
            eFlatQueryAnchoredNoIdentitiesOutput,
            eXmlOutput,
            eTabularOutput,
            eTabularWithCommentsOutput,
            eTextAsn1Output,
            eBinaryAsn1Output,
            eCSVOutput,
            eBlastAsn1
        };

        extern const QString kBLOSUM45;
        extern const QString kBLOSUM50;
        extern const QString kBLOSUM62;
        extern const QString kBLOSUM80;
        extern const QString kBLOSUM90;
        extern const QString kPAM30;
        extern const QString kPAM70;
        extern const QString kPAM250;

        extern const QStringList kMatrixList;

        extern const OptionProfile kPsiBlastOptionProfile;
    }
}

#endif // PSIBLASTCONSTANTS_H
