/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SlowPairwiseParametersDialog.h"
#include "ui_SlowPairwiseParametersDialog.h"
#include "../../core/constants/ClustalWConstants.h"
#include "../../core/util/OptionSet.h"
#include "../../core/macros.h"

SlowPairwiseParametersDialog::SlowPairwiseParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::SlowPairwiseParametersDialog)
{
    ui_->setupUi(this);

    ui_->gapOpenPenaltyDoubleSpinBox_->setDecimals(6);
    ui_->gapOpenPenaltyDoubleSpinBox_->setVisibleDecimals(1);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setDecimals(6);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setVisibleDecimals(1);
}

SlowPairwiseParametersDialog::~SlowPairwiseParametersDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
OptionSet SlowPairwiseParametersDialog::pairwiseOptions() const
{
    using namespace constants::ClustalW;

    OptionSet options;
    if (ui_->gapOpenPenaltyDoubleSpinBox_->isEnabled())
        options << Option(kSlowGapOpenPenaltyOpt, ui_->gapOpenPenaltyDoubleSpinBox_->value());
    if (ui_->gapExtensionPenaltyDoubleSpinBox_->isEnabled())
        options << Option(kSlowGapExtensionPenaltyOpt, ui_->gapExtensionPenaltyDoubleSpinBox_->value());
    if (ui_->weightMatrixComboBox_->isEnabled())
    {
        QString weightMatrix = ui_->weightMatrixComboBox_->currentText();
        if (!weightMatrix.isEmpty())
        {
            ASSERT(grammar_ == eDnaGrammar || grammar_ == eAminoGrammar);
            options << Option(clustalWMatrixOption(), weightMatrix);
        }
    }

    return options;
}

void SlowPairwiseParametersDialog::setGrammar(const Grammar &grammar)
{
    grammar_ = grammar;
    setWeightMatricesBasedOnGrammar();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void SlowPairwiseParametersDialog::setWeightMatricesBasedOnGrammar()
{
    ui_->weightMatrixComboBox_->clear();

    QStringList weightMatrices;
    using namespace constants::ClustalW;
    QString defaultWeightMatrix;
    if (grammar_ == eAminoGrammar)
    {
        weightMatrices << kProteinWeightMatrixBlosum
                       << kProteinWeightMatrixPam
                       << kProteinWeightMatrixGonnet
                       << kProteinWeightMatrixId;
        defaultWeightMatrix = kProteinWeightMatrixGonnet;
    }
    else if (grammar_ == eDnaGrammar)
    {
        weightMatrices << kDnaWeightMatrixIub
                       << kDnaWeightMatrixClustalw;
        defaultWeightMatrix = kDnaWeightMatrixIub;
    }
    ui_->weightMatrixComboBox_->addItems(weightMatrices);
    ui_->weightMatrixComboBox_->setCurrentIndex(ui_->weightMatrixComboBox_->findText(defaultWeightMatrix));
}

QString SlowPairwiseParametersDialog::clustalWMatrixOption() const
{
    using namespace constants::ClustalW;
    if (grammar_ == eAminoGrammar)
        return kSlowProteinWeightMatrixOpt;
    else if (grammar_ == eDnaGrammar)
        return kSlowDnaWeightMatrixOpt;

    return QString();
}
