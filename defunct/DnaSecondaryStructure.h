/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#ifndef DNASECONDARYSTRUCTURE_H
#define DNASECONDARYSTRUCTURE_H

#include <QtCore/QList>
#include <QtCore/QString>

/** Models secondary structure of DNA strands (heterodimers, homodimers, hairpins). HAIRPIN IMPLEMENTATION NOT ACCURATE.
  *
  * Provides a means of estimating the most stable secondary structure of DNA strands.
  * Currently, dimer() accurately scans input DNA sequences <QString, QString> for base-pairing and returns
  * a struct of the highest scoring structure.
  *
  * Data in the struct is organized into 3 lines formatted for display to userin AlignShop: Line 1 = sequence1,
  * Line 2 = identity/nonidentity between the two strands. Identity is denoted with the pipe symbol, nonidentity with white space.
  * Line 3 = sequence2.
  *
  * Also inluded in struct dimerContainer is a measure (int dimerScore_) of the number of H-bonds estimated in the most stable structure.
  *
  * Only the first structures detected in a tie with the highest dimerScore_ is reported. Users could eventually like to see all tied structures.
  *
  * Hairpin modeling is not implemented accurately. Some of the sequence space is unexplored and the line output is incorrect.
  */

struct dimerContainer
{
    QList<QString> displayLines_;
    int dimerScore_;
};
// Tell Qt how to handle this data structure so it can optimize it's container usage and copying performance
Q_DECLARE_TYPEINFO(dimerContainer, Q_MOVABLE_TYPE);


class DnaSecondaryStructure
{
private:
    QString sequence_;
    int highestHairpinScore_;           // maximal hairpin for a given sequence
    QList<QString> hairpinDiagram_;
    QList<QString> dimerDiagram_;


public:
    //Constructor
    DnaSecondaryStructure();
    //Destructor
    ~DnaSecondaryStructure();

    void setSequence(QString currentSequence);
    QString sequence();
    int hairpinSearch();                // returns a score for the maximal # of hydrogen bonds that could be formed in a hairpin stem
    QList<QString> hairpinDisplay();    // returns series of QStrings representing a simple text output of the predicted hairpin
    static dimerContainer dimer(QString sequence1, QString sequence2);

};


inline
DnaSecondaryStructure::~DnaSecondaryStructure()
{

}
#endif // DNASECONDARYSTRUCTURE_H
