/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QThread>

#include "Alphabet.h"
#include "constants.h"
#include "enums.h"

int numberOfCores()
{
    int nCores = QThread::idealThreadCount();
    if (nCores == -1)
        nCores = 1;

    return nCores;
}

namespace constants
{
    const int kSecondsPerMinute = 60;
    const int kSecondsPerHour = kSecondsPerMinute * 60;
    const int kSecondsPerDay = kSecondsPerHour * 24;

    const char k7BitCharacters[] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    const char kGapCharacters[] = "-.";
    const char kDefaultGapCharacter = '-';

    const int kInvalidColumn = -1;

    const int kParserStreamingBufferSize = 8192;

    const char kDefaultAnyCharacter = 'X';

    const char kDnaAnyCharacter = 'N';
    const char kDnaBasicCharacters[] = "ACGT";
    const char kDnaExtendedCharacters[] = "ABCDGSTW";
    const char kDnaAmbiguousCharacters[] = "ABCDGHKMNRSTVWY";

    const char kRnaAnyCharacter = 'N';
    const char kRnaBasicCharacters[] = "ACGU";
    const char kRnaAmbiguousCharacters[] = "ABCDGHKMNRSUVWY";

    const char kAminoAnyCharacter = 'X';
    const char kAminoBasicCharacters[] = "ACDEFGHIKLMNPQRSTVWY";            // It is important for the NNStructureTool and others that these are in alphabetical order
    const char kAminoExtendedCharacters[] = "ACDEFGHIKLMNOPQRSTUVWY";
    const char kAminoAmbiguousCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    const Alphabet kDnaBasicAlphabet(eDnaGrammar, kDnaBasicCharacters, kDnaAnyCharacter);
    const Alphabet kDnaExtendedAlphabet(eDnaGrammar, kDnaExtendedCharacters, kDnaAnyCharacter);
    const Alphabet kDnaAmbiguousAlphabet(eDnaGrammar, kDnaAmbiguousCharacters, kDefaultAnyCharacter);
    const Alphabet kRnaBasicAlphabet(eRnaGrammar, kRnaBasicCharacters, kRnaAnyCharacter);
    const Alphabet kRnaAmbiguousAlphabet(eRnaGrammar, kRnaAmbiguousCharacters, kRnaAnyCharacter);
    const Alphabet kAminoBasicAlphabet(eAminoGrammar, kAminoBasicCharacters, kAminoAnyCharacter);
    const Alphabet kAminoExtendedAlphabet(eAminoGrammar, kAminoExtendedCharacters, kAminoAnyCharacter);
    const Alphabet kAminoAmbiguousAlphabet(eAminoGrammar, kAminoAmbiguousCharacters, kAminoAnyCharacter);
    const QVector<Alphabet> kStandardAlphabetVector(QVector<Alphabet>()
                                                    << constants::kDnaBasicAlphabet
                                                    << constants::kDnaExtendedAlphabet
                                                    << constants::kDnaAmbiguousAlphabet
                                                    << constants::kAminoBasicAlphabet
                                                    << constants::kAminoExtendedAlphabet
                                                    << constants::kAminoAmbiguousAlphabet
                                                    << constants::kRnaBasicAlphabet
                                                    << constants::kRnaAmbiguousAlphabet);


    const int kNumberOfCores(numberOfCores());

#ifdef Q_OS_WIN
    const QString kBlastDbCmdRelativePath = "tools/blastdbcmd.exe";
    const QString kPsiBlastRelativePath = "tools/psiblast.exe";
    const QString kMakeBlastDbRelativePath = "tools/makeblastdb.exe";
    const QString kKalignRelativePath = "tools/kalign.exe";
    const QString kClustalWRelativePath = "tools/clustalw2.exe";
#else
    const QString kBlastDbCmdRelativePath = "tools/blastdbcmd";
    const QString kPsiBlastRelativePath = "tools/psiblast";
    const QString kMakeBlastDbRelativePath = "tools/makeblastdb";
    const QString kKalignRelativePath = "tools/kalign";
    const QString kClustalWRelativePath = "tools/clustalw2";
#endif

    const QString kRestrictionEnzymeEmbossPath = "data/link_emboss_e";

    // Urls
    const QUrl kNcbiBlastDbFtpUrl_("ftp://ftp.ncbi.nih.gov/blast/db/");
    const QUrl kRebaseUrl_("http://rebase.neb.com/rebase/link_emboss_e");

    // License info
    const QByteArray kOutsideSecret("eechoh6E");
}
