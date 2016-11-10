/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QPersistentModelIndex>

#include <QtGui/QDoubleValidator>
#include <QtGui/QPushButton>

#include "BlastDialog.h"
#include "ui_BlastDialog.h"

#include "../models/CustomRoles.h"
#include "../../core/constants.h"
#include "../../core/constants/PsiBlastConstants.h"
#include "../../core/macros.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QWidget *]
  */
BlastDialog::BlastDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::BlastDialog)
{
    ui_->setupUi(this);

    // Matrices - do not apply for nucleotide searches
    ui_->matrixComboBox->addItems(constants::PsiBlast::kMatrixList);
    int blosum62_index = ui_->matrixComboBox->findText(constants::PsiBlast::kBLOSUM62);
    ASSERT(blosum62_index != -1);
    ui_->matrixComboBox->setCurrentIndex(blosum62_index);

    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    ui_->evalueLineEdit->setValidator(doubleValidator);
    ui_->inclusionLineEdit->setValidator(doubleValidator);

    // Buttons
    QPushButton *okButton = ui_->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText("BLAST!");
    okButton->setDefault(true);
    okButton->setEnabled(false);
}

/**
  */
BlastDialog::~BlastDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QModelIndex
  */
QModelIndex BlastDialog::blastDatabaseIndex() const
{
    if (blastDatabaseIndices_.isEmpty())
        return QModelIndex();

    return blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex());
}

/**
  * @returns QVector<Option>
  */
OptionSet BlastDialog::blastOptions() const
{
    // Since this dialog is modal there should be no way that the blast database model could have been modified.
    ASSERT(blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex()).isValid());

    OptionSet options;

    using namespace constants::PsiBlast;
    options << Option(kDatabaseOpt,
                      blastDatabaseIndices_.at(ui_->blastDatabaseComboBox->currentIndex()).data(CustomRoles::kFullBlastPathRole).toString());
    options << Option(kEvalueOpt, ui_->evalueLineEdit->text());
    options << Option(kMatrixOpt, ui_->matrixComboBox->currentText());
    // 17 Nov 2011
    // It appears that to have the proper pruning of sequences, must use the num_alignments option by itself.
    options << Option(kNumAlignmentsOpt, ui_->maxHitsSpinBox->value());
    if (ui_->psiBlastCheckBox->isChecked())
    {
        options << Option(kIterationsOpt, ui_->iterationsSpinBox->value());
        options << Option(kInclusionEThreshOpt, ui_->inclusionLineEdit->text());
    }
    options << Option(kOutputFormatOpt, eXmlOutput);

    return options;
}

/**
  * @returns int
  */
int BlastDialog::nThreads() const
{
    return ui_->threadsSpinBox->value();
}

/**
  * @returns QTableWidget *
  */
QTableWidget * BlastDialog::sequenceTableWidget() const
{
    return ui_->sequenceTableWidget;
}

/**
  * @param blastDatabaseIndices [const QVector<QPersistentModelIndex> &]
  */
void BlastDialog::setBlastDatabaseIndices(const QVector<QPersistentModelIndex> &blastDatabaseIndices)
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
void BlastDialog::setMaxThreads(int nThreads)
{
    ASSERT(nThreads > 0);
    ui_->threadsSpinBox->setMaximum(nThreads);
    ui_->threadsSlider->setMaximum(nThreads);
}
