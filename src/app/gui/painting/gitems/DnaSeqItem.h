/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQITEM_H
#define DNASEQITEM_H

#include "AbstractSeqItem.h"
#include "../../../core/Entities/DnaSeq.h"
#include "../../../core/global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class DnaSeqColumnAdapter;

/**
  * DnaSeqItem extends AbstractSeqItem to specifically render an DnaSeq object.
  */
class DnaSeqItem : public AbstractSeqItem
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DnaSeqItem(const DnaSeqSPtr &dnaSeq, QGraphicsItem *parentItem = nullptr);
    DnaSeqItem(const DnaSeqSPtr &dnaSeq, DnaSeqColumnAdapter *columnAdapter, QGraphicsItem *parentItem = nullptr);
    ~DnaSeqItem();

    DnaSeqSPtr dnaSeq() const;


Q_SIGNALS:
    void aboutToBeDestroyed();
};

#endif // DNASEQITEM_H
