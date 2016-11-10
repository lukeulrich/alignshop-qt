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

/** Manages parsing of REBASE restriction enzyme data files (rebase.neb.com). Handles only emboss-formatted REBASE files.
  *
  * RebaseParser() returns a QList containing struct restrictionEnzyme objects. Each object contains
  * the enzyme name, recognition sequence, type of termini produced by digest (blunt or sticky-ended),
  * and the cut positions on both strands.
  */

struct restrictionEnzyme
{
    QString name_;
    QString recognitionSequence_;
    QString bluntOrSticky_;
    int cut_pos1_;
    int cut_pos2_;

    restrictionEnzyme(QString name)
    {
        name_ = name;
    }
};
// Tell Qt how to handle this data structure so it can optimize it's container usage and copying performance
Q_DECLARE_TYPEINFO(restrictionEnzyme, Q_MOVABLE_TYPE);


class RebaseParser
{
public:
    RebaseParser(); // constructor

    QList<restrictionEnzyme> parseRebaseFile(); // extract relevant enzyme information from REBASE file
};



#endif // REBASEPARSER_H
