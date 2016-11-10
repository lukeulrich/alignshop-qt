#include "PrimerDesignInputPage.h"
#include "ui_PrimerDesignInputPage.h"
#include <PrimerDesign/Range.h>
#include <forms/RestrictionEnzymeDialog.h>
#include <forms/PrimerDesign/PrimerGenerationDialog.h>

using namespace PrimerDesign;
using namespace Widgets;

PrimerDesignInputPage::PrimerDesignInputPage(PrimerDesignWizard *parent) :
    PrimerDesignBasePage(parent),
    ui(new Ui::PrimerDesignInputPage)
{
    ui->setupUi(this);
    worker_ = 0;
    hideError();

    ui->forwardPrefixValue->setValidator(&seqValidator_);
    ui->reversePrefixValue->setValidator(&seqValidator_);
}

PrimerDesignInputPage::~PrimerDesignInputPage()
{
    delete ui;
}

void PrimerDesignInputPage::cleanupPage()
{
    PrimerDesignInput *params = baseWizard()->getInput();
    params->ampliconBounds = Range(ui->ampliconStartValue->value(), ui->ampliconStopValue->value());
    params->ampliconSizeRange = Range(ui->ampliconMinValue->value(), ui->ampliconMaxValue->value());
    params->forwardPrefix = ui->forwardPrefixValue->restrictionEnzyme().value;
    params->primerSizeRange = Range(ui->primerMinValue->value(), ui->primerMaxValue->value());
    params->reversePrefix = ui->reversePrefixValue->restrictionEnzyme().value;
    params->tmRange = RangeF(ui->tmMinValue->value(), ui->tmMaxValue->value());
    params->forwardSuffix = ui->forwardSuffix->text();
    params->reverseSuffix = ui->reverseSuffix->text();
    params->sodiumConcentration = ui->sodiumConcentration->value();
}

void PrimerDesignInputPage::initializePage()
{
    PrimerDesignInput *params = baseWizard()->getInput();
    ui->PrimerDesignInputPageValue->setSequence(params->amplicon);

    windowBounds_ = AmpliconRange(1, params->amplicon.length());
    setWindowBounds(params->ampliconBounds.min(), params->ampliconBounds.max());

    ui->ampliconMaxValue->setValue(params->ampliconSizeRange.max());
    ui->ampliconMinValue->setValue(params->ampliconSizeRange.min());

    ui->primerMaxValue->setValue(params->primerSizeRange.max());
    ui->primerMinValue->setValue(params->primerSizeRange.min());

    ui->tmMaxValue->setValue(params->tmRange.max());
    ui->tmMinValue->setValue(params->tmRange.min());

    ui->forwardPrefixValue->setText(params->forwardPrefix);
    ui->reversePrefixValue->setText(params->reversePrefix);

    ui->forwardSuffix->setText(params->forwardSuffix);
    ui->reverseSuffix->setText(params->reverseSuffix);

    ui->sodiumConcentration->setValue(params->sodiumConcentration);

    ui->ampliconMaxValue->setMaximum(params->amplicon.length());
}

bool PrimerDesignInputPage::validatePage()
{
    cleanupPage();
    hideError();

    PrimerDesignInput *inputParams = baseWizard()->getInput();
    QString errorMessage = inputParams->getErrorMessage();

    if (errorMessage.isEmpty())
    {
        PrimerGenerationDialog dlg(inputParams, this);
        if (dlg.exec() == QDialog::Rejected &&
            dlg.finderResult().value.count() == 0)
        {
            return false;
        }

        baseWizard()->setFinderResult(dlg.finderResult());
        errorMessage = dlg.finderResult().errorMessage;
    }

    if (!errorMessage.isEmpty())
    {
        ui->errorMessageValue->setError(errorMessage);
        ui->errorMessageGroup->show();
        return false;
    }
    else
    {
        return true;
    }
}

void PrimerDesignInputPage::onSelectionChanged()
{
    Selection *current = ui->PrimerDesignInputPageValue->currentSelection();

    if (current)
    {
        int start = current->start;
        int stop = current->stop;

        if (windowBounds_.setRange(start + 1, stop + 1))
        {
            onWindowRangeChanged();
        }
    }
}

void PrimerDesignInputPage::onWindowRangeChanged()
{
    updateMinAndMaxElements();
    updateSelection();
}

void PrimerDesignInputPage::updateSelection()
{
    Selection *current = ui->PrimerDesignInputPageValue->currentSelection();

    if (!current)
    {
        ui->PrimerDesignInputPageValue->addSelection(windowBounds_.getStart() - 1, windowBounds_.getStop() - 1);
    }
    else
    {
        current->start = windowBounds_.getStart() - 1;
        current->stop = windowBounds_.getStop() - 1;
        ui->PrimerDesignInputPageValue->viewport()->update();
    }
}

void PrimerDesignInputPage::updateMinAndMaxElements()
{
    ui->ampliconStartValue->setValue(windowBounds_.getStart());
    ui->ampliconStopValue->setValue(windowBounds_.getStop());
    ui->ampliconMaxValue->setValue(windowBounds_.getStop()-windowBounds_.getStart()+1);
    ui->ampliconMinValue->setValue(windowBounds_.getStop()-windowBounds_.getStart()+1);
    ui->ampliconLengthValue->setText(QString::number(1 + windowBounds_.getStop() - windowBounds_.getStart()));
}

void PrimerDesignInputPage::onStartChanged()
{
    setWindowBounds(ui->ampliconStartValue->value(), windowBounds_.getStop());
}

void PrimerDesignInputPage::onStopChanged()
{
    setWindowBounds(windowBounds_.getStart(), ui->ampliconStopValue->value());
}

void PrimerDesignInputPage::setWindowBounds(int start, int stop)
{
    windowBounds_.setRange(start, stop);
    onWindowRangeChanged();
}

void PrimerDesignInputPage::browse(RestrictionEnzymeTextbox *txt, QString captionParam)
{
    RestrictionEnzymeDialog dlg(this);
    dlg.setWindowTitle(tr("%0 restriction enzyme browser").arg(captionParam));
    dlg.setSelectedRestrictionSite(txt->restrictionEnzyme().name);
    if (dlg.exec() == QDialog::Accepted)
    {
        RestrictionEnzyme enzyme;
        enzyme.name = dlg.selectedRestrictionSiteName();
        enzyme.value = dlg.selectedRestrictionSite();
        txt->setRestrictionEnzyme(enzyme);
    }
}

void PrimerDesignInputPage::forwardPrefixBrowseClicked()
{
    browse(ui->forwardPrefixValue, tr("Forward"));
}

void PrimerDesignInputPage::reversePrefixBrowseClicked()
{
    browse(ui->reversePrefixValue, tr("Reverse"));
}

void PrimerDesign::PrimerDesignInputPage::hideError()
{
    ui->errorMessageGroup->hide();
}
