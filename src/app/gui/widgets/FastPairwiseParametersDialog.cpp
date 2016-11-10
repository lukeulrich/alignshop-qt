/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "FastPairwiseParametersDialog.h"
#include "ui_FastPairwiseParametersDialog.h"
#include "../../core/constants/ClustalWConstants.h"
#include "../../core/util/OptionSet.h"

FastPairwiseParametersDialog::FastPairwiseParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::FastPairwiseParametersDialog)
{
    ui_->setupUi(this);
}

FastPairwiseParametersDialog::~FastPairwiseParametersDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
OptionSet FastPairwiseParametersDialog::pairwiseOptions() const
{
    using namespace constants::ClustalW;

    OptionSet options;
    if (ui_->wordSizeSpinBox_->isEnabled())
        options << Option(kFastWordSizeOpt, ui_->wordSizeSpinBox_->value());
    if (ui_->bestDiagonalsSpinBox_->isEnabled())
        options << Option(kFastBestDiagonalsOpt, ui_->bestDiagonalsSpinBox_->value());
    if (ui_->windowSpinBox_->isEnabled())
        options << Option(kFastWindowBestDiagonalsOpt, ui_->windowSpinBox_->value());
    if (ui_->gapPenaltySpinBox_->isEnabled())
        options << Option(kFastGapPenaltyOpt, ui_->gapPenaltySpinBox_->value());
    options << Option(kFastScoreOpt, scoreString());

    return options;
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QString FastPairwiseParametersDialog::scoreString() const
{
    using namespace constants::ClustalW;

    if (ui_->scoreByPercentageRadioButton_->isChecked())
        return kFastScorePercent;
    else if (ui_->scoreByAbsoluteRadioButton_->isChecked())
        return kFastScoreAbsolute;

    return QString();
}
