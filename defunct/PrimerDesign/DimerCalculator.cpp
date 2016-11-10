#include "DimerCalculator.h"

using namespace PrimerDesign;

int DimerCalculator::score(QString seq1, QString seq2)
{
    int maxScore = 0;
    const QChar *seq1Start = seq1.constData();
    const QChar *seq2Start = seq2.constData();
    const QChar *seq1Last = seq1Start + seq1.length() - 1;
    const QChar *seq2Last = seq2Start + seq2.length() - 1;

    for (int i = 0; i < seq1.length(); ++i)
    {
        maxScore = qMax(maxScore, score(seq1Start + i, seq2Start, seq1Last, seq2Last));
    }

    for (int i = 1; i < seq2.length(); ++i)
    {
        maxScore = qMax(maxScore, score(seq1Start, seq2Start + i, seq1Last, seq2Last));
    }

    return maxScore;
}
