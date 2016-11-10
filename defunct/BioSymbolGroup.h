/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSYMBOLGROUP_H
#define BIOSYMBOLGROUP_H

#include "BioSymbol.h"
#include "global.h"

/**
  * BioSymbolGroup manages a collection of BioSymbols and provides a convenient mechanism for testing if a character
  * belongs to a particular symbol.
  *
  * BioSymbolGroups provide a flexible mechanism for encapsulating various rules which may be applied to such things as
  * character count distributions. They are principally intended for use with the character count distribution of a
  * multiple sequence alignment to determine consensus data patterns such as Clustal patterns.
  *
  * A BioSymbolGroup also contains a defaultSymbol character that is used when calculating a symbol string and there is
  * no symbol associated with the given character.
  */
class BioSymbolGroup
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    BioSymbolGroup(const char defaultSymbol = ' ');                         //!< Construct a BioSymbolGroup with defaultSymbol (see intro)

    // ------------------------------------------------------------------------------------------------
    // Operators
    BioSymbolGroup &operator<<(const BioSymbol &bioSymbol);                 //!< Convenience method that adds bioSymbol to this group (or replaces an existing version if it already exists) and returns a reference to this object

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void addBioSymbol(const BioSymbol &bioSymbol);                          //!< Adds bioSymbol to this group (or replaces an existing version if it already exists)
    QHash<char, BioSymbol> bioSymbols() const;                              //!< Returns the hash of BioSymbols
    //!< Calculates and returns the symbolic string representation of the characters represented within listHashCharDouble
    QString calculateSymbolString(const ListHashCharDouble &listHashCharDouble) const;
    void clear();                                                           //!< Clears all BioSymbols
    int count() const;                                                      //!< Returns the number of BioSymbols in this group
    char defaultSymbol() const;                                             //!< Returns the default symbol character
    bool isCharAssociatedWithSymbol(char character, char symbol) const;     //!< Returns true if character is associated with symbol based on the bioSymbols contained in this group; false otherwise
    bool isEmpty() const;                                                   //!< Returns true if there are no BioSymbols in this group; false otherwise
    void removeBioSymbol(char symbol);                                      //!< Removes the BioSymbol denoted by symbol
    void setDefaultSymbol(char defaultSymbol);                              //!< Sets the default symbol character to defaultSymbol

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int nextInsertNumber() const;                                           //!< Returns a monotonically increasing number beginning with 1; used for assigning an order value to when a BioSymbol is inserted

    QHash<char, BioSymbol> bioSymbols_;
    char defaultSymbol_;

    QHash<char, QSet<char> > charSymbolLookup_;                             //!< (internal) Associates the given character with the list of symbols
    QHash<char, int> bioSymbolInsertOrder_;                                 //!< (internal) Associates the given symbol with the order it was inserted; used for tie-breaking matching rules

    // Friend class provides access to our bioSymbolInsertOrder_ member for sorting purposes
    friend class BioSymbolThresholdInsertOrderLessThan;

    /**
      * BioSymbolThresholdInsertOrderLessThan simply provides a functor style interface for sorting BioSymbols first by
      * their actual proportions and then if they are equivalent by their insert order.
      */
    class BioSymbolThresholdInsertOrderLessThan
    {
    public:
        BioSymbolThresholdInsertOrderLessThan(const BioSymbolGroup *self);  //!< Trivial constructor taking pointer to instantiating class
        bool operator()(const PairCharDouble &a, const PairCharDouble &b);  //!< Core comparison method; first = symbol, second = proportion

    private:
        const BioSymbolGroup *self_;                                        //!< Pointer to source BioSymbolGroup for access to its symbol insert order member
    };
};

Q_DECLARE_METATYPE(BioSymbolGroup);

#endif // BIOSYMBOLGROUP_H
