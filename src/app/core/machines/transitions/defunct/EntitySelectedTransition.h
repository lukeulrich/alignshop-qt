/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ENTITYSELECTEDTRANSITION_H
#define ENTITYSELECTEDTRANSITION_H

#include <QtCore/QAbstractTransition>

class QPlainTextEdit;
class MultiSeqTableModel;

class EntitySelectedTransition : public QAbstractTransition
{
public:
    EntitySelectedTransition(MultiSeqTableModel *model, QPlainTextEdit *plainTextEdit);

protected:
    virtual bool eventTest(QEvent *event);
    virtual void onTransition(QEvent *event);

private:
    MultiSeqTableModel *model_;
    QPlainTextEdit *plainTextEdit_;
};

#endif // ENTITYSELECTEDTRANSITION_H
