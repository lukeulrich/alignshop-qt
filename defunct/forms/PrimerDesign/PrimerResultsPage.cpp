#include "PrimerResultsPage.h"
#include "ui_PrimerResultsPage.h"
#include "PrimerDesign/PrimerPair.h"
#include "PrimerDesign/Primer.h"
#include <QtCore/QList>
#include <QMessageBox>
#include <QtAlgorithms>
#include <QTextEdit>
#include "DnaString.h"
#include <forms/PrimerDesign/PrimerPairHighlighter.h>

using namespace PrimerDesign;
using namespace Widgets;

PrimerResultsPage::PrimerResultsPage(PrimerDesignWizard *parent) :
    PrimerDesignBasePage(parent),
    ui(new Ui::PrimerResultsPage)
{
    ui->setupUi(this);
    model_ = new PrimerPairListModel(true);
    ui->PrimerDesignInputPageValue->setAllowSelect(false);
}

PrimerResultsPage::~PrimerResultsPage()
{
    delete ui;
    delete model_;
}

bool PrimerResultsPage::validatePage()
{
    PrimerPairFinderResult *result = baseWizard()->getFinderResult();
    QList<PrimerPair> pairs;

    QSet<int> includedRows;
    QModelIndexList indices = ui->primerResultList->selectionModel()->selectedIndexes();
    for (int i = 0; i < indices.count(); ++i)
    {
        int row = indices.at(i).row();
        if (!includedRows.contains(row))
        {
            includedRows << row;
            pairs.append(result->value.at(row));
        }
    }

    if (!pairs.length())
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setText("Please select one or more primers from the list.");
        msg.exec();
        return false;
    }

    result->value = pairs;

    return true;
}

void PrimerResultsPage::initializePage()
{
    PrimerPairFinderResult *result = baseWizard()->getFinderResult();
    PrimerDesignInput *params = baseWizard()->getInput();
    ui->PrimerDesignInputPageValue->setSequence(params->amplicon);

    list_.clear();

    for (int i = 0; i < result->value.length(); ++i)
    {
        list_.add(new PrimerPair(result->value[i]));
    }

    model_->bind(&list_);
    ui->primerResultList->setModel(model_);

    QObject::connect(
        ui->primerResultList->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this,
        SLOT(onPrimerSelectionChanged(QItemSelection,QItemSelection)));

    ui->primerResultList->selectRow(0);

    ui->primerResultList->resizeColumnsToContents();
    ui->primerResultList->resizeRowsToContents();

    wizard()->setButtonText(QWizard::FinishButton, tr("Save Selected Primers"));
}

void PrimerResultsPage::onPrimerSelectionChanged(const QItemSelection & /* selected */, const QItemSelection & /* deselected */)
{
    PrimerPairHighlighter highlighter(ui->PrimerDesignInputPageValue);

    if (!ui->primerResultList->selectionModel()->hasSelection())
    {
        highlighter.clear();
    }
    else
    {
        int row = ui->primerResultList->selectionModel()->selectedIndexes().last().row();
        PrimerPair pair = baseWizard()->getFinderResult()->value[row];
        highlighter.highlight(&pair);
        ui->primerResultList->resizeRowsToContents();
    }
}
