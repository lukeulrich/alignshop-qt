/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SymbolStringCalculator.h"
#include "../BioSymbol.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private helper class
/**
  * BioSymbolThresholdSerialLessThanPrivate simply provides a functor style interface for sorting BioSymbols first by
  * their actual proportions and then if they are equivalent give precedence to the most recent (last inserted) rule.
  */
class BioSymbolThresholdSerialLessThanPrivate
{
public:
    BioSymbolThresholdSerialLessThanPrivate(const SymbolStringCalculator *calculator) : calculator_(calculator)
    {
    }

    //! Core comparison method; first = symbol, second = proportion
    bool operator()(const PairCharDouble &a, const PairCharDouble &b)
    {
        return a.second < b.second ||
                (a.second == b.second &&
                 calculator_->bioSymbolGroup_.serialNumber(a.first) > calculator_->bioSymbolGroup_.serialNumber(b.first));
    }

private:
    const SymbolStringCalculator *calculator_;
};


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param bioSymbolGroup [const BioSymbolGroup &]
  * @param defaultSymbol [const char]
  */
SymbolStringCalculator::SymbolStringCalculator(const BioSymbolGroup &bioSymbolGroup, const char defaultSymbol)
    : bioSymbolGroup_(bioSymbolGroup), defaultSymbol_(defaultSymbol)
{
    buildCharSymbolAssociation();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
/**
  * @param other [const SymbolStringCalculator &]
  * @returns bool
  */
bool SymbolStringCalculator::operator==(const SymbolStringCalculator &other) const
{
    return (this == &other) || (bioSymbolGroup_ == other.bioSymbolGroup_ && defaultSymbol_ == other.defaultSymbol_);
}

/**
  * @param other [const SymbolStringCalculator &]
  * @returns bool
  */
bool SymbolStringCalculator::operator!=(const SymbolStringCalculator &other) const
{
    return !operator==(other);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioSymbolGroup
  */
BioSymbolGroup SymbolStringCalculator::bioSymbolGroup() const
{
    return bioSymbolGroup_;
}

/**
  * Analyzes each character's amount in each column of vectorHashCharDouble relative to a required BioSymbol threshold.
  * If at least one BioSymbol matches the column contents, then the highest matching BioSymbol's symbol is used for this
  * column. If the character count distribution in this column does not surpass any of the BioSymbol's thresholds, the
  * defaultSymbol_ character is used. This process is done for each column and results in a symbol string, which is
  * returned.
  *
  * Overarching principle when choosing BioSymbolGroup rules: If two or more BioSymbol's thresholds are simultaneously
  * matched, divide each distribution value by the number of characters in that BioSymbol and take the highest (most
  * specific, that is, the one with the fewest characters) BioSymbol. If there is a tie, take the first rule defined.
  *
  * It is assumed that the sum of all values in each column is <= 1, otherwise the behavior is undefined.
  *
  * @param vectorHashCharDouble [const VectorHashCharDouble &]
  * @returns QByteArray
  */
QByteArray SymbolStringCalculator::computeSymbolString(const VectorHashCharDouble &vectorHashCharDouble) const
{
    QByteArray symbolString;
    const QHash<char, BioSymbol> &bioSymbols = bioSymbolGroup_.bioSymbols();

    // it = iterator
    // *it = QHash<char, qreal>
    VectorHashCharDouble::ConstIterator it;
    for (it = vectorHashCharDouble.constBegin(); it != vectorHashCharDouble.constEnd(); ++it)
    {
        // Now looking at one single column recognized by it

        // ----------------------------------------
        // ----------------------------------------
        // Only perform the sum check in debug mode
#ifdef QT_DEBUG
        {
            // Check that all values in this column sum to <= 1
            qreal sum = 0.;
            QHash<char, qreal>::ConstIterator j_it = it->constBegin();
            while (j_it != it->constEnd())
            {
                sum += j_it.value();
                ++j_it;
            }
            ASSERT_X(sum <= 1. || qFuzzyCompare(sum, 1.), "Sum of values in column must be <= 1");
        }
#endif
        // ----------------------------------------
        // ----------------------------------------

        // Sum the different symbol amounts
        QHash<char, qreal> symbolProportions;

        QHash<char, qreal>::ConstIterator j_it = it->constBegin();
        // j_it.key() = character
        // j_it.value() = proportion
        while (j_it != it->constEnd())
        {
            char ch = j_it.key();

            // Map this character to all symbols it belongs to
            if (charSymbolAssociation_.contains(ch))
                for (const char *x = charSymbolAssociation_.value(ch); *x; ++x)
                    symbolProportions[*x] += j_it.value();

            ++j_it;
        }

        // Build vector of matching symbol groups
        QVector<PairCharDouble> matchingSymbols;
        j_it = symbolProportions.constBegin();
        for (; j_it != symbolProportions.constEnd(); ++j_it)
        {
            ASSERT(bioSymbols.contains(j_it.key()));

            // j_it.key() = symbol
            // j_it.value() = actual proportion found
            if (j_it.value() >= bioSymbols.value(j_it.key()).threshold())
                matchingSymbols << qMakePair(j_it.key(), j_it.value());
        }

        // If no rules were matched
        if (matchingSymbols.isEmpty())
        {
            symbolString += defaultSymbol_;
            continue;
        }

        // Or, if only a single rule was matched, use its symbol
        if (matchingSymbols.count() == 1)
        {
            symbolString += bioSymbols.value(matchingSymbols.at(0).first).symbol();
            continue;
        }

        // Otherwise, determine the winning symbol
        // 1) Compute the effective threshold (actual proportion of this symbol divided by the number of
        //    characters it contains) for each matching BioSymbol
        QVector<PairCharDouble> matchingSymbolsEffectiveThresholds;
        foreach (const PairCharDouble &matchingSymbol, matchingSymbols)
        {
            int nCharacters = bioSymbols.value(matchingSymbol.first).characterSet().size();

            matchingSymbolsEffectiveThresholds << qMakePair(matchingSymbol.first, matchingSymbol.second / static_cast<qreal>(nCharacters));
        }

        // 2) Sort by the effective thresholds
//        qDebug() << "Before sort:" << matchingSymbolsEffectiveThresholds;
        qStableSort(matchingSymbolsEffectiveThresholds.begin(),
                    matchingSymbolsEffectiveThresholds.end(),
                    BioSymbolThresholdSerialLessThanPrivate(this));
//        qDebug() << "After sort:" << matchingSymbolsEffectiveThresholds;
//        for (int i=0; i< matchingSymbolsEffectiveThresholds.count(); ++i)
//            qDebug() << matchingSymbolsEffectiveThresholds.at(i).first << bioSymbolInsertOrder_.value(matchingSymbolsEffectiveThresholds.at(i).first);

        // 3) Last one should be our winner - unless there is a tie :)
        symbolString += bioSymbols.value(matchingSymbolsEffectiveThresholds.last().first).symbol();
    }

    return symbolString;
}

/**
  * @returns char
  */
char SymbolStringCalculator::defaultSymbol() const
{
    return defaultSymbol_;
}

/**
  * @param bioSymbolGroup [const BioSymbolGroup &]
  */
void SymbolStringCalculator::setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup)
{
    bioSymbolGroup_ = bioSymbolGroup;
    buildCharSymbolAssociation();
}

/**
  * @param defaultSymbol [const char]
  */
void SymbolStringCalculator::setDefaultSymbol(const char defaultSymbol)
{
    defaultSymbol_ = defaultSymbol;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Given a symbol group, essentially reverse its data structure so that it is possible to find all symbols associated
  * with a particular character.
  *
  * Given the BioSymbolGroup:
  *   Symbol        Characters
  *   p             KE
  *   +             KR
  *   -             DE
  *
  * This method constructs the inverse:
  *   Character     Symbols
  *   K             p+
  *   E             p-
  *   D             -
  *   R             +
  */
void SymbolStringCalculator::buildCharSymbolAssociation()
{
    charSymbolAssociation_.clear();

    const QHash<char, BioSymbol> &bioSymbols = bioSymbolGroup_.bioSymbols();
    QHash<char, BioSymbol>::ConstIterator it = bioSymbols.constBegin();
    for (; it != bioSymbols.constEnd(); ++it)
    {
        // it.key() => char
        // it.value() => BioSymbol
        const QByteArray &characters = it.value().characters();
        const char *x = characters.constData();
        while (*x)
        {
            ASSERT(it.key() == it.value().symbol());
            charSymbolAssociation_[*x].append(it.key());
            ++x;
        }
    }
}


