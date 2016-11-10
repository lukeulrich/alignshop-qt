/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QtCore/QRegExp>

#include "math.h"

#include "DnaSecondaryStructure.h"
#include "DnaString.h"

#include <QDebug>



DnaSecondaryStructure::DnaSecondaryStructure()
{

}

QString DnaSecondaryStructure::sequence()
{
    return sequence_;
}

void DnaSecondaryStructure::setSequence(QString currentSequence)
{
    sequence_ = currentSequence;
}


/* THIS FUNCTION IS INCORRECTLY IMPLENTED. CODE LEFT FOR REFERENCE DURING FUTURE IMPLEMENTATION
   Searches for region of maximal possible base pairing if a sequence was folded back on itself in a hairpin
   Returns a value representing the # of hydrogen bonds that can form in the stem. (3 for C/G and 2 for T/A)
   @return int
*/
int DnaSecondaryStructure::hairpinSearch()
{
    highestHairpinScore_ = 0;
    if (sequence_.length() < 5) {return -1;} // if a sequence is too short, return a negative hairpin value
                                             // should improve VALIDATION here; would be good to signal an error
    // QString stemUpstream, stemDownstreamReverseComplement, stemSpacers;
    // int longestStem, loopsize; // loop size will vary

    int minimalLoopSize = 3;                // loops smaller than 3 are sterically not reasonable
    DnaString dnastring = sequence_;        // used to generate reverse complement
                                            // reverse complement a simpler comparison; identical bases represent those that would base pair
                                            // in reality, the downstream portion of a hairpin stem is not the reverse complement (it's the same strand as the upstream portion
                                            // but use of the reverse complement provides a simpler implementation of the comparison
    QString reverseComplement = dnastring.reverseComplement();

    qDebug() << "Working Sequence = " << sequence_;

    // generate full list of fragments to test for hairpins
    QList<QString> slidingWindow;

    for(int i = 4; i <= (sequence_.length()-3)/2; ++i)          // i is incremented through all the allowable fragment sizes
    {
        for(int j = 0; j <= sequence_.length() - 2*i -2; ++j)   // j is incremented through all the potential loop sizes
        {
            slidingWindow.append(sequence_.mid(j,i));
        }
    }


    qDebug() << slidingWindow;
    QHash<QChar, QChar> basePairs;
    basePairs['A'] = 'T';
    basePairs['T'] = 'A';
    basePairs['G'] = 'C';
    basePairs['C'] = 'G';

    for (int i = 0; i < slidingWindow.size(); ++i) // increment through all elements of slidingWindow list
    {
//        QString currentFirstLineDisplay = "";
//        QString currentSecondLineDisplay = "";
//        QString currentThirdLineDisplay = "";
//        QString currentFourthLineDisplay = "";
        qDebug() << slidingWindow[i].length();
        qDebug() << sequence_.length();
        //float temporary = sequence_.length() - 2*slidingWindow[i].length();

        //int maximalLoopSize = floor(temporary);
        int maximalLoopSize = sequence_.length() - 2. *slidingWindow[i].length(); //maximal loop size equals size of the entire sequence minus 2 times the length of the stem
        for(int j = maximalLoopSize; j >= minimalLoopSize; --j) // j = current loop size
        {

            QString currentComparisonWindow = reverseComplement.mid(reverseComplement.length()- j -2 *slidingWindow[i].length(), slidingWindow[i].length()); //extract the relevant portion to compare to the current upstream fragment
            QString currentFirstLineDisplay, currentSecondLineDisplay, currentThirdLineDisplay, currentFourthLineDisplay;
            int hairpinScore = 0;

            // adds bases that precede the sliding window and appropriate spaces
            // works for j=3, sequence of 11 bases
            if(sequence_.indexOf(slidingWindow[i])>0)
            {
                //qDebug() << "Prepend" << sequence_.mid(0,sequence_.indexOf(slidingWindow[i]));
                //qDebug() << "IndexOf" << sequence_.indexOf(slidingWindow[i]);
                currentFirstLineDisplay.append(" ");
                currentSecondLineDisplay.append(sequence_.mid(0,sequence_.indexOf(slidingWindow[i])));
                currentThirdLineDisplay.append(" ");
                currentFourthLineDisplay.append(" ");
            }

            for(int k = 0; k < slidingWindow[i].length(); ++k) //k increments through the bases in the current sliding window fragment
            {
                if(slidingWindow[i][k] == currentComparisonWindow[k])
                {
                        currentFirstLineDisplay.append(" ");
                        currentSecondLineDisplay.append(currentComparisonWindow[k]);
                        currentThirdLineDisplay.append(basePairs[currentComparisonWindow[k]]);
                        currentFourthLineDisplay.append(" ");
                    if(currentComparisonWindow[k] == 'G' || currentComparisonWindow[k] == 'C'){hairpinScore = hairpinScore + 3;} // G's or C's get 3 points as they form 3 hydrogen bonds
                    else{hairpinScore = hairpinScore + 2;}  // A's or T's get 2 points with 2 hydrogen bonds
                }

                if(slidingWindow[i][k] != currentComparisonWindow[k])
                {
                    currentFirstLineDisplay.append(slidingWindow[i][k]);
                    currentSecondLineDisplay.append(" ");
                    currentThirdLineDisplay.append(" ");
                    currentFourthLineDisplay.append(basePairs[currentComparisonWindow[k]]);

                }
                // All of the below looks correct
//                qDebug() << "J " << j << "K " <<k;
//                qDebug() << currentFirstLineDisplay;
//                qDebug() << currentSecondLineDisplay;
//                qDebug() << currentThirdLineDisplay;
//                qDebug() << currentFourthLineDisplay;
            }


            for(int k = 0; k < j; ++k) // k increments through sequence in loop to generate loop diagram
            {
                if (k == 0)
                {
                    //qDebug() << "J in the k loop: " << j;
                    if( j % 2 == 0)
                    {
                        currentFirstLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i])+slidingWindow[i].size(),j/2 - 1 ) );
                        DnaString reverseLoop = sequence_.mid(sequence_.indexOf(slidingWindow[i]) + slidingWindow[i].size()+j/2+1, j/2 -1);
                        currentFourthLineDisplay.append(reverseLoop.reverse());
//                        qDebug() << "Current window =" << slidingWindow[i] << "J = " << j;
//                        qDebug() << currentFirstLineDisplay;
//                        qDebug() << currentSecondLineDisplay;
//                        qDebug() << currentThirdLineDisplay;
//                        qDebug() << currentFourthLineDisplay;

                    }
//
                    else
                    {
                        float jFloat = float(j);
                        currentFirstLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i])+slidingWindow[i].size(), ceil(jFloat/2.) -1  ) );
                        //DnaString reverseLoop = sequence_.mid(sequence_.indexOf(slidingWindow[i]) + slidingWindow[i].size()+ ceil(jFloat/2. - 1), floor(jFloat/2.) );
                        DnaString reverseLoop = sequence_.mid(sequence_.indexOf(slidingWindow[i]) + slidingWindow[i].size()+ ceil(jFloat/2.), floor(jFloat/2.) );
                        //qDebug() << "original 4th line: " << currentFourthLineDisplay;
                        //qDebug() << "adding on to 4th line" << reverseLoop.reverse();
                        currentFourthLineDisplay.append(reverseLoop.reverse());

                        for(int l = 0; l <= floor(jFloat/2. - 1); l++)
                        {
                            currentSecondLineDisplay.append(" ");
                            currentThirdLineDisplay.append(" ");
//                            qDebug() << "Current window =" << slidingWindow[i] << "J = " << j;
//                            qDebug() << currentFirstLineDisplay;
//                            qDebug() << currentSecondLineDisplay;
//                            qDebug() << currentThirdLineDisplay;
//                            qDebug() << currentFourthLineDisplay;

                        }
                        currentSecondLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i])+slidingWindow[i].size()+1, 1 ));
                        currentThirdLineDisplay.append("/");

                        //qDebug() << "J in the else =" << j;
                        //qDebug() << slidingWindow[i];
                        //qDebug() << "First " << currentFirstLineDisplay << ceil(jFloat/2. - 1);

                        //qDebug() << "Second" << currentSecondLineDisplay;
                        //qDebug() << "Third " << currentThirdLineDisplay;
                        //qDebug() << "Fourth" << currentFourthLineDisplay << floor(jFloat/2.);

                    }
