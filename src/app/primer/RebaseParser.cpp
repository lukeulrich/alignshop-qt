/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "RebaseParser.h"

/* slurps in restriction enzyme information from a REBASE file (rebase.neb.com), formats it,
   and returns the data in the form of a QList of restrictionEnzyme struct objects
*/
QVector<RestrictionEnzyme> RebaseParser::parseRebaseFile(const QString &file)
{
    QFile inputFile(file);

    if (!inputFile.open(QIODevice::ReadOnly))
        return QVector<RestrictionEnzyme>();

    QTextStream in(&inputFile);
    QVector<RestrictionEnzyme> enzymes;  // initialize a QList of restriction enzyme struct objects

    while (!in.atEnd())
    {
        QString line = in.readLine();       // read through all of the lines

        // Eliminate header information; all non-enzyme lines start with #
        if (line.startsWith('#'))
            continue;

        QStringList parts = line.split('\t', QString::SkipEmptyParts);
        if (parts.size() != 9)
            continue;

        QString name = parts.at(0);
        BioString recognitionSite(parts.at(1).toAscii(), eDnaGrammar);

        bool ok = false;

        QVector<int> forwardCuts;
        int firstForwardCut = parts.at(5).toInt(&ok);
        if (!ok)
            continue;
        if (firstForwardCut != 0)
            forwardCuts << firstForwardCut;
        int secondForwardCut = parts.at(7).toInt(&ok);
        if (!ok)
            continue;
        if (secondForwardCut != 0)
            forwardCuts << secondForwardCut;

        QVector<int> reverseCuts;
        int firstReverseCut = parts.at(6).toInt(&ok);
        if (!ok)
            continue;
        if (firstReverseCut != 0)
            reverseCuts << firstReverseCut;
        int secondReverseCut = parts.at(8).toInt(&ok);
        if (!ok)
            continue;
        if (secondReverseCut != 0)
            reverseCuts << secondReverseCut;

        // Eliminate all enzymes that cut at more or less than 2 sites (i.e. cutting across greater or fewer than the 2 strands of a helix)
        RestrictionEnzyme restrictionEnzyme(name, recognitionSite, forwardCuts, reverseCuts);
        if (restrictionEnzyme.numCuts() != 2)
            continue;

        enzymes << restrictionEnzyme;

/*

        QRegExp rx(".*\\s\\d+\\s(\\d+).*");
        if (rx.indexIn(line) == -1)
            continue;
        if(rx.cap(1).toInt() > 2 || rx.cap(1).toInt() < 2){continue;} //manually verified in spreadsheet with EMBOSS file

        // Eliminate unusual enzymes that have variable length redundancies on one side or another
        // at time of writing (2010-05-26), this resulted in a final list of 543 enzymes
        // This number was cross-checked against the original link_emboss_e data file
        rx.setPattern("(.*)\\s(\\D+)\\s.*(\\d+)\\s(-?\\d+)\\s(-?\\d+)\\s\\d+\\s\\d+");
        if (rx.indexIn(line) == -1)
            continue;
        if(rx.cap(4).toInt() < 1 || rx.cap(4).toInt() > rx.cap(2).length()){continue;}
        if(rx.cap(5).toInt() < 1 || rx.cap(5).toInt() > rx.cap(2).length()){continue;}

        // Send all of the captured data to the enzymes struct
        enzymes.append(RestrictionEnzyme(rx.cap(1)));
        enzymes.last().recognitionSequence_ = rx.cap(2);
        enzymes.last().cut_pos1_ = rx.cap(4).toInt();
        enzymes.last().cut_pos2_ = rx.cap(5).toInt();

        if(rx.cap(3).toInt() == 1){enzymes.last().bluntOrSticky_ = "blunt";}
        else if(rx.cap(3).toInt() == 0){enzymes.last().bluntOrSticky_ = "sticky";}

        */
        //qDebug() << p << rx.cap(1) << rx.cap(2) << rx.cap(3) << rx.cap(4) << rx.cap(5);
    }

    return enzymes;
}
