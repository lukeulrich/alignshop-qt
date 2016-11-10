/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQITEM_H
#define AMINOSEQITEM_H

#include "AbstractSeqItem.h"
#include "../../../core/Entities/AminoSeq.h"
#include "../../../core/global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AminoSeqColumnAdapter;

/**
  * AminoSeqItem extends AbstractSeqItem to specifically render an AminoSeq object.
  */
class AminoSeqItem : public AbstractSeqItem
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    AminoSeqItem(const AminoSeqSPtr &aminoSeq, QGraphicsItem *parentItem = nullptr);
    AminoSeqItem(const AminoSeqSPtr &aminoSeq, AminoSeqColumnAdapter *columnAdapter, QGraphicsItem *parentItem = nullptr);
};

#endif // AMINOSEQITEM_H
