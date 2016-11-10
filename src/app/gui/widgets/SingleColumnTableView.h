/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SINGLECOLUMNTABLEVIEW_H
#define SINGLECOLUMNTABLEVIEW_H

#include <QtGui/QTableView>
#include "../../core/global.h"

class SingleColumnTableView : public QTableView
{
    Q_OBJECT

public:
    explicit SingleColumnTableView(QWidget *parent = nullptr);

    int column() const;
    void setColumn(const int column);
    void setModel(QAbstractItemModel *model);
    void setIgnoreShortcutOverride(const QKeySequence &shortcut);
    void setIgnoreShortcutOverrides(const QList<QKeySequence> &shortcuts);
    void setIgnoreShortcutKey(const char key);


protected:
    virtual bool event(QEvent *event);
    void hideAllSections();


private:
    int column_;
    QSet<int> ignoredShortcuts_;
    QSet<int> ignoredShortcutKeys_;
};

#endif // SINGLECOLUMNTABLEVIEW_H
