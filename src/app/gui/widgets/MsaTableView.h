/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSATABLEVIEW_H
#define MSATABLEVIEW_H

#include "SingleColumnTableView.h"
#include "../../core/global.h"

class QAbstractItemModel;

class AbstractMsaView;

class MsaTableView : public SingleColumnTableView
{
    Q_OBJECT

public:
    explicit MsaTableView(QWidget *parent = nullptr);
    MsaTableView(AbstractMsaView *msaView, QWidget *parent = nullptr);

    void setMsaView(AbstractMsaView *msaView);


protected Q_SLOTS:
    virtual void resizeFont();



private:
    AbstractMsaView *msaView_;
};

#endif // MSATABLEVIEW_H
