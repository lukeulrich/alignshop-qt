/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ENTITYDESELECTEDTRANSITION_H
#define ENTITYDESELECTEDTRANSITION_H

#include <QtCore/QAbstractTransition>

class MultiSeqTableModel;

class EntityDeselectedTransition : public QAbstractTransition
{
public:
    EntityDeselectedTransition(MultiSeqTableModel *model);

protected:
    virtual bool eventTest(QEvent *event);
    virtual void onTransition(QEvent * /* event */) {}

private:
    MultiSeqTableModel *model_;
};

#endif // ENTITYDESELECTEDTRANSITION_H
