/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QPoint>
#include <QtCore/QPointF>

#include <QtGui/QColor>
#include <QtGui/QImage>

#include <cmath>

#include "global.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Global functions
/**
  * Returns a random integer between minimum and maximum inclusive.
  *
  * @param minimum [int]
  * @param maximum [int]
  * @returns int
  */
int randomInteger(int minimum, int maximum)
{
    ASSERT(maximum >= minimum);

    return qrand() % ((maximum + 1) - minimum) + minimum;
}

/**
  * @returns bool
  */
bool isGapCharacter(char ch)
{
    const char *x = constants::kGapCharacters;
    while (*x != '\0')
    {
        if (*x == ch)
            return true;

        ++x;
    }

    return false;
}

/**
  * @returns bool
  */
bool isGapCharacter(QChar ch)
{
    return isGapCharacter(ch.toAscii());
}


/**
  * @param listHashCharInt [const ListHashCharInt &]
  * @param divisor [int]
  * @returns ListHashCharDouble
  */
ListHashCharDouble divideListHashCharInt(const ListHashCharInt &listHashCharInt, int divisor)
{
    ASSERT_X(divisor != 0, "divisor may not be zero");

    ListHashCharDouble dividend;
    ListHashCharInt::const_iterator i;
    for (i = listHashCharInt.begin(); i != listHashCharInt.end(); ++i)
    {
        // i == iterator
        // *i == QHash<char, int>

        // Add the column for this dividend
        dividend << QHash<char, qreal>();
        QHash<char, int>::const_iterator j;
        for (j = i->constBegin(); j != i->constEnd(); ++j)
        {
            // j == iterator
            // j.key() == character
            // j.value() == count of character
            dividend.last().insert(j.key(), static_cast<qreal>(j.value()) / static_cast<qreal>(divisor));
        }
    }

    return dividend;
}

/**
  * @param point [const QPointF &]
  * @returns QPoint
  */
QPoint floorPoint(const QPointF &point)
{
    return QPoint(floor(point.x()),
                  floor(point.y()));
}

/**
  * Specifically searches for non-zero blue values. Returns a null, uninitialized Rect if no blue pixel data is found.
  *
  * Optimized for 32-bit image data for finding the top and bottom boundaries.
  *
  * @param image [const QImage &]
  * @param threshold [int]
  * @returns Rect
  */
