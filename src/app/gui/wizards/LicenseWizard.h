/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>

#include "../../core/global.h"

class QCheckBox;
class QLineEdit;
class QTextEdit;

class LicenseWizard : public QWizard
{
    Q_OBJECT

public:
    LicenseWizard(QWidget *parent = nullptr);

    void setLicenseText(const QString &licenseText);
};


class LicenseInformationPage : public QWizardPage
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit LicenseInformationPage(QWidget *parent = nullptr);


    bool isComplete();
    bool validatePage();
    void displayErrorMessageBox();


private:
    QLineEdit *nameLineEdit_;
    QLineEdit *licenseKeyLineEdit_;
};



class AgreementPage : public QWizardPage
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit AgreementPage(QWidget *parent = nullptr);


    bool isComplete() const;


private:
    QTextEdit *licenseAgreementTextEdit_;
    QCheckBox *agreeToTermsCheckBox_;
};


class ThanksPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ThanksPage(QWidget *parent = nullptr);
};

#endif // LICENSEWIZARD_H
