#include "PrimerPairFinder.h"

using namespace PrimerDesign;

class ReverseSearcher
{
public:
    int left;
    int right;

    ReverseSearcher(QList<Primer > data)
    {
        data_ = data;

        qSort(data_.begin(), data_.end(), tmComparer);
    }

    void setTm(double tm)
    {
        const int PRIMER_PAIR_SEARCH_WINDOW = 100;
        int bestMatch = findClosest(tm, 0, data_.length() - 1);
        left = qMax(0, bestMatch - PRIMER_PAIR_SEARCH_WINDOW);
        right = qMin(data_.length(), bestMatch + PRIMER_PAIR_SEARCH_WINDOW);
    }

private:
    QList<Primer > data_;

    static bool tmComparer(const Primer &p1, const Primer &p2)
    {
        return p1.tm() < p2.tm();
    }

    inline int midpoint(int left, int right)
    {
        return ((right - left) / 2) + left;
    }

    int findClosest(double tm, int left, int right)
    {
        if (left >= right)
        {
            return left;
        }

        int mid = midpoint(left, right);

        if (tm < data_.at(mid).tm())
        {
            return findClosest(tm, left, mid - 1);
        }

        if (tm > data_.at(mid).tm())
        {
            return findClosest(tm, mid + 1, right);
        }

        return mid;
    }
};

class PrimerPairResult : public PrimerPairFinderResult
{
public:
    void add(const PrimerPair &pair)
    {
        const int MAX_LIST_SIZE = 50;

        if (value.count() < MAX_LIST_SIZE)
        {
            value.append(pair);

            if (value.count() == MAX_LIST_SIZE)
            {
                moveWorstToFront();
            }
        }
        else if (pair.score() < value.at(0).score())
        {
            value[0] = pair;
            moveWorstToFront();
        }
    }

private:
    void moveWorstToFront()
    {
        double worstScore = 0;
        int worstScoreIndex = -1;

        for (int i = 0; i < value.count(); ++i)
        {
            if (value.at(i).score() > worstScore)
            {
                worstScore = value.at(i).score();
                worstScoreIndex = i;
            }
        }

        PrimerPair p = value.at(worstScoreIndex);
        value.removeAt(worstScoreIndex);
        value.prepend(p);
    }
};

PrimerPairFinderResult PrimerPairFinderResult::error(const QString &message)
{
    PrimerPairFinderResult result;
    result.isError = true;
    result.errorMessage = message;
    return result;
}

PrimerPairFinderResult PrimerPairFinderResult::valid(QList<PrimerPair> value)
{
    PrimerPairFinderResult result;
    result.isError = false;
    result.value = value;
    return result;
}

PrimerPairFinder::PrimerPairFinder(PrimerDesignInput *input)
{
    input_ = input->toZeroBased();
    amplicon_ = input_.getBoundedAmplicon();
    cancelling_ = false;
}

PrimerPairFinderResult PrimerPairFinder::findPrimerPairs()
{
    QString errorMessage = input_.getErrorMessage();
    if (errorMessage.isEmpty())
    {
        return performPrimerPairComputation();
    }

    return PrimerPairFinderResult::error(errorMessage);
}

void PrimerPairFinder::cancel()
{
    cancelling_ = true;
}

PrimerPairFinderResult PrimerPairFinder::performPrimerPairComputation()
{
    for (int currentPrimerSize = input_.primerSizeRange.min();
         currentPrimerSize <= input_.primerSizeRange.max() && !cancelling_;
         ++currentPrimerSize)
    {
        computeForwardPrimers(currentPrimerSize);
        computeReversePrimers(currentPrimerSize);
    }

    if (forwardPrimers.isEmpty())
    {
        return PrimerPairFinderResult::error("No forward primers were found.");
    }

    if (reversePrimers.isEmpty())
    {
        return PrimerPairFinderResult::error("No reverse primers were found.");
    }

    return getPairedPrimers();
}

void PrimerPairFinder::computeForwardPrimers(int currentPrimerSize)
{
    int maxFowardPosition = amplicon_.length() - input_.ampliconSizeRange.min();
    QRegExp suffix(input_.forwardSuffix);

    for (int i = 0; i < maxFowardPosition && !cancelling_; ++i)
    {
        QString forwardPrimer = amplicon_.mid(i, currentPrimerSize);
        if (forwardPrimerIsUnique(forwardPrimer))
        {
            validateAndStorePrimer(
                forwardPrimer, input_.forwardPrefix, suffix, i, forwardPrimers);
        }
    }
}

void PrimerPairFinder::computeReversePrimers(int currentPrimerSize)
{
    int maxReversePosition = amplicon_.length() - input_.ampliconSizeRange.min();
    qDebug() << maxReversePosition;
    QString reverseAmplicon = DnaString(amplicon_).reverseComplement();
    QRegExp suffix(input_.reverseSuffix);

    for (int i = 0; i <= maxReversePosition && !cancelling_; ++i)
    {
        QString reversePrimer = reverseAmplicon.mid(i, currentPrimerSize);
        qDebug() << i<< reversePrimer;
        validateAndStorePrimer(reversePrimer, input_.reversePrefix, suffix, amplicon_.length() - i, reversePrimers);
    }
}

PrimerPairFinderResult PrimerPairFinder::getPairedPrimers()
{
    PrimerPair pair(PrimerDesignInputRef(new PrimerDesignInput(input_.toOneBased())));
    ReverseSearcher searchWindow(reversePrimers);
    PrimerPairResult result;

    for (int f = 0; f < forwardPrimers.count() && !cancelling_; ++f)
    {
        pair.setForwardPrimer(forwardPrimers.at(f));
        searchWindow.setTm(forwardPrimers.at(f).tm());

        for (int r = searchWindow.left; r < searchWindow.right && !cancelling_; ++r)
        {
            pair.setReversePrimer(reversePrimers.at(r));
            if (input_.ampliconSizeRange.contains(pair.ampliconLength()-1)) //2011-05-18 (Paul) had to had the -1 to correctly capture the whole sequence; this apparently is indexed incorrectly in the original definictions by Chris
            {
                pair.recomputePrimerPairScore();
                result.add(pair);
            }
        }
    }

    if (result.value.count())
    {
        return result;
    }

    return PrimerPairFinderResult::error("No primer pairs could be found.");
}

bool PrimerPairFinder::forwardPrimerIsUnique(const QString &forwardPrimer)
{
    int position = amplicon_.indexOf(forwardPrimer);
    return amplicon_.indexOf(forwardPrimer, position + 1) < 0;
}

void PrimerPairFinder::validateAndStorePrimer(
    const QString &primerSequence,
    const QString &prefix,
    const QRegExp &suffix,
    int sequencePosition,
    QList<Primer> &collection)
{
    int suffixIndex = primerSequence.length() - 3;

    if (suffix.indexIn(primerSequence, suffixIndex) == suffixIndex)
    {
        Primer p(prefix + primerSequence, input_);
        if (input_.tmRange.contains(p.tm()))
        {
            p.setSequencePosition(sequencePosition);
            collection.append(p);
        }
    }
}
