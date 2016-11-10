/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ClustalWMsaBuilderOptionsWidget.h"
#include "ui_ClustalWMsaBuilderOptionsWidget.h"
#include "FastPairwiseParametersDialog.h"
#include "SlowPairwiseParametersDialog.h"
#include "../../core/constants/ClustalWConstants.h"
#include "../../core/util/OptionSet.h"
#include "../../core/macros.h"

#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// Constructor and destructor
ClustalWMsaBuilderOptionsWidget::ClustalWMsaBuilderOptionsWidget(const Grammar grammar, QWidget *parent) :
    IMsaBuilderOptionsWidget(parent),
    ui_(new Ui::ClustalWMsaBuilderOptionsWidget),
    grammar_(grammar),
    slowPairwiseParamatersDialog_(nullptr),
    fastPairwiseParamatersDialog_(nullptr)
{
    ASSERT(grammar == eDnaGrammar || grammar == eAminoGrammar);

    ui_->setupUi(this);

    setPenaltySpinBoxesPrecision(6);
    setPenaltySpinBoxesVisiblePrecision(1);
    setWeightMatricesBasedOnGrammar();

    connect(ui_->editPairwiseParametersButton_, SIGNAL(released()), SLOT(showPairwiseAlignmentParameters()));
}

ClustalWMsaBuilderOptionsWidget::~ClustalWMsaBuilderOptionsWidget()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// Public methods
OptionSet ClustalWMsaBuilderOptionsWidget::msaBuilderOptions() const
{
    using namespace constants::ClustalW;
    OptionSet clustalWOptions;
    clustalWOptions << Option(kOutOrderOpt, outputOrder());
    clustalWOptions << Option(kGuideTreeOpt, guideTreeAlgorithm());
    if (ui_->excludeGapPositionsCheckBox_->isChecked())
        clustalWOptions << Option(kTreeExcludeGapPositionsOpt);
    if (ui_->correctMultipleSubstitutionsCheckBox_->isChecked())
        clustalWOptions << Option(kTreeCorrectMultipleSubstitionsOpt);
    if (ui_->fastPairwiseAlignmentsRadioButtion_->isChecked())
        clustalWOptions << Option(kQuickTreeOpt);
    clustalWOptions << getPairwiseParameterOptions();
    if (ui_->gapOpenPenaltyDoubleSpinBox_->isEnabled())
        clustalWOptions << Option(kMsaGapOpenPenalty, ui_->gapOpenPenaltyDoubleSpinBox_->value());
    if (ui_->gapExtensionPenaltyDoubleSpinBox_->isEnabled())
        clustalWOptions << Option(kMsaGapExtensionPenalty, ui_->gapExtensionPenaltyDoubleSpinBox_->value());
    if (ui_->delayDivergentSequencesSpinBox_->isEnabled())
        clustalWOptions << Option(kMsaPercentIdentityForDelay, ui_->delayDivergentSequencesSpinBox_->value());
    if (ui_->weightMatrixComboBox_->isEnabled())
        clustalWOptions << Option(msaMatrixOptionKey(), ui_->weightMatrixComboBox_->currentText());
    if (ui_->disableSequenceWeightingCheckBox_->isChecked())
        clustalWOptions << Option(kMsaDisableSequenceWeighting);
    if (ui_->iterateCheckBox_->isChecked())
    {
        clustalWOptions << Option(kMsaIteration, iterationMethod());
        clustalWOptions << Option(kMsaNumberofIterations, ui_->numIterationsSpinBox_->value());
    }

    foreach (const Option &option, clustalWOptions.asVector())
        qDebug() << Q_FUNC_INFO << option.name_ << option.value_;

    return clustalWOptions;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void ClustalWMsaBuilderOptionsWidget::showPairwiseAlignmentParameters()
{
    if (ui_->slowPairwiseAlignmentsRadioButton_->isChecked())
        displaySlowPairwiseParametersDialog();
    else
        displayFastPairwiseParametersDialog();
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void ClustalWMsaBuilderOptionsWidget::setPenaltySpinBoxesPrecision(const int decimals)
{
    ui_->gapOpenPenaltyDoubleSpinBox_->setDecimals(decimals);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setDecimals(decimals);
}

void ClustalWMsaBuilderOptionsWidget::setPenaltySpinBoxesVisiblePrecision(const int decimals)
{
    ui_->gapOpenPenaltyDoubleSpinBox_->setVisibleDecimals(decimals);
    ui_->gapExtensionPenaltyDoubleSpinBox_->setVisibleDecimals(decimals);
}

void ClustalWMsaBuilderOptionsWidget::displaySlowPairwiseParametersDialog()
{
    if (slowPairwiseParamatersDialog_ == nullptr)
    {
        slowPairwiseParamatersDialog_ = new SlowPairwiseParametersDialog(this);
        slowPairwiseParamatersDialog_->setGrammar(grammar_);
    }

    slowPairwiseParamatersDialog_->exec();
}

void ClustalWMsaBuilderOptionsWidget::displayFastPairwiseParametersDialog()
{
    if (fastPairwiseParamatersDialog_ == nullptr)
        fastPairwiseParamatersDialog_ = new FastPairwiseParametersDialog(this);

    fastPairwiseParamatersDialog_->exec();
}

OptionSet ClustalWMsaBuilderOptionsWidget::getPairwiseParameterOptions() const
{
    OptionSet pairwiseOptions;
    if (ui_->slowPairwiseAlignmentsRadioButton_->isChecked())
    {
        if (slowPairwiseParamatersDialog_ != nullptr)
            pairwiseOptions = slowPairwiseParamatersDialog_->pairwiseOptions();
    }
    else
    {
        if (fastPairwiseParamatersDialog_ != nullptr)
            pairwiseOptions = fastPairwiseParamatersDialog_->pairwiseOptions();
    }
    return pairwiseOptions;
}

void ClustalWMsaBuilderOptionsWidget::setWeightMatricesBasedOnGrammar()
{
    ui_->weightMatrixComboBox_->clear();

    QStringList weightMatrices;
    QString defaultWeightMatrix;
    using namespace constants::ClustalW;
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

QString ClustalWMsaBuilderOptionsWidget::msaMatrixOptionKey() const
{
    using namespace constants::ClustalW;
    if (grammar_ == eAminoGrammar)
        return kMsaProteinMatrix;
    else if (grammar_ == eDnaGrammar)
        return kMsaDnaMatrix;

    return QString();
}

QString ClustalWMsaBuilderOptionsWidget::outputOrder() const
{
    using namespace constants::ClustalW;
    if (ui_->orderByAlignedRadioButton_->isChecked())
        return kOrderByAligned;
    else if (ui_->orderByInputSequencesRadioButton_->isChecked())
        return kOrderByInput;

    return QString();
}

QString ClustalWMsaBuilderOptionsWidget::guideTreeAlgorithm() const
{
    using namespace constants::ClustalW;
    if (ui_->guideByNJRadioButton_->isChecked())
        return kGuideTreeNJ;
    else if (ui_->guideByUPGMARadioButton_->isChecked())
        return kGuideTreeUPGMA;

    return QString();
}

QString ClustalWMsaBuilderOptionsWidget::iterationMethod() const
{
    using namespace constants::ClustalW;
    if (ui_->iterateByAlignmentRadioButton_->isChecked())
        return kIterationAlignment;
    else if (ui_->iterateByTreeRadioButton_->isChecked())
        return kIterationTree;

    return kIterationNone;
}
