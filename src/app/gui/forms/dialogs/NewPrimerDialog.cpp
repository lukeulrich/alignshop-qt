/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "NewPrimerDialog.h"
#include "ui_NewPrimerDialog.h"
#include "../../../core/constants.h"
#include "../../../core/BioString.h"
#include "../../../core/misc.h"
#include "../../../primer/DnaSequenceValidator.h"
#include "../../../primer/Primer.h"
#include "../../../primer/PrimerFactory.h"
#include "../../../primer/PrimerSearchParameters.h"     // To make QSharedPointer warnings go away
#include "../../../primer/RestrictionEnzymeBrowserDialog.h"
#include "../../../primer/ThermodynamicCalculator.h"


static const int kMinimumRecommendedCoreSequenceLength = 5;


// ------------------------------------------------------------------------------------------------
// Constructor and destructor
NewPrimerDialog::NewPrimerDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::NewPrimerDialog)
{
    ui_->setupUi(this);
    ui_->sequenceLineEdit_->setValidator(new DnaSequenceValidator(this));
    QString tmToolTip = QString("Calculated with %1 mM [Na<sup>+</sup>] and %2 mM [primer DNA]").arg(PrimerFactory::kDefaultSodiumMolarity_ * 1000.).arg(PrimerFactory::kDefaultPrimerDnaMolarity_ * 1000.);
    ui_->calculatedTmLabel_->setToolTip(tmToolTip);
    ui_->staticTmLabel_->setToolTip(tmToolTip);
    setTmLabelToNullValue();
    enableDisableOkButton();

    connect(ui_->reBrowserToolButton_, SIGNAL(released()), SLOT(onREToolButtonReleased()));
    connect(ui_->nameLineEdit_, SIGNAL(textChanged(QString)), SLOT(enableDisableOkButton()));
    connect(ui_->sequenceLineEdit_, SIGNAL(textChanged(QString)), SLOT(enableDisableOkButton()));
    connect(ui_->reLineEdit_, SIGNAL(textChanged(QString)), SLOT(recalculateTm()));
    connect(ui_->sequenceLineEdit_, SIGNAL(textChanged(QString)), SLOT(recalculateTm()));

    connect(ui_->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(onAcceptRequest()));
}

NewPrimerDialog::~NewPrimerDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
Primer NewPrimerDialog::primer() const
{
    PrimerFactory primerFactory;

    Primer newPrimer = primerFactory.makePrimer(BioString(ui_->sequenceLineEdit_->text().toAscii(), eDnaGrammar),
                                                ui_->reLineEdit_->restrictionEnzyme());
    newPrimer.setName(ui_->nameLineEdit_->text());
    return newPrimer;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void NewPrimerDialog::onREToolButtonReleased()
{
    RestrictionEnzymeBrowserDialog reDialog(this);
    reDialog.setWindowTitle("Select Restriction Enzyme");
    reDialog.setRebaseFile(qApp->applicationDirPath() + QDir::separator() + constants::kRestrictionEnzymeEmbossPath);

    if (reDialog.exec())
        ui_->reLineEdit_->setRestrictionEnzyme(reDialog.selectedRestrictionEnzyme());
}

void NewPrimerDialog::enableDisableOkButton()
{
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(requiredFieldsAreSatisfied());
}

void NewPrimerDialog::recalculateTm()
{
    BioString dnaString = completePrimerSequence();
    if (dnaString.isEmpty())
    {
        setTmLabelToNullValue();
        return;
    }

    ThermodynamicCalculator thermoCalc;
    double tm = thermoCalc.meltingTemperature(dnaString, PrimerFactory::kDefaultSodiumMolarity_, PrimerFactory::kDefaultPrimerDnaMolarity_);
    setTmLabelWithDouble(tm);
}

void NewPrimerDialog::onAcceptRequest()
{
    if (primerCoreSequenceLengthIsUnusuallySmall())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Atypical primer length");
        msgBox.setText(QString("The primer core sequence, %1, is unusually small. Are you sure you wish to define "
                               "this primer?").arg(ui_->sequenceLineEdit_->text()));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No)
            return;
    }

    // Otherwise, all is good!
    accept();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool NewPrimerDialog::requiredFieldsAreSatisfied() const
{
    bool hasValidName = ui_->nameLineEdit_->text().trimmed().size() > 0;
    bool hasValidSequence = ui_->sequenceLineEdit_->text().size() > 0;

    return hasValidName && hasValidSequence;
}

BioString NewPrimerDialog::completePrimerSequence() const
{
    return BioString(ui_->reLineEdit_->restrictionEnzyme().recognitionSite().asByteArray() +
                     ui_->sequenceLineEdit_->text().trimmed().toAscii(),
                     eDnaGrammar);
}

void NewPrimerDialog::setTmLabelToNullValue()
{
    ui_->calculatedTmLabel_->clear();
}

void NewPrimerDialog::setTmLabelWithDouble(const double tm)
{
    ui_->calculatedTmLabel_->setText(QString::number(::round(tm, 1)) + " °C");
}

bool NewPrimerDialog::primerCoreSequenceLengthIsUnusuallySmall() const
{
    return ui_->sequenceLineEdit_->text().size() < kMinimumRecommendedCoreSequenceLength;
}