Rect boundingRect(const QImage &image, int threshold)
{
    if (image.width() == 0 || image.height() == 0)
        return Rect();

    int x_min = 0;
    int x_max = image.width()-1;
    int y_min = 0;
    int y_max = image.height()-1;

    int h = image.height();
    int w = image.width();

    // ------------
    // Scan the top
    if (image.depth() == 32)
    {
        for (; y_min < h; ++y_min)
        {
            QRgb *p = (QRgb *)(image.scanLine(y_min));
            for (int x=0; x< w; ++x)
                if (qBlue(*p++) > threshold)
                    goto DONE_TOP;
        }
    }
    else    // Image is not 32 bpp
    {
        for (; y_min < h; ++y_min)
            for (int x=0; x< w; ++x)
                if (qBlue(image.pixel(x, y_min)) > threshold)
                    goto DONE_TOP;
    }
 DONE_TOP:

    // ---------------
    // Scan the bottom
    if (image.depth() == 32)
    {
        for (; y_max > y_min; --y_max)
        {
            QRgb *p = (QRgb *)(image.scanLine(y_max));
            for (int x=0; x< w; ++x)
                if (qBlue(*p++) > threshold)
                    goto DONE_BOTTOM;
        }
    }
    else
    {
        for (; y_max > y_min; --y_max)
            for (int x=0; x< w; ++x)
                if (qBlue(image.pixel(x, y_max)) > threshold)
                    goto DONE_BOTTOM;
    }
 DONE_BOTTOM:

    // -------------
    // Scan the left
    for (; x_min < w; ++x_min)
//         for (int y=0; y< h; ++y)
        // Optimize a bit by only scanning between the y_min and y_max regions
        for (int y=y_min; y<= y_max; ++y)
            if (qBlue(image.pixel(x_min, y)) > threshold)
                goto DONE_LEFT;
 DONE_LEFT:

    // --------------
    // Scan the right
    for (; x_max > x_min; --x_max)
//        for (int y=0; y< h; ++y)
         // Optimize a bit by only scanning between the y_min and y_max regions
         for (int y=y_min; y<= y_max; ++y)
            if (qBlue(image.pixel(x_max, y)) > threshold)
                goto DONE_RIGHT;
 DONE_RIGHT:

    if (x_min > x_max || y_min > y_max)
        return Rect();

    return Rect(x_min, y_min, x_max - x_min, y_max - y_min);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// "Private" initialization functions for complex constant objects (e.g. kDnaBasePairHash)
static QHash<QChar, QChar> __dnaBasePairHash();
static QHash<QChar, QChar> __rnaBasePairHash();
static QHash<Alphabet, QHash<TableGroup, const char *> > __tableHash();

/**
  * Contains relevant global constants, functions, and other miscellaneous declarations.
  */
namespace constants
{
    const char kGapCharacters[] = ".-";
    const char kDefaultGapCharacter = '-';
    const char kStopCodonCharacter = '*';

    // Primer constants
    const int kAmpliconLengthMin = 50;
    const int kAmpliconLengthMax = 1024 * 1024;

    const int kPrimerLengthDefault = 17;
    const int kPrimerLengthMin = 10;
    const int kPrimerLengthMax = 120;

    const double kTmDefaultMax = 60;
    const double kTmDefaultMin = 50;
    const double kTmDefault = (kTmDefaultMax + kTmDefaultMin) / 2;

    // Generic biostring
    const char kGenericBioStringMaskCharacter = 'X';
    const char kGenericBioStringCharacters[] = "ABCDEFGHJIJKLMNOPQRSTUVWXYZ.-*";

    // Amino acid
    const char kAminoMaskCharacter = 'X';
    const char kAminoCharacters[] = "ACDEFGHIKLMNPQRSTVWY";
    const char kAminoExpandedCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXY";

    // DNA
    const char kDnaMaskCharacter = 'N';
    const char kDnaCharacters[] = "ACGT";
    const char kDnaExpandedCharacters[] = "ABCDGHIKMNRSTVWXY";
    const QHash<QChar, QChar> kDnaBasePair(__dnaBasePairHash());

    // RNA
    const char kRnaMaskCharacter = 'N';
    const char kRnaCharacters[] = "ACGU";
    const char kRnaExpandedCharacters[] = "ABCDGHIKMNRSUVWXY";
    const QHash<QChar, QChar> kRnaBasePair(__rnaBasePairHash());

    // Auto-detect values
    const int kAutoDetectAlphabet = -1;
    const int kAutoDetectDataFormat = -1;

    // Database table names
    const char kTableDataTree[] = "data_tree";

    const char kTableAstrings[] = "astrings";
    const char kTableDstrings[] = "dstrings";
    const char kTableRstrings[] = "rstrings";

    const char kTableAminoSeqs[] = "amino_seqs";
    const char kTableAminoSubseqs[] = "amino_subseqs";
    const char kTableAminoMsas[] = "amino_msas";
    const char kTableAminoMsaSubseqs[] = "amino_msas_subseqs";

    const char kTableDnaSeqs[] = "dna_seqs";
    const char kTableDnaSubseqs[] = "dna_subseqs";
    const char kTableDnaMsas[] = "dna_msas";
    const char kTableDnaMsaSubseqs[] = "dna_msas_subseqs";

    const char kTableRnaSeqs[] = "rna_seqs";
    const char kTableRnaSubseqs[] = "rna_subseqs";
    const char kTableRnaMsas[] = "rna_msas";
    const char kTableRnaMsaSubseqs[] = "rna_msas_subseqs";

    // Create mapping to these tables via the alphabet
    const QHash<Alphabet, QHash<TableGroup, const char *> > kTableHash(__tableHash());

    // Various messages
    const char kMessageInvalidConnection[] = "Invalid database connection";
    const char kMessageErrorPreparingQuery[] = "Error preparing database query";
    const char kMessageErrorExecutingQuery[] = "Error executing database query";
    const char kMessageErrorMissingTableName[] = "Missing table name";

    // All 7-bit valid characters
    const char k7BitCharacters[] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    // Default rendering grid resolution
    const int kDefaultResolution = 64;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private function definitions for initializing the complex constant data structures
static QHash<QChar, QChar> __dnaBasePairHash()
{
    QHash<QChar, QChar> basePairs;

    basePairs['A'] = 'T';
    basePairs['T'] = 'A';
    basePairs['G'] = 'C';
    basePairs['C'] = 'G';

    return basePairs;
}

static QHash<QChar, QChar> __rnaBasePairHash()
{
    QHash<QChar, QChar> basePairs;

    basePairs['A'] = 'U';
    basePairs['U'] = 'A';
    basePairs['G'] = 'C';
    basePairs['C'] = 'G';

    return basePairs;
}

static QHash<Alphabet, QHash<TableGroup, const char *> > __tableHash()
{
    QHash<Alphabet, QHash<TableGroup, const char *> > tableHash;

    // Amino
    tableHash[eAminoAlphabet][eStringGroup] = constants::kTableAstrings;
    tableHash[eAminoAlphabet][eSeqGroup] = constants::kTableAminoSeqs;
    tableHash[eAminoAlphabet][eSubseqGroup] = constants::kTableAminoSubseqs;
    tableHash[eAminoAlphabet][eMsaGroup] = constants::kTableAminoMsas;
    tableHash[eAminoAlphabet][eMsaSubseqGroup] = constants::kTableAminoMsaSubseqs;

    // Dna
    tableHash[eDnaAlphabet][eStringGroup] = constants::kTableDstrings;
    tableHash[eDnaAlphabet][eSeqGroup] = constants::kTableDnaSeqs;
    tableHash[eDnaAlphabet][eSubseqGroup] = constants::kTableDnaSubseqs;
    tableHash[eDnaAlphabet][eMsaGroup] = constants::kTableDnaMsas;
    tableHash[eDnaAlphabet][eMsaSubseqGroup] = constants::kTableDnaMsaSubseqs;

    // Rna
    tableHash[eRnaAlphabet][eStringGroup] = constants::kTableRstrings;
    tableHash[eRnaAlphabet][eSeqGroup] = constants::kTableRnaSeqs;
    tableHash[eRnaAlphabet][eSubseqGroup] = constants::kTableRnaSubseqs;
    tableHash[eRnaAlphabet][eMsaGroup] = constants::kTableRnaMsas;
    tableHash[eRnaAlphabet][eMsaSubseqGroup] = constants::kTableRnaMsaSubseqs;

    return tableHash;
}

