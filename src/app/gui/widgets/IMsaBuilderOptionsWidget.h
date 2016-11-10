/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSABUILDEROPTIONSWIDGET_H
#define IMSABUILDEROPTIONSWIDGET_H

#include <QtGui/QWidget>
#include "../../core/global.h"

class OptionSet;

class IMsaBuilderOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    IMsaBuilderOptionsWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

    QString msaBuilderId() const
    {
        return msaBuilderId_;
    }
    void setMsaBuilderId(const QString &msaBuilderId)
    {
        msaBuilderId_ = msaBuilderId;
    }

    virtual OptionSet msaBuilderOptions() const = 0;

private:
    QString msaBuilderId_;
};

#endif // IMSABUILDEROPTIONSWIDGET_H
