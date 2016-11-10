/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SYMBOLSTRINGCALCULATOR_H
#define SYMBOLSTRINGCALCULATOR_H

#include <QtCore/QHash>
#include <QtCore/QSet>

#include "../BioSymbolGroup.h"
#include "../types.h"

/**
  * SymbolStringCalculator determines the symbol string from a vector distribution of character frequencies.
  */
class SymbolStringCalculator
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SymbolStringCalculator(const BioSymbolGroup &bioSymbolGroup, const char defaultSymbol = ' ');

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const SymbolStringCalculator &other) const;
    bool operator!=(const SymbolStringCalculator &other) const;

    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioSymbolGroup bioSymbolGroup() const;
    QByteArray computeSymbolString(const VectorHashCharDouble &vectorHashCharDouble) const;
    char defaultSymbol() const;
    void setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup);
    void setDefaultSymbol(const char defaultSymbol);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void buildCharSymbolAssociation();

    // ------------------------------------------------------------------------------------------------
    // Private members
    BioSymbolGroup bioSymbolGroup_;
    char defaultSymbol_;
    QHash<char, QByteArray> charSymbolAssociation_;                             //!< Associates the given character with the list of symbols

    // Friend class provides access for sorting purposes
    friend class BioSymbolThresholdSerialLessThanPrivate;
};

#endif // SYMBOLSTRINGCALCULATOR_H
