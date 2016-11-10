/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LicenseInfoDialog.h"
#include "ui_LicenseInfoDialog.h"
#include "../../../core/misc.h"

LicenseInfoDialog::LicenseInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicenseInfoDialog)
{
    ui->setupUi(this);
    ui->textEdit->setText(::readBetaLicense());
}

LicenseInfoDialog::~LicenseInfoDialog()
{
    delete ui;
}

void LicenseInfoDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
