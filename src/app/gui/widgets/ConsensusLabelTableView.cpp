/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QHeaderView>

#include "ConsensusLabelTableView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
ConsensusLabelTableView::ConsensusLabelTableView(QWidget *parent)
    : SingleColumnTableView(parent)
{
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setFrameShape(QTableView::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setShowGrid(false);
}


