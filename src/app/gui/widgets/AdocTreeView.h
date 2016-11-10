/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCTREEVIEW_H
#define ADOCTREEVIEW_H

#include <QtGui/QTreeView>

#include "../../core/global.h"

class AdocTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit AdocTreeView(QWidget *parent = nullptr);

Q_SIGNALS:
    void clearCut();
    void cut(const QItemSelection &selection);
    void pasteTo(const QModelIndex &index);
    void deleteSelection(const QItemSelection &selection);

protected:
    virtual void dropEvent(QDropEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // ADOCTREEVIEW_H
