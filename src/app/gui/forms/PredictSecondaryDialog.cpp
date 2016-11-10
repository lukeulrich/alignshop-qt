/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QPersistentModelIndex>

#include <QtGui/QDoubleValidator>
#include <QtGui/QPushButton>

#include "PredictSecondaryDialog.h"
#include "ui_PredictSecondaryDialog.h"

#include "../models/CustomRoles.h"
#include "../../core/constants/PsiBlastConstants.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
PredictSecondaryDialog::PredictSecondaryDialog(QWidget *parent)
    : QDialog(parent),
    ui_(new Ui::PredictSecondaryDialog)
{
    ui_->setupUi(this);

    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    ui_->evalueLineEdit->setValidator(doubleValidator);
    ui_->inclusionLineEdit->setValidator(doubleValidator);

    // Buttons
    QPushButton *okButton = ui_->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText("Start Prediction");
    okButton->setDefault(true);
    okButton->setEnabled(false);

    QPushButton *cancelButton = ui_->buttonBox->button(QDialogButtonBox::Cancel);
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
}

/**
  */
PredictSecondaryDialog::~PredictSecondaryDialog()
{
    delete ui_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QModelIndex
  */
QModelIndex PredictSecondaryDialog::blastDatabaseIndex() const
{
    if (blastDatabaseIndices_.isEmpty())
        return QModelIndex();

    return blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex());
}

/**
  * @returns int
  */
int PredictSecondaryDialog::nThreads() const
{
    return ui_->threadsSpinBox->value();
}

/**
  * @returns OptionSet
  */
OptionSet PredictSecondaryDialog::psiBlastOptions() const
{
    // Since this dialog is modal there should be no way that the blast database model could have been modified.
    ASSERT(blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex()).isValid());

    OptionSet options;

    using namespace constants::PsiBlast;
    options << Option(kDatabaseOpt,
                      blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex()).data(CustomRoles::kFullBlastPathRole).toString());
    options << Option(kEvalueOpt, ui_->evalueLineEdit->text());
    options << Option(kIterationsOpt, ui_->iterationsSpinBox->value());
    options << Option(kInclusionEThreshOpt, ui_->inclusionLineEdit->text());

    return options;
}

/**
  * @param blastDatabaseIndices [const QVector<QPersistentModelIndex> &]
  */
void PredictSecondaryDialog::setBlastDatabaseIndices(const QVector<QPersistentModelIndex> &blastDatabaseIndices)
{
    QModelIndex current;
    if (ui_->blastDatabaseComboBox->currentIndex() != -1)
        current = blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex());

    ui_->blastDatabaseComboBox->clear();

    blastDatabaseIndices_ = blastDatabaseIndices;
    foreach (const QPersistentModelIndex &index, blastDatabaseIndices_)
        ui_->blastDatabaseComboBox->addItem(index.data().toString());

    int index = blastDatabaseIndices_.indexOf(current);
    if (index != -1)
        ui_->blastDatabaseComboBox->setCurrentIndex(index);

    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui_->blastDatabaseComboBox->count() > 0);
}

/**
  * @param nThreads [int]
  */
void PredictSecondaryDialog::setMaxThreads(int nThreads)
{
    ASSERT(nThreads > 0);
    ui_->threadsSpinBox->setMaximum(nThreads);
    ui_->threadsSlider->setMaximum(nThreads);
}
