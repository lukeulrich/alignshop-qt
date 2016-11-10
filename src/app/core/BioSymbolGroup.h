/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSYMBOLGROUP_H
#define BIOSYMBOLGROUP_H

#include <QtCore/QHash>
#include <QtCore/QMetaType>
#include "BioSymbol.h"

/**
  * BioSymbolGroup manages a collection of BioSymbols and provides a convenient mechanism for testing if a character
  * belongs to a particular symbol.
  */
class BioSymbolGroup
{
public:
    // ------------------------------------------------------------------------------------------------
    // Operators
    BioSymbolGroup &operator<<(const BioSymbol &bioSymbol);                 //!< Convenience method that adds bioSymbol to this group (or replaces an existing version if it already exists) and returns a reference to this object
    BioSymbolGroup &operator<<(const QVector<BioSymbol> &bioSymbols);

    bool operator==(const BioSymbolGroup &other) const;                     //!< Returns true if other is equivalent to this group; false otherwise
    bool operator!=(const BioSymbolGroup &other) const;                     //!< Returns true if other is not equivalent to this group; false otherwise

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void addBioSymbol(const BioSymbol &bioSymbol);                          //!< Adds bioSymbol to this group (or replaces an existing version if it already exists)
    QHash<char, BioSymbol> bioSymbols() const;                              //!< Returns the hash of BioSymbols
    QVector<BioSymbol> bioSymbolVector() const;                             //!< Returns the member BioSymbols ordered accordng to the order they were added
    void clear();                                                           //!< Clears all BioSymbols
    int count() const;                                                      //!< Returns the number of BioSymbols in this group
    bool isCharAssociatedWithSymbol(char character, char symbol) const;     //!< Returns true if character is associated with symbol based on the bioSymbols contained in this group; false otherwise
    bool isEmpty() const;                                                   //!< Returns true if there are no BioSymbols in this group; false otherwise
    void removeBioSymbol(char symbol);                                      //!< Removes the BioSymbol denoted by symbol
    int serialNumber(char ch) const;                                        //!< Returns the serial number that ch was inserted into this BioSymbol group or 0 if ch is not found
    void setThresholdForAllBioSymbols(const double newThreshold);           //!< Sets the threshold of all biosymbols in this group to newThreshold
    QVector<char> symbolsInSerialOrder() const;                             //!< Returns an vector of symbols ordered according to the order they were added


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int nextSerialNumber() const;                                           //!< Returns a monotonically increasing number beginning with 1; used for assigning an order value to when a BioSymbol is inserted

    QHash<char, BioSymbol> bioSymbols_;
    QHash<char, int> bioSymbolSerialNumbers_;                               //!< Associates the given symbol character with the order it was added; useful for tie-breaking rules

    friend QDataStream &operator<<(QDataStream &out, const BioSymbolGroup &bioSymbolGroup);
    friend QDataStream &operator>>(QDataStream &in, BioSymbolGroup &bioSymbolGroup);
};

Q_DECLARE_METATYPE(BioSymbolGroup)

Q_DECLARE_TYPEINFO(BioSymbolGroup, Q_MOVABLE_TYPE);

QDataStream &operator<<(QDataStream &out, const BioSymbolGroup &bioSymbolGroup);
QDataStream &operator>>(QDataStream &in, BioSymbolGroup &bioSymbolGroup);

#endif // BIOSYMBOLGROUP_H
