/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#ifndef REBASEPARSER_H
#define REBASEPARSER_H

#include <QtCore/QString>
#include "RestrictionEnzyme.h"

/** Manages parsing of REBASE restriction enzyme data files (rebase.neb.com). Handles only emboss-formatted REBASE files.
  *
  * RebaseParser() returns a QList containing struct restrictionEnzyme objects. Each object contains
  * the enzyme name, recognition sequence, type of termini produced by digest (blunt or sticky-ended),
  * and the cut positions on both strands.
  */

class RebaseParser
{
public:
    QVector<RestrictionEnzyme> parseRebaseFile(const QString &file); // extract relevant enzyme information from REBASE file
};



#endif // REBASEPARSER_H
