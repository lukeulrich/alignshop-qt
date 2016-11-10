/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "EmptyMsaBuilderOptionsWidget.h"
#include "ui_EmptyMsaBuilderOptionsWidget.h"
#include "../../core/util/OptionSet.h"

EmptyMsaBuilderOptionsWidget::EmptyMsaBuilderOptionsWidget(QWidget *parent) :
    IMsaBuilderOptionsWidget(parent),
    ui_(new Ui::EmptyMsaBuilderOptionsWidget)
{
    ui_->setupUi(this);
}

EmptyMsaBuilderOptionsWidget::~EmptyMsaBuilderOptionsWidget()
{
    delete ui_;
}

OptionSet EmptyMsaBuilderOptionsWidget::msaBuilderOptions() const
{
    return OptionSet();
}
