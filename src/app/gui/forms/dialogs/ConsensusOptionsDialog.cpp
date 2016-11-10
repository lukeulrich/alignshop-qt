/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ConsensusOptionsDialog.h"
#include "ui_ConsensusOptionsDialog.h"
#include "../../../core/macros.h"

ConsensusOptionsDialog::ConsensusOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::ConsensusOptionsDialog)
{
    ui_->setupUi(this);

    thresholdSpinBoxes_ << ui_->thresholdSpinBox1_
                        << ui_->thresholdSpinBox2_
                        << ui_->thresholdSpinBox3_
                        << ui_->thresholdSpinBox4_
                        << ui_->thresholdSpinBox5_;

    enabledCheckBoxes_ << ui_->enabledCheckbox1_
                       << ui_->enabledCheckbox2_
                       << ui_->enabledCheckbox3_
                       << ui_->enabledCheckbox4_
                       << ui_->enabledCheckbox5_;

}

ConsensusOptionsDialog::~ConsensusOptionsDialog()
{
    delete ui_;
}

QVector<double> ConsensusOptionsDialog::thresholds() const
{
    QVector<double> enabledThresholds;
    foreach (const QSpinBox *spinBox, thresholdSpinBoxes_)
        if (spinBox->isEnabled())
            enabledThresholds << static_cast<double>(spinBox->value()) / 100.;
    return enabledThresholds;
}

void ConsensusOptionsDialog::setThresholds(const QVector<double> &newThresholds)
{
    int i = 0;
    foreach (const double newThreshold, newThresholds)
    {
        ASSERT(newThreshold >= .5 && newThreshold <= 1.);
        thresholdSpinBoxes_[i]->setValue(newThreshold * 100);
        enabledCheckBoxes_[i]->setChecked(true);

        ++i;
        if (i == thresholdSpinBoxes_.size())
            break;
    }

    for (int j=i; j< thresholdSpinBoxes_.size(); ++j)
    {
        enabledCheckBoxes_[j]->setChecked(false);
        thresholdSpinBoxes_[j]->setDisabled(true);
    }
}
