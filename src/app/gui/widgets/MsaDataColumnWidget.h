/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSADATACOLUMNWIDGET_H
#define MSADATACOLUMNWIDGET_H

#include <QtGui/QWidget>

#include "AbstractMsaSideWidget.h"
#include "../../core/global.h"

class QModelIndex;

class AbstractMsaView;
class MsaSubseqTableModel;

class MsaDataColumnWidget : public AbstractMsaSideWidget
{
    Q_OBJECT

public:
    explicit MsaDataColumnWidget(QWidget *parent = nullptr);
    MsaDataColumnWidget(AbstractMsaView *msaView, MsaSubseqTableModel *model, const int column, QWidget *parent = nullptr);

    void setColumn(const int column);
    void setModel(MsaSubseqTableModel *model, const int column = 0);

protected:
    virtual void paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow);
    virtual void wheelEvent(QWheelEvent *wheelEvent);

private Q_SLOTS:
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    MsaSubseqTableModel *model_;
    int column_;
};

#endif // MSADATACOLUMNWIDGET_H
