/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SECONDARYSTRUCTURECOLORPROVIDER_H
#define SECONDARYSTRUCTURECOLORPROVIDER_H

#include "PositionalMsaColorProvider.h"
#include "LinearColorScheme.h"

/**
  * SecondaryStructureColorProvider provides Msa colors using a linear color scheme in conjunction with the predicted
  * secondary structure data.
  */
class SecondaryStructureColorProvider : public PositionalMsaColorProvider
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an instance with linearColorScheme
    SecondaryStructureColorProvider(const LinearColorScheme &linearColorScheme);

    // ------------------------------------------------------------------------------------------------
    // Reimplmented public methods
    virtual TextColorStyle color(const Msa &msa, int row, int column) const;        //!< Returns the text color style for the row and column position within msa with respect to any predicted secondary structure
    //! Returns the text color style for the columns in row within msa with respect to any predicted secondary structure
    QVector<TextColorStyle> colors(const Msa &msa, int row, const ClosedIntRange &columns) const;

private:
    LinearColorScheme linearColorScheme_;
};

#endif // SECONDARYSTRUCTURECOLORPROVIDER_H
