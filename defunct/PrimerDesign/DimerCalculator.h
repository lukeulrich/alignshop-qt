/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef DIMERCALCULATOR_H
#define DIMERCALCULATOR_H

#include <QtCore>

namespace PrimerDesign
{
    /// Computes the dimer score for a pair of sequences.
    /// The score is based on a variety of factors such as delta TM, "stickiness", etc.
    class DimerCalculator
    {
    public:
        /// Computes the dimer score for the specified sequence(s)
        /// @param seq1 QString
        /// @param seq2 QString
        /// @return int
        static int score(QString seq1, QString seq2);

    private:
        /// Computes the score for the specified sequences.
        /// This moves forward through the first sequence and backward through the second sequence's reverse
        /// compliment and produces a score based on the overlapping characters.
        /// @param seq1 const QChar *
        /// @param seq2 const QChar *
        /// @param last1 const QChar *
        /// @param last2 const QChar *
        /// @return int
        static inline int score(const QChar *seq1, const QChar *seq2, const QChar *last1, const QChar *last2)
        {
            int score = 0;

            while (seq1 <= last1 && seq2 <= last2)
            {
                if (*seq1 == reverseCompliment(*last2))
                {
                    score += scoreOf(*seq1);
                }

                ++seq1;
                --last2;
            }

            return score;
        }

        /// Gives the reverse compliment value of the specified char.
        /// @param ch QChar
        /// @return char
        static inline char reverseCompliment(QChar ch)
        {
            return ch == 'A' ? 'T' :
                   ch == 'T' ? 'A' :
                   ch == 'G' ? 'C' :
                   ch == 'C' ? 'G' :
                   '?';
        }

        /// Gives the score of the specified char.
        /// @param ch QChar
        /// @return int
        static inline int scoreOf(QChar ch)
        {
            return ch == 'G' || ch == 'C' ? 3 : 2;
        }
    };
}

#endif // DIMERCALCULATOR_H
