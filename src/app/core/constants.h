/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVector>

class Alphabet;

namespace constants
{
    extern const int kSecondsPerMinute;
    extern const int kSecondsPerHour;
    extern const int kSecondsPerDay;

    extern const char k7BitCharacters[];

    extern const char kGapCharacters[];
    extern const char kDefaultGapCharacter;

    extern const int kInvalidColumn;

    extern const int kParserStreamingBufferSize;

    // Various character sets associated with each macromolecule type
    extern const char kDefaultAnyCharacter;

    extern const char kDnaAnyCharacter;
    extern const char kDnaBasicCharacters[];
    extern const char kDnaExtendedCharacters[];
    extern const char kDnaAmbiguousCharacters[];

    extern const char kRnaAnyCharacter;
    extern const char kRnaBasicCharacters[];
    extern const char kRnaAmbiguousCharacters[];

    extern const char kAminoAnyCharacter;
    extern const char kAminoBasicCharacters[];
    extern const char kAminoExtendedCharacters[];
    extern const char kAminoAmbiguousCharacters[];

    // Predefined alphabets
    extern const Alphabet kDnaBasicAlphabet;
    extern const Alphabet kDnaExtendedAlphabet;
    extern const Alphabet kDnaAmbiguousAlphabet;
    extern const Alphabet kRnaBasicAlphabet;
    extern const Alphabet kRnaAmbiguousAlphabet;
    extern const Alphabet kAminoBasicAlphabet;
    extern const Alphabet kAminoExtendedAlphabet;
    extern const Alphabet kAminoAmbiguousAlphabet;

    extern const QVector<Alphabet> kStandardAlphabetVector;

    // Pssm
    // Defined here because it is needed in the construction of arrays on the stack (e.g. Pssm.h)
    const int kPssmWidth  = 20;         // Number of characters which have a score; equivalent
                                        // to the number of characters in the basic amino alphabet

    // Number of cores available on the system
    extern const int kNumberOfCores;

    // Path to blast+ programs - relative to executable
    extern const QString kBlastDbCmdRelativePath;
    extern const QString kPsiBlastRelativePath;
    extern const QString kMakeBlastDbRelativePath;
    extern const QString kKalignRelativePath;
    extern const QString kClustalWRelativePath;

    // Path to various data files
    extern const QString kRestrictionEnzymeEmbossPath;

    // Urls
    extern const QUrl kNcbiBlastDbFtpUrl_;
    extern const QUrl kRebaseUrl_;

    // License data
    extern const QByteArray kOutsideSecret;
}

#endif   // CONSTANTS_H
