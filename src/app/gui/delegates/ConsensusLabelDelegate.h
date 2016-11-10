/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSLABELDELEGATE_H
#define CONSENSUSLABELDELEGATE_H

#include "SpinBoxDelegate.h"

class ConsensusLabelDelegate : public SpinBoxDelegate
{
public:
    explicit ConsensusLabelDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CONSENSUSLABELDELEGATE_H
