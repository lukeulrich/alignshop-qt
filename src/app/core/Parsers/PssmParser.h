/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSSMPARSER_H
#define PSSMPARSER_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include "../PODs/Pssm.h"

class QTextStream;

/**
  * PssmParser parses custom AG-formatted PSSM files and returns a corresponding PSSM data structure.
  *
  * An example of the format expected by PssmParser is below:
  *
  * # AG-PSSM - last position-specific scoring matrix computed
  * # PSI_SCALE_FACTOR:200
  *
  *              A     R     N     D     C     Q     E     G     H     I     L     K     M     F     P     S     T     W     Y     V
  *    1 M    -211  -305  -476  -674  -316   -98  -442  -591  -345   240   428  -302  1213    -4  -547  -330  -152  -317  -224   144
  *    2 S     124  -231   136   542  -305   -61    35  -134  -232  -574  -608   -95  -411  -590  -234   802   191  -685  -450  -445
  *    3 S     347  -223   -20   -57  -310    -1   334   352  -237  -551  -556   -74  -373  -584  -247   605    88  -608  -443  -372
  *    4 I    -193  -298  -307  -432  -338   230  -246  -554  -392   335   230  -232   541  -241   212   -92   480  -502  -340   113
  *    5 R    -193   652   688   -82  -531    -5  -117  -306  -115  -585  -566   104  -387  -642   460   224   253  -699  -456  -472
  *    ...
  *  103 E    -173   -23   -54   302  -722   371   981  -422   -24  -639  -569   155  -400  -638  -223   -29  -173  -567  -404  -488
  *  104 Y    -353  -339  -416  -613  -481  -284  -404  -608   339  -266  -212  -364  -199   588  -584  -337  -321   431  1319  -242
  *
  *                      K         Lambda
  * Standard Ungapped    0.1387     0.3183
  * Standard Gapped      0.0410     0.2670
  * PSI Ungapped         0.1387     0.3183
  * PSI Gapped           0.0410     0.2670
  * [EOF]
  *
  * The only permitted variance is that the score columns may be re-ordered freely given that there are 20 score columns
  * and there is a corresponding header line. It is critical that at least one blank line occurs after the last row of
  * scores. The statistics section is ignored.
  *
  * Any deviations from the above format specifications will cause a QString exception to be thrown.
  */
class PssmParser
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    Pssm parseFile(const QString &fileName) const;                      //!< Parse the PSSM data contained in fileName and return a Pssm

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    Pssm parseStream(QTextStream &stream) const;                        //!< Parse the pssm data from stream and return a Pssm
    QVector<char> splitPssmTableHeader(const QString &thLine) const;    //!< Helper method to split the score header column containing residues/nucleotides into a vector of chars
    QStringList splitByWhitespace(const QString &string) const;         //!< Separates the values in string by their whitespace and returns a QStringList of the words found
};

#endif // PSSMPARSER_H
