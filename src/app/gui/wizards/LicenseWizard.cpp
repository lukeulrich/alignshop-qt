/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include "LicenseWizard.h"
#include "../../core/Services/LicenseValidator.h"

LicenseWizard::LicenseWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowIcon(QIcon(":aliases/images/icons/alignshop"));
    setWindowTitle("AlignShop License Activation");
    addPage(new AgreementPage);
    addPage(new LicenseInformationPage);
    addPage(new ThanksPage);
    setOption(QWizard::NoBackButtonOnStartPage);
}

void LicenseWizard::setLicenseText(const QString &licenseText)
{
    setField("agreement", licenseText);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
LicenseInformationPage::LicenseInformationPage(QWidget *parent)
    : QWizardPage(parent),
      nameLineEdit_(nullptr),
      licenseKeyLineEdit_(nullptr)
{
    setTitle("License Information");
    setSubTitle("Please input your name and the license key you received by email. If you did not receive an email "
                "with this information, please request another license key.");

    nameLineEdit_ = new QLineEdit;
    nameLineEdit_->setPlaceholderText("(Required)");
    connect(nameLineEdit_, SIGNAL(textChanged(QString)), SIGNAL(completeChanged()));

    licenseKeyLineEdit_ = new QLineEdit;
    licenseKeyLineEdit_->setPlaceholderText("(Required");
    connect(licenseKeyLineEdit_, SIGNAL(textChanged(QString)), SIGNAL(completeChanged()));

    QGridLayout *gridLayout = new QGridLayout;
    setLayout(gridLayout);
    gridLayout->addWidget(new QLabel("Name:"),          0, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(nameLineEdit_,                0, 1, 1, 1);

    gridLayout->addWidget(new QLabel("License key:"),   1, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(licenseKeyLineEdit_,          1, 1, 1, 1);

    registerField("name", nameLineEdit_);
    registerField("licenseKey", licenseKeyLineEdit_);
}

bool LicenseInformationPage::isComplete()
{
    return !nameLineEdit_->text().isEmpty() &&
           !licenseKeyLineEdit_->text().isEmpty();
}

bool LicenseInformationPage::validatePage()
{
    QString name = nameLineEdit_->text();
    QString licenseKey = licenseKeyLineEdit_->text();
    if (!LicenseValidator::isValidLicense(name, licenseKey))
    {
        displayErrorMessageBox();
        return false;
    }

    return true;
}

void LicenseInformationPage::displayErrorMessageBox()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Invalid license");
    msgBox.setText("Invalid user name and/or license key information.");
    msgBox.setInformativeText("Please check that 1) you have entered "
                   "your name exactly as you did when requesting a license key and 2) that the license key "
                   "you entered is correct.\n\n");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
AgreementPage::AgreementPage(QWidget *parent)
    : QWizardPage(parent),
      licenseAgreementTextEdit_(nullptr),
      agreeToTermsCheckBox_(nullptr)
{
    setTitle("Terms and conditions");
    setSubTitle("If you agree to the following license agreement, please check the checkbox below and press Next.");

    licenseAgreementTextEdit_ = new QTextEdit;
    licenseAgreementTextEdit_->setReadOnly(true);
    agreeToTermsCheckBox_ = new QCheckBox("I agree to the terms and conditions");
    connect(agreeToTermsCheckBox_, SIGNAL(toggled(bool)), SIGNAL(completeChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(6, 6, 6, 0);
    setLayout(layout);
    layout->addWidget(licenseAgreementTextEdit_);
    layout->addWidget(agreeToTermsCheckBox_);

    registerField("agreement", licenseAgreementTextEdit_, "html");
}

bool AgreementPage::isComplete() const
{
    return agreeToTermsCheckBox_->isChecked() && !licenseAgreementTextEdit_->toPlainText().isEmpty();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ThanksPage::ThanksPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Registration Successful");
    setSubTitle("Thank you for testing AlignShop!");

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(new QLabel("We are actively developing AlignShop and are grateful for any suggestions, bugs,\n"
                                 "or other feedback you may to improve this product.\n\n"));
}


