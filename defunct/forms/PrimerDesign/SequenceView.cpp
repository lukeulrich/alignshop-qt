#include "SequenceView.h"
#include "ui_SequenceView.h"
#include <PrimerDesign/PrimerDesignFile.h>

using namespace PrimerDesign;

const char *MAIN_TITLE = "Primer Designer - ";
const char *SETTINGS_FILE_NAME = "settings.txt";

SequenceView::SequenceView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SequenceView)
{
    ui->setupUi(this);

    bindToObjectModel();

    onCurrentChanged();

    if (!loadLastFile())
    {
        setCurrentFileName("Untitled.prm");
        load();
    }
}

SequenceView::~SequenceView()
{
    delete ui;
}

void SequenceView::bindToObjectModel()
{
    ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;
    sequences_.bind(&groups);
    sortedModel_.setSourceModel(&sequences_);
    ui->listView->installEventFilter(this);
    ui->listView->setModel(&sortedModel_);

    QObject::connect(
        ui->listView->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this,
        SLOT(onSelectionChanged(QItemSelection, QItemSelection)));

    QObject::connect(
        groups.notifier(),
        SIGNAL(itemAdded(int)),
        this,
        SLOT(onSequenceAdded(int)));

    QObject::connect(
        groups.notifier(),
        SIGNAL(currentChanged()),
        this,
        SLOT(onCurrentChanged()));

    primerDesignOM_.primerPairGroups.setCurrentIndex(0);
}

void SequenceView::onSequenceAdded(int index)
{
    ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;
    ObservableSequence &sequence = groups.at(index)->sequence;

    QObject::connect(
        &sequence,
        SIGNAL(nameChanged(ObservableSequence*)),
        this,
        SLOT(onNameChanged(ObservableSequence*)));

    QModelIndex unsortedIndex = sequences_.index(index, 0);
    QModelIndex sortedIndex = sortedModel_.mapFromSource(unsortedIndex);
    ui->listView->setCurrentIndex(sortedIndex);

    sort();
}

void SequenceView::onAddSequenceClicked()
{
    ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;
    NewSequenceDialog newSeqDialog(this);

    if (newSeqDialog.exec() == QDialog::Accepted)
    {
        DnaSequence sequence = newSeqDialog.toSequence();
        PrimerPairGroup *group = new PrimerPairGroup();
        group->sequence.setName(sequence.name());
        group->sequence.setSequence(sequence.sequence());
        group->sequence.setNotes(sequence.notes());
        groups.add(group);
        groups.setCurrentIndex(groups.indexOf(group));
    }
}


//2011-05-18 Needs changed to allow multiple selections in the PrimerPair list
void SequenceView::onSelectionChanged(const QItemSelection & selected, const QItemSelection & /* deselected */)
{
    ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;

    if (selected.indexes().count())
    {
        QModelIndex index = sortedModel_.mapToSource(selected.indexes().first());
        groups.setCurrentIndex(index.row());
    }
    else
    {
        groups.setCurrentIndex(Row::ObservableList::NO_CURRENT);
    }
}

void SequenceView::updateSequenceListViewSelectedIndex()
{
    ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;
    QModelIndex selectedIndex = sortedModel_.mapFromSource(sequences_.index(groups.currentIndex(), 0));
    if (ui->listView->selectionModel()->currentIndex() != selectedIndex)
    {
        ui->listView->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
    }
}

void SequenceView::toggleZeroItemsMessageVisibility()
{
    if (primerDesignOM()->primerPairGroups.current())
    {
        ui->sequenceDetails->setVisible(true);
        ui->zeroItemsFrame->setVisible(false);
    }
    else
    {
        ui->sequenceDetails->setVisible(false);
        ui->zeroItemsFrame->setVisible(true);
    }
}

void SequenceView::onCurrentChanged()
{
    updateSequenceListViewSelectedIndex();
    toggleZeroItemsMessageVisibility();
}

void SequenceView::sort()
{
    sortedModel_.sort(0);
    ui->listView->scrollTo(ui->listView->currentIndex());
}

bool SequenceView::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->listView && event->type() == QEvent::KeyPress)
    {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
         if (keyEvent->key() == Qt::Key_Delete)
         {
             QItemSelection s = ui->listView->selectionModel()->selection();
             QModelIndex index = sortedModel_.mapToSource(s.indexes().first());
             ObservablePrimerPairGroupList &groups = primerDesignOM()->primerPairGroups;
             int row = index.row();

             QString itemName = groups.at(row)->sequence.name();
             QString promtTitle = tr("''%1'' Deletion").arg(itemName);
             QString promptText =
                 tr("The sequence ''%1'' and all of its primers are about to be deleted.  This cannot be undone. Are you sure you want to continue?")
                     .arg(itemName);

             if (QMessageBox::question(
                 this,
                 promtTitle,
                 promptText,
                 QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
             {
                 groups.removeAt(row);
                 groups.setCurrentIndex(row >= groups.length() ? row - 1 : row);
                 return true;
             }
         }
     }

    return false;
}

void SequenceView::onNameChanged(ObservableSequence * /* sequence */)
{
    sort();
}

void SequenceView::setCurrentFileName(const QString &fileName)
{
    currentFileName_ = fileName;
    QString correctFileName = (fileName.isEmpty() ? "Unnamed Document" : fileName);
    QString title = QString(MAIN_TITLE);
    this->setWindowTitle(title + correctFileName);
}

void SequenceView::load()
{
    primerDesignOM_.primerPairGroups.clear();
    QFile file(currentFileName_);
    file.open(QFile::ReadOnly);
    QTextStream reader(&file);
    PrimerDesignFile::deserialize(primerDesignOM_.primerPairGroups, reader.readAll());
    primerDesignOM_.primerPairGroups.setCurrentIndex(0);
}

void SequenceView::save()
{
    if (!currentFileName_.isEmpty())
    {
        writeToFile();
    }
    else if (primerDesignOM_.primerPairGroups.length())
    {
        saveAs();
    }
}

void SequenceView::closeEvent(QCloseEvent * /* e */)
{
    save();
    rememberCurrentFileName();
}

void SequenceView::writeToFile()
{
    QFile file(currentFileName_);
    file.open(QFile::Truncate | QFile::WriteOnly);
    QTextStream writer(&file);
    writer << PrimerDesignFile::serialize(primerDesignOM()->primerPairGroups);
}

PrimerDesignOM * SequenceView::primerDesignOM()
{
    return &primerDesignOM_;
}

void SequenceView::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Dataset"),
            QDir::currentPath(),
            tr("Primer Designer Files (*.prm)"));

    if (!filename.isEmpty())
    {
        setCurrentFileName(filename);
        writeToFile();
    }
}

void SequenceView::onFileOpenClicked()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Open Dataset"),
            QDir::currentPath(),
            tr("Primer Designer Files (*.prm)"));

    if(!filename.isEmpty())
    {
        save();
        setCurrentFileName(filename);
        load();
    }
}

void SequenceView::rememberCurrentFileName()
{
    QFile file(SETTINGS_FILE_NAME);
    file.open(QFile::Truncate | QFile::WriteOnly);
    QTextStream writer(&file);
    writer << currentFileName_;
}

bool SequenceView::loadLastFile()
{
    QFile file(SETTINGS_FILE_NAME);
    file.open(QFile::ReadOnly);
    QTextStream reader(&file);
    QString lastFileName = reader.readAll().trimmed();

    if (lastFileName.isEmpty())
    {
        return false;
    }

    setCurrentFileName(lastFileName);
    load();
    return true;
}
