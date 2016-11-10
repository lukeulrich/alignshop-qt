/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LICENSEINFODIALOG_H
#define LICENSEINFODIALOG_H

#include <QtCore/QString>
#include <QtGui/QDialog>

namespace Ui {
    class LicenseInfoDialog;
}

class LicenseInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LicenseInfoDialog(QWidget *parent = 0);
    ~LicenseInfoDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::LicenseInfoDialog *ui;
};

#endif // LICENSEINFODIALOG_H
