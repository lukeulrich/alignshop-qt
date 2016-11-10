/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARCOLORPROVIDER_H
#define CHARCOLORPROVIDER_H

#include "CharColorScheme.h"
#include "PositionalMsaColorProvider.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;
struct TextColorStyle;

class CharColorProvider : public PositionalMsaColorProvider
{
public:
    CharColorProvider(const CharColorScheme &charColorScheme);

    virtual TextColorStyle color(const Msa &msa, int row, int column) const;        //!< Returns the text color style for the row and column position within msa
    //! Returns the text color style for the columns in row within msa
    virtual QVector<TextColorStyle> colors(const Msa &msa, int row, const ClosedIntRange &columns) const;

private:
    CharColorScheme charColorScheme_;
};

#endif // CHARCOLORPROVIDER_H
