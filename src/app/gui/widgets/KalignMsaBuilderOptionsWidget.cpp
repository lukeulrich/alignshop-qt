/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "KalignMsaBuilderOptionsWidget.h"
#include "ui_KalignMsaBuilderOptionsWidget.h"
#include "../../core/constants/KalignConstants.h"
#include "../../core/util/OptionSet.h"

KalignMsaBuilderOptionsWidget::KalignMsaBuilderOptionsWidget(QWidget *parent) :
    IMsaBuilderOptionsWidget(parent),
    ui_(new Ui::KalignMsaBuilderOptionsWidget)
{
    ui_->setupUi(this);

    setPenaltySpinBoxesPrecision(6);
    setPenaltySpinBoxesVisiblePrecision(1);
}

KalignMsaBuilderOptionsWidget::~KalignMsaBuilderOptionsWidget()
{
    delete ui_;
}

OptionSet KalignMsaBuilderOptionsWidget::msaBuilderOptions() const
{
    OptionSet kalignOptions;

    using namespace constants::Kalign;
    if (ui_->gapOpenPenaltyDoubleSpinBox_->isEnabled())
        kalignOptions << Option(kGapOpenOpt, ui_->gapOpenPenaltyDoubleSpinBox_->value());
    if (ui_->gapExtensionPenaltyDoubleSpinBox_->isEnabled())
        kalignOptions << Option(kGapExtensionOpt, ui_->gapExtensionPenaltyDoubleSpinBox_->value());
    if (ui_->terminalGapPenaltyDoubleSpinBox_->isEnabled())
        kalignOptions << Option(kTerminalGapExtensionPenaltyOpt, ui_->terminalGapPenaltyDoubleSpinBox_->value());

    kalignOptions << Option(kSortOpt, sortMethod());
    kalignOptions << Option(kDistanceOpt, distanceMethod());
    kalignOptions << Option(kGuideTreeOpt, guideTree());

    return kalignOptions;
}

void KalignMsaBuilderOptionsWidget::setGapOpenPenalty(const double newGapOpenPenalty)
{
    ui_->gapOpenPenaltyDoubleSpinBox_->setValue(newGapOpenPenalty);
}

void KalignMsaBuilderOptionsWidget::setGapExtendPenalty(const double newGapExtendPenalty)
{
    ui_->gapExtensionPenaltyDoubleSpinBox_->setValue(newGapExtendPenalty);
}

void KalignMsaBuilderOptionsWidget::setTerminalGapPenalty(const double newTerminalGapPenalty)
{
    ui_->terminalGapPenaltyDoubleSpinBox_->setValue(newTerminalGapPenalty);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void KalignMsaBuilderOptionsWidget::setPenaltySpinBoxesPrecision(const int decimals)
{
    ui_->gapOpenPenaltyDoubleSpinBox_->setDecimals(decimals);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setDecimals(decimals);
    ui_->terminalGapPenaltyDoubleSpinBox_->setDecimals(decimals);
}

void KalignMsaBuilderOptionsWidget::setPenaltySpinBoxesVisiblePrecision(const int decimals)
{
    ui_->gapOpenPenaltyDoubleSpinBox_->setVisibleDecimals(decimals);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setVisibleDecimals(decimals);
    ui_->terminalGapPenaltyDoubleSpinBox_->setVisibleDecimals(decimals);
}

QString KalignMsaBuilderOptionsWidget::sortMethod() const
{
    using namespace constants::Kalign;
    if (ui_->orderByInputSequencesRadioButton_->isChecked())
        return kSortInput;
    else if (ui_->orderByTreeRadioButton_->isChecked())
        return kSortTree;
    else
        return kSortGaps;
}

QString KalignMsaBuilderOptionsWidget::distanceMethod() const
{
    using namespace constants::Kalign;

    if (ui_->distanceWuRadioButton_->isChecked())
        return kDistanceWu;
    else if (ui_->distancePairRadioButton_->isChecked())
        return kDistancePair;

    return kDistanceWu;
}

QString KalignMsaBuilderOptionsWidget::guideTree() const
{
    using namespace constants::Kalign;

    if (ui_->guideByUPGMARadioButton_->isChecked())
        return kGuideTreeUPGMA;
    else if (ui_->guideByNJRadioButton_->isChecked())
        return kGuideTreeNJ;

    return kGuideTreeUPGMA;
}
