#include "PrimerParamsPreviewDialog.h"
#include "ui_PrimerParamsPreviewDialog.h"

using namespace PrimerDesign;

PrimerParamsPreviewDialog::PrimerParamsPreviewDialog(PrimerPair *pair, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrimerParamsPreviewDialog)
{
    ui->setupUi(this);
    setWindowTitle(pair->name() + " Parameters");
    params_ = pair->params();
    ui->ampliconBoundsValue->setText(toString(params_->ampliconBounds));
    ui->ampliconLengthValue->setText(toString(params_->ampliconSizeRange));
    ui->forwardPrefifxValue->setText(params_->forwardPrefix);
    ui->forwardSuffixValue->setText(params_->forwardSuffix);
    ui->primerLengthValue->setText(toString(params_->primerSizeRange));
    ui->reversePrefixValue->setText(params_->reversePrefix);
    ui->reverseSuffixValue->setText(params_->reverseSuffix);
    ui->tmRangeValue->setText(toString(params_->tmRange));
    ui->sodiumConcentrationValue->setText(QString::number(params_->sodiumConcentration));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

PrimerParamsPreviewDialog::~PrimerParamsPreviewDialog()
{
    delete ui;
}

void PrimerParamsPreviewDialog::onCreateMoreClicked()
{
    accept();
}

QString PrimerParamsPreviewDialog::toString(const Range &range)
{
    return QString::number(range.min()) + " - " + QString::number(range.max());
}

QString PrimerParamsPreviewDialog::toString(const RangeF &range)
{
    return QString::number(range.min()) + " - " + QString::number(range.max());
}
