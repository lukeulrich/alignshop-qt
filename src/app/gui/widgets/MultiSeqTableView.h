/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MULTISEQTABLEVIEW_H
#define MULTISEQTABLEVIEW_H

#include <QtGui/QTableView>

#include "../../core/global.h"

class MultiSeqTableView : public QTableView
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit MultiSeqTableView(QWidget *parent = nullptr);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void clearCut();
    void cut(const QItemSelection &selection);
    void enterPressed(const QModelIndex &currentIndex);
    void pasteTo(const QModelIndex &index);
    void deleteSelection(const QItemSelection &selection);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // MULTISEQTABLEVIEW_H
