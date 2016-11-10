/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MakeBlastDbConstants.h"

#include <QtCore/QString>
#include "../PODs/OptionSpec.h"
#include "../util/OptionProfile.h"

namespace constants
{
    namespace MakeBlastDb
    {
        const QString kFriendlyMakeBlastDbName            = "makeblastdb";

        const QString kInFileOpt                          = "-in";
        const QString kMoleculeTypeOpt                    = "-dbtype";
        const QString kTitleOpt                           = "-title";
        const QString kParseSeqIdsOpt                     = "-parse_seqids";
        const QString kOutFileOpt                         = "-out";
        const QString kFastaTickOpt                       = "-fasta_tick";
        const QString kLogFileOpt                         = "-logfile";

        // Possible database types
        const QString kMoleculeTypeProtein                = "prot";
        const QString kMoleculeTypeNucleotide             = "nucl";

        OptionProfile initializeMakeBlastDbOptionProfile()
        {
            bool notRequired = false;
            bool noEmptyValue = false;
            bool isDefault = true;
            bool notDefault = false;

            OptionProfile profile;

            // Techincally, the infile is required
            profile << OptionSpec(kInFileOpt,       notRequired, noEmptyValue);
            profile << OptionSpec(kMoleculeTypeOpt, notRequired, noEmptyValue, isDefault, kMoleculeTypeProtein, QRegExp("^prot|nucl$"));
            profile << OptionSpec(kTitleOpt,        notRequired, noEmptyValue);
            profile << OptionSpec(kParseSeqIdsOpt);
            profile << OptionSpec(kOutFileOpt,      notRequired, noEmptyValue);
            profile << OptionSpec(kFastaTickOpt,    notRequired, noEmptyValue, notDefault, "1000", QRegExp("^[1-9]\\d*$"));
            profile << OptionSpec(kLogFileOpt,      notRequired, noEmptyValue);

            return profile;
        }
        const OptionProfile kMakeBlastDbOptionProfile(initializeMakeBlastDbOptionProfile());
    }
}
