/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QMap>
#include <QtCore/QVector>

#include "BioSymbolGroup.h"
#include "misc.h"
#include "macros.h"

static int registerMetaTypes()
{
    qRegisterMetaType<BioSymbolGroup>("BioSymbolGroup");
    qRegisterMetaTypeStreamOperators<BioSymbolGroup>("BioSymbolGroup");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetaTypes);


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Operator methods
/**
  * @param bioSymbol [const BioSymbol &]
  * @returns BioSymbolGroup &
  */
BioSymbolGroup &BioSymbolGroup::operator<<(const BioSymbol &bioSymbol)
{
    addBioSymbol(bioSymbol);

    return *this;
}

BioSymbolGroup &BioSymbolGroup::operator<<(const QVector<BioSymbol> &bioSymbols)
{
    foreach (const BioSymbol &bioSymbol, bioSymbols)
        addBioSymbol(bioSymbol);

    return *this;
}

/**
  * @param other [const BioSymbolGroup &]
  * @returns bool
  */
bool BioSymbolGroup::operator==(const BioSymbolGroup &other) const
{
    return this == &other || bioSymbols_ == other.bioSymbols_;
}

/**
  * @param other [const BioSymbolGroup &]
  * @returns bool
  */
bool BioSymbolGroup::operator!=(const BioSymbolGroup &other) const
{
    return !operator==(other);
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
    bioSymbolSerialNumbers_.insert(bioSymbol.symbol(), nextSerialNumber());
}

/**
  * @returns QHash<char, BioSymbol>
  */
QHash<char, BioSymbol> BioSymbolGroup::bioSymbols() const
{
    return bioSymbols_;
}

QVector<BioSymbol> BioSymbolGroup::bioSymbolVector() const
{
    QVector<BioSymbol> symbolVector;
    QVector<char> orderedSymbols = symbolsInSerialOrder();
    foreach (const char symbol, orderedSymbols)
        symbolVector << bioSymbols_.value(symbol);
    return symbolVector;
}

/**
  */
void BioSymbolGroup::clear()
{
    bioSymbols_.clear();
    bioSymbolSerialNumbers_.clear();
}

/**
  * @returns int
  */
int BioSymbolGroup::count() const
{
    return bioSymbols_.count();
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
    bioSymbols_.remove(symbol);
}

/**
  * @param ch [char]
  * @returns int
  */
int BioSymbolGroup::serialNumber(char ch) const
{
    if (bioSymbolSerialNumbers_.contains(ch))
    {
        ASSERT(bioSymbols_.contains(ch));
        return bioSymbolSerialNumbers_.value(ch);
    }

    return 0;
}

void BioSymbolGroup::setThresholdForAllBioSymbols(const double newThreshold)
{
    ASSERT(newThreshold >= 0. && newThreshold <= 1.);
    QHash<char, BioSymbol>::Iterator it = bioSymbols_.begin();
    for (; it != bioSymbols_.end(); ++it)
        it.value().setThreshold(newThreshold);
}

QVector<char> BioSymbolGroup::symbolsInSerialOrder() const
{
    QMap<int, char> map;

    QHash<char, int>::ConstIterator it = bioSymbolSerialNumbers_.constBegin();
    for (; it != bioSymbolSerialNumbers_.constEnd(); ++it)
        map.insert(it.value(), it.key());

    return map.values().toVector();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns int
  */
int BioSymbolGroup::nextSerialNumber() const
{
    static int i = 0;

    return ++i;
}


QDataStream &operator<<(QDataStream &out, const BioSymbolGroup &bioSymbolGroup)
{
    out << bioSymbolGroup.bioSymbolVector();
    return out;
}

QDataStream &operator>>(QDataStream &in, BioSymbolGroup &bioSymbolGroup)
{
    QVector<BioSymbol> bioSymbols;
    in >> bioSymbols;
    bioSymbolGroup << bioSymbols;
    return in;
}
