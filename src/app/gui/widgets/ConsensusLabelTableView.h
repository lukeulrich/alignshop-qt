/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSLABELTABLEVIEW_H
#define CONSENSUSLABELTABLEVIEW_H

#include "SingleColumnTableView.h"
#include "../../core/global.h"

class ConsensusLabelTableView : public SingleColumnTableView
{
    Q_OBJECT

public:
    explicit ConsensusLabelTableView(QWidget *parent = nullptr);
};

#endif // CONSENSUSLABELTABLEVIEW_H
