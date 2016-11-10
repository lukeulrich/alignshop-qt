/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>

#include "DnaSeqItem.h"
#include "../../models/ColumnAdapters/DnaSeqColumnAdapter.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param dnaSeq [const DnaSeqSPtr &]
  * @param parentItem [QGraphicsItem *]
  */
DnaSeqItem::DnaSeqItem(const DnaSeqSPtr &dnaSeq, QGraphicsItem *parentItem)
    : AbstractSeqItem(dnaSeq, parentItem)
{
}

/**
  * @param dnaSeq [const DnaSeqSPtr &]
  * @param columnAdapter [DnaSeqColumnAdapter *]
  * @param parentItem [QGraphicsItem *]
  */
DnaSeqItem::DnaSeqItem(const DnaSeqSPtr &dnaSeq, DnaSeqColumnAdapter *columnAdapter, QGraphicsItem *parentItem)
    : AbstractSeqItem(dnaSeq, columnAdapter, DnaSeqColumnAdapter::eStartColumn, DnaSeqColumnAdapter::eStopColumn, parentItem)
{
}

DnaSeqItem::~DnaSeqItem()
{
    emit aboutToBeDestroyed();
}

DnaSeqSPtr DnaSeqItem::dnaSeq() const
{
    return boost::shared_static_cast<DnaSeq>(abstractSeq());
}
