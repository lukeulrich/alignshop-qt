/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include "RebaseParser.h"
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QRegExp>


// Constructor
RebaseParser::RebaseParser()
{

}

/* slurps in restriction enzyme information from a REBASE file (rebase.neb.com), formats it,
   and returns the data in the form of a QList of restrictionEnzyme struct objects
*/
QList<restrictionEnzyme> RebaseParser::parseRebaseFile()
{
        QFile inputFile(":/rebase/link_emboss_e");

        inputFile.open(QIODevice::ReadOnly);

        QTextStream in(&inputFile);
        QStringList restrictionList;
        QHash<QString, int> cutList; // cutList will hold the cut positions; keys will be the name of the enzyme
                                     // I incorporated this QHash because the QStringList can not hold integers

        QList<restrictionEnzyme> enzymes;  // initialize a QList of restriction enzyme struct objects

        while (!in.atEnd())
        {
            QString line = in.readLine();       // read through all of the lines

            // Eliminate header information; all non-enzyme lines start with #
             int i = line.indexOf(QRegExp("#"));
             if(i>-1){continue;}

            // Eliminate all enzymes that cut at more or less than 2 sites (i.e. cutting across greater or fewer than the 2 strands of a helix)
             QRegExp rx(".*\\s\\d+\\s(\\d+).*");
             rx.indexIn(line);
             if(rx.cap(1).toInt() > 2 || rx.cap(1).toInt() < 2){continue;} //manually verified in spreadsheet with EMBOSS file

             // Eliminate unusual enzymes that have variable length redundancies on one side or another
             // at time of writing (2010-05-26), this resulted in a final list of 543 enzymes
             // This number was cross-checked against the original link_emboss_e data file
             rx.setPattern("(.*)\\s(\\D+)\\s.*(\\d+)\\s(-?\\d+)\\s(-?\\d+)\\s\\d+\\s\\d+");
             rx.indexIn(line);
             if(rx.cap(4).toInt() < 1 || rx.cap(4).toInt() > rx.cap(2).length()){continue;}
             if(rx.cap(5).toInt() < 1 || rx.cap(5).toInt() > rx.cap(2).length()){continue;}

             // Send all of the captured data to the enzymes struct
             enzymes.append(rx.cap(1));
             enzymes.last().recognitionSequence_ = rx.cap(2);
             enzymes.last().cut_pos1_ = rx.cap(4).toInt();
             enzymes.last().cut_pos2_ = rx.cap(5).toInt();

             if(rx.cap(3).toInt() == 1){enzymes.last().bluntOrSticky_ = "blunt";}
             else if(rx.cap(3).toInt() == 0){enzymes.last().bluntOrSticky_ = "sticky";}

             //qDebug() << p << rx.cap(1) << rx.cap(2) << rx.cap(3) << rx.cap(4) << rx.cap(5);


        }
        return enzymes;

}
