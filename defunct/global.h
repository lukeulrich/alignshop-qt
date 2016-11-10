/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore/QMetaType>
#include <QtCore/QHash>

#include "util/Rect.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Macros
#define ASSERT_X(x,y) Q_ASSERT_X(x, __FUNCTION__, y)
#define ASSERT(x) Q_ASSERT_X(x, __FUNCTION__, "")


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Useful typedefs
typedef QList<QHash<char, int> > ListHashCharInt;       //!< Self-descriptive typedef: List of hashes that each associate a char to an integer value
Q_DECLARE_METATYPE(ListHashCharInt);
typedef QList<QHash<char, qreal> > ListHashCharDouble;  //!< Self-descriptive typedef: List of hashes that each associate a char to a double value
Q_DECLARE_METATYPE(ListHashCharDouble);
typedef QPair<char, qreal> PairCharDouble;              //!< Self-descriptive typedef: character, qreal pair
Q_DECLARE_METATYPE(PairCharDouble);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Global utility functions
int randomInteger(int minimum, int maximum);            //!< Returns a random integer between minimum and maximum inclusive
bool isGapCharacter(char ch);                           //!< Returns true if ch is a gap character; false otherwise
bool isGapCharacter(QChar ch);                          //!< Returns true if ch is a gap character; false otherwise
//! Iterates through all values in listHashCharInt and divides them by divisor and returns the resulting dividend set (has equal dimensions); divisor must not be zero
ListHashCharDouble divideListHashCharInt(const ListHashCharInt &listHashCharInt, int divisor);
QPoint floorPoint(const QPointF &point);                //!< Converts the floating point, point, to a QPoint by flooring its x and y values
//! Returns the smallest bounding rectangle that fully contains all blue pixel data above threshold in image
Rect boundingRect(const QImage &image, int threshold = 10);



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Enums
enum Alphabet
{
    eUnknownAlphabet = 0,
    eAminoAlphabet,
    eDnaAlphabet,
    eRnaAlphabet
};

enum ItemViewType
{
    eViewAll = 0,
    eViewAmino,
    eViewDna,
    eViewRna
};

enum DataFormatType {
    eUnknownFormatType = 0,
    eFastaType,
    eAlignedFastaType,
    eClustalType
};

enum SortDirection {
    eSortAscending = 0,
    eSortDescending
};

enum TableGroup {
    eStringGroup = 0,
    eSeqGroup,
    eSubseqGroup,
    eMsaGroup,
    eMsaSubseqGroup
};

Q_DECLARE_METATYPE(Alphabet)


/**
  * Contains relevant global constants, functions, and other miscellaneous declarations.
  */
namespace constants
{
    extern const char kGapCharacters[];
    extern const char kDefaultGapCharacter;
    extern const char kStopCodonCharacter;

    // Primer constants
    extern const int kAmpliconLengthMin;
    extern const int kAmpliconLengthMax;

    extern const int kPrimerLengthDefault;
    extern const int kPrimerLengthMin;
    extern const int kPrimerLengthMax;

    extern const double kTmDefaultMax;
    extern const double kTmDefaultMin;
    extern const double kTmDefault;

    // Generic biostring
    extern const char kGenericBioStringMaskCharacter;
    extern const char kGenericBioStringCharacters[];

    // Amino acid
    extern const char kAminoMaskCharacter;
    extern const char kAminoCharacters[];
    extern const char kAminoExpandedCharacters[];

    // DNA
    extern const char kDnaMaskCharacter;
    extern const char kDnaCharacters[];
    extern const char kDnaExpandedCharacters[];
    extern const QHash<QChar, QChar> kDnaBasePair;

    // RNA
    extern const char kRnaMaskCharacter;
    extern const char kRnaCharacters[];
    extern const char kRnaExpandedCharacters[];
    extern const QHash<QChar, QChar> kRnaBasePair;

    // Auto-detect values
    extern const int kAutoDetectAlphabet;
    extern const int kAutoDetectDataFormat;

    // Database table names
    extern const char kTableDataTree[];

    extern const char kTableAstrings[];
    extern const char kTableDstrings[];
    extern const char kTableRstrings[];

    extern const char kTableAminoSeqs[];
    extern const char kTableAminoSubseqs[];
    extern const char kTableAminoMsas[];
    extern const char kTableAminoMsaSubseqs[];

    extern const char kTableDnaSeqs[];
    extern const char kTableDnaSubseqs[];
    extern const char kTableDnaMsas[];
    extern const char kTableDnaMsaSubseqs[];

    extern const char kTableRnaSeqs[];
    extern const char kTableRnaSubseqs[];
    extern const char kTableRnaMsas[];
    extern const char kTableRnaMsaSubseqs[];

    // Create mapping to these tables via the alphabet
    extern const QHash<Alphabet, QHash<TableGroup, const char *> > kTableHash;

    // Various messages
    extern const char kMessageInvalidConnection[];
    extern const char kMessageErrorPreparingQuery[];
    extern const char kMessageErrorExecutingQuery[];
    extern const char kMessageErrorMissingTableName[];

    // All valid 7-bit characters
    extern const char k7BitCharacters[];

    // Default rendering grid resolution
    extern const int kDefaultResolution;
}

#endif // GLOBAL_H
