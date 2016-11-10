/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "../../../version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::AboutDialog)
{
    ui_->setupUi(this);
    ui_->versionLabel_->setText(QString("AlignShop %1").arg(constants::versionString()));
    ui_->revisionLabel_->setText(QString("Revision %1").arg(constants::version::kRevision));
}

AboutDialog::~AboutDialog()
{
    delete ui_;
}

