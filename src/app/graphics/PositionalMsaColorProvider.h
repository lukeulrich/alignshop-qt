/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef POSITIONALMSACOLORPROVIDER_H
#define POSITIONALMSACOLORPROVIDER_H

#include <QtCore/QVector>

#include "TextColorStyle.h"
#include "../core/Msa.h"
#include "../core/util/ClosedIntRange.h"

/**
  * PositionalMsaColorProvider defines a abstract and default concrete implementation for returning colors based on a
  * specific position within a user-supplied Msa.
  */
class PositionalMsaColorProvider
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    virtual ~PositionalMsaColorProvider();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual TextColorStyle color(const Msa &msa, int row, int column) const;        //!< Returns the text color style for the row and column position within msa
    //! Returns a vector of text color styles for the columns in row within msa
    virtual QVector<TextColorStyle> colors(const Msa &msa, int row, const ClosedIntRange &columns) const;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
inline
PositionalMsaColorProvider::~PositionalMsaColorProvider()
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Default implementation always returns black foreground and white background.
  *
  * @param msa [const Msa &]
  * @param row [int]
  * @param column [int]
  * @returns TextColorStyle
  */
inline
TextColorStyle PositionalMsaColorProvider::color(const Msa & /* msa */, int /* row */, int /* column */) const
{
    return TextColorStyle(Qt::black, Qt::white);
}

/**
  * Default implementation always returns a vector initialized to TextColorStyles with black foreground and white
  * background.
  *
  * @param msa [const Msa &]
  * @param row [int]
  * @param columns [const ClosedIntRange &]
  * @returns QVector<TextColorStyle>
  */
inline
QVector<TextColorStyle>
PositionalMsaColorProvider::colors(const Msa & /* msa */, int /* row */, const ClosedIntRange &columns) const
{
    return QVector<TextColorStyle>(columns.length(), TextColorStyle(Qt::black, Qt::white));
}

#endif // POSITIONALMSACOLORPROVIDER_H
