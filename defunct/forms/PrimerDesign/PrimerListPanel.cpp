#include <forms/PrimerDesign/PrimerListPanel.h>
#include <ui_PrimerListPanel.h>
#include <forms/PrimerDesign/PrimerDesignWizard.h>
#include <PrimerDesign/IPrimerDesignOM.h>
#include <PrimerDesign/PrimerDesignInput.h>
#include <forms/PrimerDesign/PrimerParamsPreviewDialog.h>
#include <PrimerDesign/PrimerPairNamer.h>

using namespace PrimerDesign;

const int SEQUENCE_COLUMN = 1;

PrimerListPanel::PrimerListPanel(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PrimerListPanel)
{
    ui->setupUi(this);
    ui->showInfo->setEnabled(false);
    ui->actionCopy->setEnabled(false);

    ObservablePrimerPairGroupList &groups = IPrimerDesignOMProvider::groups(this);
    sortedModel_.setSourceModel(&primerPairs_);
    ui->primersGrid->setModel(&sortedModel_);
    ui->primersGrid->installEventFilter(this);
    ui->primersGrid->setSortingEnabled(false);

    QObject::connect(
        ui->primersGrid->horizontalHeader(),
        SIGNAL(sectionPressed(int)),
        this,
        SLOT(onHeaderPressed(int)));

    QObject::connect(
        ui->primersGrid->horizontalHeader(),
        SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
        this,
        SLOT(onSortIndicatorChanged(int,Qt::SortOrder)));

    QObject::connect(
        ui->primersGrid->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this,
        SLOT(onSelectionChanged(QItemSelection, QItemSelection)));
    QObject::connect(groups.notifier(), SIGNAL(currentChanging()), this, SLOT(onCurrentChanging()));
    QObject::connect(groups.notifier(), SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));

    ui->primersGrid->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->primersGrid->addAction(ui->actionCopy);
    QObject::connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(onCopyAction()));
}

PrimerListPanel::~PrimerListPanel()
{
    delete ui;
}

void PrimerListPanel::onAddPrimerClicked()
{
    addPrimer(0);
}

void PrimerListPanel::rowsChanged()
{
    ObservablePrimerPairList *list = IPrimerDesignOMProvider::currentPrimerPairs(this);
    if (list && list->length())
    {
        ui->primersGrid->setVisible(true);
        ui->primersGrid->resizeColumnsToContents();
        ui->primersGrid->resizeRowsToContents();
        ui->zeroPrimersLabel->setVisible(false);
    }
    else
    {
        ui->primersGrid->setVisible(false);
        ui->zeroPrimersLabel->setVisible(true);
    }
}

void PrimerListPanel::onCurrentChanged()
{
    ObservableListBase *currentList = IPrimerDesignOMProvider::currentPrimerPairs(this);
    if (currentList)
    {
        QObject::connect(currentList->notifier(), SIGNAL(itemAdded(int)), this, SLOT(rowsChanged()));
        QObject::connect(currentList->notifier(), SIGNAL(itemRemoved(int)), this, SLOT(rowsChanged()));
        primerPairs_.bind(currentList);
        ui->primersGrid->horizontalHeader()->setSortIndicatorShown(false);
    }

    rowsChanged();
    ui->showInfo->setEnabled(false);
}

void PrimerListPanel::onCurrentChanging()
{
    ObservableListBase *currentList = IPrimerDesignOMProvider::currentPrimerPairs(this);
    if (currentList)
    {
        QObject::disconnect(currentList->notifier(), SIGNAL(itemAdded(int)), this, SLOT(rowsChanged()));
        QObject::disconnect(currentList->notifier(), SIGNAL(itemRemoved(int)), this, SLOT(rowsChanged()));
    }
}

