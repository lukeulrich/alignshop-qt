/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "../DimerScoreCalculator.h"
#include "../../core/BioString.h"
#include "../../core/util/ClosedIntRange.h"

#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QTextStream out(stdout);

    BioString sequence1(eDnaGrammar);
    BioString sequence2(eDnaGrammar);

    int numArguments = application.arguments().size();
    if (numArguments == 1)
    {
        QString program = QFileInfo(application.arguments().first()).baseName();
        out << "Usage: " << program << " sequence1 [sequence2]" << endl << endl;
        return 0;
    }
    else if (numArguments == 2)
    {
        sequence1 = application.arguments().at(1).toUpper().toAscii();
        sequence2 = sequence1;
    }
    else if (numArguments > 2)
    {
        sequence1 = application.arguments().at(1).toUpper().toAscii();
        sequence2 = application.arguments().at(2).toUpper().toAscii();
    }

    // Now to calculate the hbonds between two sequences:
    DimerScoreCalculator scoreCalculator;

    PairwiseHydrogenBondCount result = scoreCalculator.locateMaximumHydrogenBonds(sequence1, sequence2);

    out << "Hydrogen bonds: " << result.hydrogenBonds_ << endl;
    int aOffset = sequence2.length();
    out << QByteArray(" ").repeated(aOffset - 1) << sequence1.asByteArray() << endl;
    sequence2.reverse();

    // Draw the midline
    out << QByteArray(" ").repeated(aOffset - 1 + result.queryRange_.begin_ - 1);
    for (int i=result.queryRange_.begin_, j= result.subjectRange_.begin_; i<= result.queryRange_.end_; ++i, ++j)
    {
        if ( (sequence1.at(i) == 'A' && sequence2.at(j) == 'T') ||
             (sequence1.at(i) == 'T' && sequence2.at(j) == 'A'))
        {
            out << "2";
        }
        else if ((sequence1.at(i) == 'G' && sequence2.at(j) == 'C') ||
                 (sequence1.at(i) == 'C' && sequence2.at(j) == 'G'))
        {
            out << "3";
        }
        else
        {
            out << " ";
        }
    }
    out << endl;

    out << QByteArray(" ").repeated(aOffset - 1 + result.queryRange_.begin_ - 1 - result.subjectRange_.begin_ + 1) << sequence2.asByteArray() << endl;

    return 0;
}