//                        currentFirstLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i])+slidingWindow[i].size(),ceil(j/2) + 1  ) );
//                        DnaString reverseLoop = sequence_.mid(sequence_.indexOf(slidingWindow[i]) + slidingWindow[i].size()+floor(j/2)+1, floor(j/2));
//                        currentFourthLineDisplay.append(reverseLoop.reverse());
//                        qDebug() << "J in the else =" << j;
//                        qDebug() << slidingWindow[i];
//                        qDebug() << "First " << currentFirstLineDisplay << ceil(3/2)+1;
//
//                        qDebug() << "Second" << currentSecondLineDisplay;
//                        qDebug() << "Third " << currentThirdLineDisplay;
//                        qDebug() << "Fourth" << currentFourthLineDisplay << floor(j/2);
//                        //qDebug() << hairpinScore;
//                     }




                }

                float jFloat = float(j);
                if (k < jFloat/2.-1)
                {
                    //THESE TWO LINES MAY BE NECESSARY FOR EVENS
//                    currentSecondLineDisplay.append(" " );
//                    currentThirdLineDisplay.append(" ");


//                    qDebug() << "Current window =" << slidingWindow[i] << "J = " << j;
//                    qDebug() << currentFirstLineDisplay;
//                    qDebug() << currentSecondLineDisplay;
//                    qDebug() << currentThirdLineDisplay;
//                    qDebug() << currentFourthLineDisplay;
                }

                if (k == j-1 && j%2 == 0) //this probably should be limited to those with even loop bases
                {
                    currentSecondLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i])+slidingWindow[i].size()+ ceil(j/2. - 1. ), 1 ) );
                    currentThirdLineDisplay.append(sequence_.mid(sequence_.indexOf(slidingWindow[i]) + slidingWindow[i].size()+floor(j/2. + 1.), 1 ));

