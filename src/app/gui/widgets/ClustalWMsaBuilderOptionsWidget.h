/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALWMSABUILDEROPTIONSWIDGET_H
#define CLUSTALWMSABUILDEROPTIONSWIDGET_H

#include <QtGui/QWidget>

#include "IMsaBuilderOptionsWidget.h"
#include "../../core/enums.h"

namespace Ui {
    class ClustalWMsaBuilderOptionsWidget;
}

class FastPairwiseParametersDialog;
class SlowPairwiseParametersDialog;

class ClustalWMsaBuilderOptionsWidget : public IMsaBuilderOptionsWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit ClustalWMsaBuilderOptionsWidget(const Grammar grammar, QWidget *parent = nullptr);
    ~ClustalWMsaBuilderOptionsWidget();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    OptionSet msaBuilderOptions() const;
    void setGrammar(const Grammar newGrammar);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void showPairwiseAlignmentParameters();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setPenaltySpinBoxesPrecision(const int decimals);
    void setPenaltySpinBoxesVisiblePrecision(const int decimals);
    void displaySlowPairwiseParametersDialog();
    void displayFastPairwiseParametersDialog();
    OptionSet getPairwiseParameterOptions() const;
    void setWeightMatricesBasedOnGrammar();
    QString msaMatrixOptionKey() const;
    QString outputOrder() const;
    QString guideTreeAlgorithm() const;
    QString iterationMethod() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::ClustalWMsaBuilderOptionsWidget *ui_;
    Grammar grammar_;
    SlowPairwiseParametersDialog *slowPairwiseParamatersDialog_;
    FastPairwiseParametersDialog *fastPairwiseParamatersDialog_;
};

#endif // CLUSTALWMSABUILDEROPTIONSWIDGET_H
