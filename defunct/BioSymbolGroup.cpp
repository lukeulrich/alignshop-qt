/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "BioSymbolGroup.h"

// #include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param defaultSymbol [char]
  */
BioSymbolGroup::BioSymbolGroup(const char defaultSymbol) : defaultSymbol_(defaultSymbol)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  *
  *
  * @param bioSymbol [const BioSymbol &]
  * @returns BioSymbolGroup &
  */
BioSymbolGroup &BioSymbolGroup::operator<<(const BioSymbol &bioSymbol)
{
    addBioSymbol(bioSymbol);

    return *this;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * No two BioSymbols with the same symbol character may be present in a group at the same time. Therefore, this method
  * adds bioSymbol to the group if it does not already have an entry for its symbol. Otherwise, it replaces that entry
  * in the hash with the BioSymbol passed into this method.
  *
  * @param bioSymbol [const BioSymbol &]
  */
void BioSymbolGroup::addBioSymbol(const BioSymbol &bioSymbol)
{
    bioSymbols_.insert(bioSymbol.symbol(), bioSymbol);

    // Track when this symbol was inserted
    bioSymbolInsertOrder_.insert(bioSymbol.symbol(), nextInsertNumber());

    // Add all bioSymbol characters to the symbol lookup list for easily determining all the symbols
    // a character is associated with
    foreach (char ch, bioSymbol.characterSet().toList())
        charSymbolLookup_[ch].insert(bioSymbol.symbol());
}

/**
  * @returns QHash<char, BioSymbol>
  */
QHash<char, BioSymbol> BioSymbolGroup::bioSymbols() const
{
    return bioSymbols_;
}

/**
  * Analyzes each character's amount in each column of listHashCharDouble relative to a required BioSymbol threshold. If
  * at least one BioSymbol matches the column contents, then the highest matching BioSymbol's symbol is used for this
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
  * @param listHashCharDouble [const ListHashCharDouble &]
  * @return QString
  */
QString BioSymbolGroup::calculateSymbolString(const ListHashCharDouble &listHashCharDouble) const
{
    QString symbolString;

    // it = iterator
    // *it = QHash<char, qreal>
    ListHashCharDouble::const_iterator it;
    for (it = listHashCharDouble.begin(); it != listHashCharDouble.end(); ++it)
    {
        // Now looking at one single column recognized by it

        // ----------------------------------------
        // ----------------------------------------
        // Only perform the sum check in debug mode
#ifdef QT_DEBUG
        {
            // Check that all values in this column sum to <= 1
            qreal sum = 0.;
            QHash<char, qreal>::const_iterator j_it = it->constBegin();
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

        QHash<char, qreal>::const_iterator j_it = it->constBegin();
        // j_it.key() = character
        // j_it.value() = proportion
        while (j_it != it->constEnd())
        {
            char ch = j_it.key();

            // Map this character to all symbols it belongs to
            if (charSymbolLookup_.contains(ch))
                foreach (char symbol, charSymbolLookup_[ch].toList())
                    symbolProportions[symbol] += j_it.value();

            ++j_it;
        }

        // Build list of matching symbol groups
        QList<PairCharDouble> matchingSymbols;
        j_it = symbolProportions.constBegin();
        while (j_it != symbolProportions.constEnd())
        {
            ASSERT(bioSymbols_.contains(j_it.key()));

            // j_it.key() = symbol
            // j_it.value() = actual proportion found
            if (j_it.value() >= bioSymbols_.value(j_it.key()).threshold())
            {
                // Yes! We have a matching rule!
                matchingSymbols << qMakePair(j_it.key(), j_it.value());
            }

            ++j_it;
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
            symbolString += bioSymbols_.value(matchingSymbols.at(0).first).symbol();
            continue;
        }

        // Otherwise, determine the winning symbol
        // 1) Compute the effective threshold (actual proportion of this symbol divided by the number of
        //    characters it contains) for each matching BioSymbol
        QList<PairCharDouble> matchingSymbolsEffectiveThresholds;
        foreach (const PairCharDouble &matchingSymbol, matchingSymbols)
        {
            int nCharacters = bioSymbols_.value(matchingSymbol.first).characterSet().size();

            matchingSymbolsEffectiveThresholds << qMakePair(matchingSymbol.first, matchingSymbol.second / static_cast<qreal>(nCharacters));
        }

        // 2) Sort by the effective thresholds
//        qDebug() << "Before sort:" << matchingSymbolsEffectiveThresholds;
        qStableSort(matchingSymbolsEffectiveThresholds.begin(), matchingSymbolsEffectiveThresholds.end(), BioSymbolThresholdInsertOrderLessThan(this));
//        qDebug() << "After sort:" << matchingSymbolsEffectiveThresholds;
//        for (int i=0; i< matchingSymbolsEffectiveThresholds.count(); ++i)
//            qDebug() << matchingSymbolsEffectiveThresholds.at(i).first << bioSymbolInsertOrder_.value(matchingSymbolsEffectiveThresholds.at(i).first);

        // 3) Last one should be our winner - unless there is a tie :)
        symbolString += bioSymbols_.value(matchingSymbolsEffectiveThresholds.last().first).symbol();
    }

    return symbolString;
}

/**
  */
void BioSymbolGroup::clear()
{
    bioSymbols_.clear();
}

/**
  * @returns int
  */
int BioSymbolGroup::count() const
{
    return bioSymbols_.count();
}

/**
  * @returns char
  */
char BioSymbolGroup::defaultSymbol() const
{
    return defaultSymbol_;
}

/**
  * @param character [char]
  * @param symbol [char]
  * @returns bool
  */
bool BioSymbolGroup::isCharAssociatedWithSymbol(char character, char symbol) const
{
    if (!bioSymbols_.contains(symbol))
        return false;

    return bioSymbols_.value(symbol).hasCharacter(character);
}

/**
  * @returns bool
  */
bool BioSymbolGroup::isEmpty() const
{
    return bioSymbols_.isEmpty();
}

/**
  * @param symbol [char]
  */
void BioSymbolGroup::removeBioSymbol(char symbol)
{
    if (!bioSymbols_.contains(symbol))
        return;

    BioSymbol bioSymbol = bioSymbols_.take(symbol);

    // Remove this symbol for all bioSymbol characters from the lookup list
    foreach (char ch, bioSymbol.characterSet().toList())
    {
        ASSERT_X(charSymbolLookup_.contains(ch), "Reverse character -> symbol lookup hash incompletely constructed - missing character");
        ASSERT_X(charSymbolLookup_[ch].contains(bioSymbol.symbol()), "Reverse character -> symbol lookup hash incompletely constructed - missing symbol");
        charSymbolLookup_[ch].remove(bioSymbol.symbol());
    }
}

/**
  * @param defaultSymbol [char]
  */
void BioSymbolGroup::setDefaultSymbol(char defaultSymbol)
{
    defaultSymbol_ = defaultSymbol;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int BioSymbolGroup::nextInsertNumber() const
{
    static int i = 0;

    return ++i;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private class: BioSymbolThresholdInsertOrderLessThan
/**
  * @param self [const BioSymbolGroup *]
  */
BioSymbolGroup::BioSymbolThresholdInsertOrderLessThan::BioSymbolThresholdInsertOrderLessThan(const BioSymbolGroup *self)
    : self_(self)
{
}

/**
  * Used for sorting symbol - proportion pairs by the proportion value.
  *
  * @param a [const QPair<char, qreal> &]
  * @param b [const QPair<char, qreal> &]
  * @returns bool
  * @see calculateSymbolString()
  */
bool BioSymbolGroup::BioSymbolThresholdInsertOrderLessThan::operator()(const QPair<char, qreal> &a, const QPair<char, qreal> &b)
{
    // Sort first by proportion ascending and then by insert order if equivalent
    return a.second < b.second
           || (a.second == b.second
               && self_->bioSymbolInsertOrder_.value(a.first) > self_->bioSymbolInsertOrder_.value(b.first));
}