//                    qDebug() << "Current window =" << slidingWindow[i] << "J = " << j;
//                    qDebug() << currentFirstLineDisplay;
//                    qDebug() << currentSecondLineDisplay;
//                    qDebug() << currentThirdLineDisplay;
//                    qDebug() << currentFourthLineDisplay;
                    //qDebug() << "LAST IF (3rd): " << currentThirdLineDisplay;
                }
            }


            qDebug() << "Current window =" << slidingWindow[i] << "J = " << j;
            qDebug() << currentFirstLineDisplay;
            qDebug() << currentSecondLineDisplay;
            qDebug() << currentThirdLineDisplay;
            qDebug() << currentFourthLineDisplay;

            if(hairpinScore > highestHairpinScore_)
            {
                highestHairpinScore_ = hairpinScore;  //if hairpinScore is higher than the best score yet, then set highestHairpinScore_ equal to it

                hairpinDiagram_.clear();
                hairpinDiagram_ << currentFirstLineDisplay << currentSecondLineDisplay << currentThirdLineDisplay << currentFourthLineDisplay;
                //qDebug() << "Current J = " <<j;

                //qDebug() << hairpinDiagram_;
                //qDebug() << "First " << currentFirstLineDisplay << ceil(j/2 - 1 );
                //qDebug() << "Second" << currentSecondLineDisplay;
                //qDebug() << "Third " << currentThirdLineDisplay;
                //qDebug() << "Fourth" << currentFourthLineDisplay << floor(j/2) -1 ;
            }
        }
    }
    qDebug() << slidingWindow;
    return highestHairpinScore_;
}

QList<QString> DnaSecondaryStructure::hairpinDisplay()
{
    if(hairpinDiagram_.isEmpty())
    {
        hairpinDiagram_.prepend("Please analyze a sequence first");
    }


    return hairpinDiagram_;
}

