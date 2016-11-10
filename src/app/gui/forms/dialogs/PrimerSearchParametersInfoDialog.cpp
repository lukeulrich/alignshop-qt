/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PrimerSearchParametersInfoDialog.h"
#include "ui_PrimerSearchParametersInfoDialog.h"
#include "../../../core/macros.h"
#include "../../../core/misc.h"

PrimerSearchParametersInfoDialog::PrimerSearchParametersInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::PrimerSearchParametersInfoDialog)
{
    ui_->setupUi(this);
    ui_->maximumTmStaticLabel_->setText(QString::fromUtf8("Maximum Î”T<sub>m</sub>") + " (°C):");
}

PrimerSearchParametersInfoDialog::~PrimerSearchParametersInfoDialog()
{
    delete ui_;
}

void PrimerSearchParametersInfoDialog::setPrimerSearchParameters(const PrimerSearchParameters *primerSearchParameters)
{
    ASSERT(primerSearchParameters != nullptr);

    ui_->ampliconSizeRangeLabel_->setText(QString("%1 .. %2")
                                          .arg(primerSearchParameters->ampliconLengthRange_.begin_)
                                          .arg(primerSearchParameters->ampliconLengthRange_.end_));
    ui_->primerLengthRangeLabel_->setText(QString("%1 .. %2")
                                          .arg(primerSearchParameters->primerLengthRange_.begin_)
                                          .arg(primerSearchParameters->primerLengthRange_.end_));
    ui_->tmRangeLabel_->setText(QString("%1 .. %2")
                                .arg(::round(primerSearchParameters->individualPrimerTmRange_.begin_, 1))
                                .arg(::round(primerSearchParameters->individualPrimerTmRange_.end_, 1)));
    ui_->maximumTmLabel_->setText(QString::number(::round(primerSearchParameters->maximumPrimerPairDeltaTm_, 1)));
    ui_->sodiumConcentrationLabel_->setText(QString::number(primerSearchParameters->milliMolarSodiumConcentration()));
    ui_->primerDnaConcentrationLabel_->setText(QString::number(::round(primerSearchParameters->microMolarDnaConcentration(), 1)));
    ui_->forwardReLabel_->setText(restrictionEnzymeText(primerSearchParameters->forwardRestrictionEnzyme_));
    ui_->reverseReLabel_->setText(restrictionEnzymeText(primerSearchParameters->reverseRestrictionEnzyme_));
    ui_->forwardTerminalLabel_->setText(primerSearchParameters->forwardTerminalPattern_.displayText());
    ui_->reverseTerminalLabel_->setText(primerSearchParameters->reverseTerminalPattern_.displayText());
}

QString PrimerSearchParametersInfoDialog::restrictionEnzymeText(const RestrictionEnzyme &restrictionEnzyme) const
{
    if (restrictionEnzyme.isEmpty())
        return QString();

    QString text = restrictionEnzyme.recognitionSite().asByteArray();
    if (restrictionEnzyme.name().size() > 0)
        text += QString(" - <em>%1</em>").arg(restrictionEnzyme.name());

    return text;
}
