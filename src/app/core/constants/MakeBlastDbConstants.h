/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MAKEBLASTDBCONSTANTS_H
#define MAKEBLASTDBCONSTANTS_H

class QString;

class OptionProfile;

namespace constants
{
    namespace MakeBlastDb
    {
        extern const QString kFriendlyMakeBlastDbName;

        extern const QString kInFileOpt;
        extern const QString kMoleculeTypeOpt;
        extern const QString kTitleOpt;
        extern const QString kParseSeqIdsOpt;
        extern const QString kOutFileOpt;
        extern const QString kFastaTickOpt;
        extern const QString kLogFileOpt;

        // Possible database types
        extern const QString kMoleculeTypeProtein;
        extern const QString kMoleculeTypeNucleotide;

        extern const OptionProfile kMakeBlastDbOptionProfile;
    }
}

#endif // MAKEBLASTDBCONSTANTS_H