bool PrimerListPanel::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->primersGrid && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        ObservablePrimerPairList *currentList = IPrimerDesignOMProvider::currentPrimerPairs(this);
        int row = ui->primersGrid->currentIndex().row();
        row = sortedModel_.mapToSource(sortedModel_.index(row, 0)).row();

        if (keyEvent->matches(QKeySequence::Delete))
        {
            QString itemName = currentList->at(row)->name();
            QString promtTitle = tr("''%1'' Deletion").arg(itemName);
            QString promptText =
                tr("The primer pair ''%1'' is about to be deleted.  This cannot be undone. Are you sure you want to continue?")
                    .arg(itemName);
            if (QMessageBox::question(
                this,
                promtTitle,
                promptText,
                QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
            {
                currentList->removeAt(row);
                currentList->setCurrentIndex(row >= currentList->length() ? row - 1 : row);

                ui->primersGrid->resizeColumnsToContents();
                ui->primersGrid->resizeRowsToContents();

                return true;
            }
        }
        else if (keyEvent->matches(QKeySequence::Copy))
        {
            PrimerPair *pair = currentList->at(row);
            QString clipboardValue = pair->name()
                    + "\n"
                    + pair->forwardPrimer().sequence()
                    + "\n"
                    + pair->reversePrimer().sequence();
            QApplication::clipboard()->setText(clipboardValue);
            keyEvent->ignore();
            return true;
        }
    }

    return false;
}

void PrimerListPanel::onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    ObservableListBase *currentList = IPrimerDesignOMProvider::currentPrimerPairs(this);

    if (selected.indexes().count())
    {
        QModelIndex index = sortedModel_.mapToSource(selected.indexes().first());
        currentList->setCurrentIndex(index.row());
    }
    else
    {
        currentList->setCurrentIndex(Row::ObservableList::NO_CURRENT);
    }

    ui->showInfo->setEnabled(selected.indexes().count());
    ui->actionCopy->setEnabled(selected.indexes().count());
}

void PrimerListPanel::onShowInfoClicked()
{
    ObservablePrimerPairList *currentList = IPrimerDesignOMProvider::currentPrimerPairs(this);
    if (!currentList || currentList->currentIndex() == Row::ObservableList::NO_CURRENT)
    {
        return;
    }

    PrimerPair *pair = currentList->current();
    PrimerParamsPreviewDialog dlg(pair, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        addPrimer(pair->params());
    }
}

void PrimerListPanel::addPrimer(const PrimerDesignInput *params)
{
    DnaSequence *seq = IPrimerDesignOMProvider::currentSequence(this);
    ObservablePrimerPairList *list = IPrimerDesignOMProvider::currentPrimerPairs(this);
    if (!list || !seq)
    {
        return;
    }

    PrimerDesignWizard wizard(this, seq, params);
    wizard.setWindowTitle(tr("Primer Creator - %0").arg(seq->name()));

    if (wizard.exec() == QWizard::Accepted)
    {
        PrimerPairFinderResult *result = wizard.getFinderResult();
        PrimerPairNamer namer(seq->name(), list);

        for (int i = 0; i < result->value.length(); ++i)
        {
            PrimerPair *pair = new PrimerPair(result->value[i]);
            pair->setName(namer.nextName());
            list->add(pair);
        }

        rowsChanged();
    }
}

void PrimerListPanel::onCopyAction()
{
    QKeyEvent copyKey(QKeyEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
    QApplication::sendEvent(ui->primersGrid, &copyKey);
}

void PrimerListPanel::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (logicalIndex == SEQUENCE_COLUMN)
    {
        ui->primersGrid->horizontalHeader()->setSortIndicator(lastSortedColumn_, lastSortOrder_);
    }
    else
    {
        lastSortedColumn_ = logicalIndex;
        lastSortOrder_ = order;
    }
}

void PrimerListPanel::onHeaderPressed(int logicalIndex)
{
    if (logicalIndex == SEQUENCE_COLUMN)
    {
        return;
    }

    ui->primersGrid->sortByColumn(logicalIndex);
    ui->primersGrid->horizontalHeader()->setSortIndicatorShown(true);
}
