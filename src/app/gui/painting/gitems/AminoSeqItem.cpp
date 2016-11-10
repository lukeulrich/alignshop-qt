/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>

#include "AminoSeqItem.h"
#include "../../models/ColumnAdapters/AminoSeqColumnAdapter.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param aminoSeq [const AminoSeqSPtr &]
  * @param parentItem [QGraphicsItem *]
  */
AminoSeqItem::AminoSeqItem(const AminoSeqSPtr &aminoSeq, QGraphicsItem *parentItem)
    : AbstractSeqItem(aminoSeq, parentItem)
{
}

/**
  * @param aminoSeq [const AminoSeqSPtr &]
  * @param columnAdapter [AminoSeqColumnAdapter *]
  * @param parentItem [QGraphicsItem *]
  */
AminoSeqItem::AminoSeqItem(const AminoSeqSPtr &aminoSeq, AminoSeqColumnAdapter *columnAdapter, QGraphicsItem *parentItem)
    : AbstractSeqItem(aminoSeq, columnAdapter, AminoSeqColumnAdapter::eStartColumn, AminoSeqColumnAdapter::eStopColumn, parentItem)
{
}
