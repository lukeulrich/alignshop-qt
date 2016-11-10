/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSYMBOLCOLORSCHEME_H
#define BIOSYMBOLCOLORSCHEME_H

#include "../core/BioSymbolGroup.h"
#include "CharColorScheme.h"

/**
  * BioSymbolColorScheme provides for associating specific TextColorStyles with a symbol and also a CharColorScheme for
  * associating TextColorStyle purely based on the character itself.
  *
  * Because a BioSymbol is associated with a specific set of characters, it is desirable to only color those characters
  * in a given column that are members of that BioSymbol's character set. Thus, the user may set a BioSymbolGroup which
  * will be used to return associated color's for members with the associated symbol or the default text color style
  * otherwise. For example:
  *
  * Given:
  * o BioSymbol('a', "ILV", .5)
  * o The column:
  *   I
  *   I
  *   P
  * o Which results in the symbol: a (because 2/3 residues are I which is greater than .5)
  * o TextColorStyle for 'a' => (Qt::red, Qt::white)
  *
  * Then:
  * o symbolColorStyle('I', 'a') -> (Qt::red, Qt::white)
  * o symbolColorStyle('P', 'a') -> TextColorStyle()
  *
  * If any CharColorScheme is defined, it will be given precedence when choosing colors for specific characters,
  * regardless of the symbol. To illustrate this, in the above example, if a CharColorScheme has a color style defined
  * for I, then that will be returned for all I characters regardless of the symbol.
  */
class BioSymbolColorScheme
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an instance with bioSymbolGroup and charColorScheme
    BioSymbolColorScheme(const BioSymbolGroup &bioSymbolGroup = BioSymbolGroup(),
                         const CharColorScheme &charColorScheme = CharColorScheme());

    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioSymbolGroup bioSymbolGroup() const;                                      //!< Returns the currently defined BioSymbolGroup
    CharColorScheme charColorScheme() const;                                    //!< Returns the currently defined CharColorScheme
    void setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup);               //!< Sets the BioSymbolGroup to bioSymbolGroup
    void setCharColorScheme(const CharColorScheme &charColorScheme);            //!< Sets the CharColorScheme to charColorScheme
    void setSymbolColorStyle(char symbol, const TextColorStyle &textColorStyle);//!< Sets the color style for symbol to textColorStyle
    virtual TextColorStyle symbolColorStyle(char character, char symbol) const; //!< Returns the color style for the character and symbol combination
    QHash<char, TextColorStyle> symbolColorStyles() const;                      //!< Returns a hash of all the symbol styles

protected:
    BioSymbolGroup bioSymbolGroup_;
    CharColorScheme charColorScheme_;
    QHash<char, TextColorStyle> symbolColorStyles_;
};

#endif // BIOSYMBOLCOLORSCHEME_H