/* reports highest basepairing configuration of two linear sequences
   dimerScore represents the number of H-bonds formed in the optimal configuration
   currently, this function only reports a diagram of the first identified configuration
   with the highest dimerScore; thus, a user might not be see all potential configurations
   but only the first identified in the algorithm
*/
dimerContainer DnaSecondaryStructure::dimer(QString sequence1, QString sequence2)
{
    QList<QString> dimerDisplayLines;

    QString sequence1ForwardComparisonWindow, sequence2ForwardComparisonWindow, sequence1ReverseComparisonWindow, sequence2ReverseComparisonWindow;
    int sequence1Length = sequence1.length(), sequence2Length = sequence2.length();

    // sequence lengths are resized with the mask character N
    if(sequence1Length < sequence2Length) { sequence1 += QString(sequence2Length - sequence1Length, 'N');}
    else if(sequence1Length > sequence2Length){ sequence2 += QString(sequence1Length - sequence2Length, 'N');}

    // create the reverse of sequence2 (3'->5')
    DnaString sequence2DnaString = sequence2;
    QString sequence2DnaStringReverse = sequence2DnaString.reverse();

    int maximumDimerScore = 0;
    QString maximumFirstLineDisplay, maximumSecondLineDisplay, maximumThirdLineDisplay;

    // iterate through sequence1
    for(int i = 0; i < sequence1Length; ++i)
    {
        // extract the regions of each sequence that will be compared
        // sequence1pieceF and sequence2pieceF are windows comparing sliding the window in one direction
        QString sequence1pieceF = sequence1.mid(i);
        QString sequence2pieceF = sequence2DnaStringReverse.mid(0,sequence2DnaStringReverse.length()-i);

        // while sequence1pieceR and sequence2pieceR are windows sliding the window in the opposite direction
        QString sequence1pieceR = sequence1.mid(0,sequence1.length()-i);
        QString sequence2pieceR = sequence2DnaStringReverse.mid(i);

        //initialize variables for scoring and display
        int currentDimerScore = 0;
        QString currentFirstLineDisplay, currentSecondLineDisplay, currentThirdLineDisplay;

        //set up current display lines
        currentFirstLineDisplay.append(sequence1);
        currentFirstLineDisplay += QString(i,' ');
        currentSecondLineDisplay += QString(i, ' ');
        currentThirdLineDisplay.append(sequence2DnaStringReverse);

        // iterate through the positions in the sequence, comparing each
        for(int j = 0; j < sequence1pieceF.length(); ++j)
        {
            // if they base-pair, add the relevant # of H-bonds to currentDimerScore
            if(sequence1pieceF.at(j) == constants::kDnaBasePair[sequence2pieceF.at(j)])
            {
                currentSecondLineDisplay.append('|'); // they match, so add a pipe in to the second line

                if(sequence1pieceF[j] == 'G' || sequence1pieceF[j] == 'C'){currentDimerScore = currentDimerScore + 3;} // G's or C's get 3 points as they form 3 hydrogen bonds
                else{currentDimerScore = currentDimerScore + 2;}  // A's or T's get 2 points with 2 hydrogen bonds
            }

            else {currentSecondLineDisplay.append(' ');}
        }
        //if this is the highest score yet, set maximumDimerScore equal to currentDimerScore
        if(currentDimerScore > maximumDimerScore)
        {
            maximumDimerScore = currentDimerScore;

            maximumFirstLineDisplay = currentFirstLineDisplay;
            maximumSecondLineDisplay = currentSecondLineDisplay;
            maximumSecondLineDisplay.append(QString(i, ' '));
            maximumThirdLineDisplay = currentThirdLineDisplay;
            maximumThirdLineDisplay.prepend(QString(i, ' '));
        }
        currentThirdLineDisplay.prepend(QString(i, ' ')); //this is in the proper place


        // clear current line displays for next iteration in the opposite direction
        currentFirstLineDisplay.clear();
        currentSecondLineDisplay.clear();
        currentThirdLineDisplay.clear();

        currentFirstLineDisplay.append(sequence1);
        currentFirstLineDisplay.prepend(QString(i,' '));
        currentSecondLineDisplay += QString(i, ' ');
        currentThirdLineDisplay.append(sequence2DnaStringReverse);
        currentThirdLineDisplay.append(QString(i, ' '));

        // iterate through the sequence window moving the opposite direction
        currentDimerScore = 0; // reset currentDimerScore to 0
        for(int j = 0; j < sequence1pieceR.length(); ++j)
        {
            // iterate through the positions in the sequence, comparing each
            if(sequence1pieceR.at(j) == constants::kDnaBasePair[sequence2pieceR.at(j)])
            {
                currentSecondLineDisplay.append('|');
                if(sequence1pieceR[j] == 'G' || sequence1pieceR[j] == 'C'){currentDimerScore = currentDimerScore + 3;} // G's or C's get 3 points as they form 3 hydrogen bonds
                else{currentDimerScore = currentDimerScore + 2;}  // A's or T's get 2 points with 2 hydrogen bonds
            }

            else{ currentSecondLineDisplay.append(' '); }
        }

        // if this is the highest score yet, set maximumDimerScore equal to currentDimerScore
        if(currentDimerScore > maximumDimerScore)
        {
            maximumDimerScore = currentDimerScore;
            maximumFirstLineDisplay = currentFirstLineDisplay;
            maximumSecondLineDisplay = currentSecondLineDisplay;
            maximumSecondLineDisplay.append(QString(i, ' '));
            maximumThirdLineDisplay = currentThirdLineDisplay;
        }
    }

    // Replace all N's with a space; this does leave some trailing or leading spaces in
    // the final results; ideally, they would be removed
    maximumFirstLineDisplay.replace(QRegExp("N"), " ");
    maximumThirdLineDisplay.replace(QRegExp("N"), " ");

    dimerDisplayLines << maximumFirstLineDisplay << maximumSecondLineDisplay << maximumThirdLineDisplay;

    // create a dimerContainer object to return the data
    dimerContainer currentDimer;
    currentDimer.displayLines_ = dimerDisplayLines;
    currentDimer.dimerScore_ = maximumDimerScore;

    // return the results to the calling function
    return currentDimer;
}
