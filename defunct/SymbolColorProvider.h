/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SYMBOLCOLORPROVIDER_H
#define SYMBOLCOLORPROVIDER_H

#include "PositionalMsaColorProvider.h"
#include "SymbolColorScheme.h"

class LiveSymbolString;

/**
  * SymbolColorProvider returns colors based upon the symbol string for a given Msa.
  *
  * Currently, the symbol string must come in the form of a LiveSymbolString. The other option is to have a method for
  * setting a local symbol string and then referencing that when requesting color lookups; however, the value of this
  * is limited and just as easily accomplished with a LiveSymbolString.
  *
  * If a null liveSymbolString is passed to this as a constructor, the default text color style will always be returned.
  */
class SymbolColorProvider : public PositionalMsaColorProvider
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct instance of this class with liveSymbolString and symbolColorScheme; does not take ownership of liveSymbolString
    SymbolColorProvider(const LiveSymbolString *liveSymbolString, const SymbolColorScheme &symbolColorScheme);

    // ------------------------------------------------------------------------------------------------
    // Reimplmented public methods
    virtual TextColorStyle color(const Msa &msa, int row, int column) const;        //!< Returns the text color style for the row and column position within msa with respect to the currently defined symbol string

private:
    const LiveSymbolString *liveSymbolString_;
    SymbolColorScheme symbolColorScheme_;
};

#endif // SYMBOLCOLORPROVIDER_H
