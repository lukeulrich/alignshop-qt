/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RAWSEQITEM_H
#define RAWSEQITEM_H

#include "AbstractLinearItem.h"
#include "../../../core/BioString.h"
#include "../../../core/global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QFont;

class AbstractTextRenderer;

/**
  * RawSeqItem visualizes a BioString as an AbstractLinearItem.
  */
class BioStringItem : public AbstractLinearItem
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Create a visual item for bioString using font
    BioStringItem(const BioString &bioString, const QFont &font, QGraphicsItem *parentItem = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString bioString() const;                        //!< Returns the bioString
    double height() const;                              //!< Returns the height
    //! Paint method for rendering the BioString
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setBioString(const BioString &newBioString);   //!< Sets the BioString to newBioString


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void onPixelsPerUnitChanged();                      //!< Virtual stub for updating the layout when the pixels per unit changes


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    BioString bioString_;                               //!< Source BioString
    AbstractTextRenderer *textRenderer_;                //!< Pointer to the text renderer for actually rendering the characters
};

#endif // RAWSEQITEM_H
