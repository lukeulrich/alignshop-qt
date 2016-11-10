/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef EMPTYMSABUILDEROPTIONSWIDGET_H
#define EMPTYMSABUILDEROPTIONSWIDGET_H

#include "IMsaBuilderOptionsWidget.h"

namespace Ui {
    class EmptyMsaBuilderOptionsWidget;
}

class EmptyMsaBuilderOptionsWidget : public IMsaBuilderOptionsWidget
{
    Q_OBJECT


public:
    explicit EmptyMsaBuilderOptionsWidget(QWidget *parent = nullptr);
    ~EmptyMsaBuilderOptionsWidget();

    OptionSet msaBuilderOptions() const;


private:
    Ui::EmptyMsaBuilderOptionsWidget *ui_;
};

#endif // EMPTYMSABUILDEROPTIONSWIDGET_H
