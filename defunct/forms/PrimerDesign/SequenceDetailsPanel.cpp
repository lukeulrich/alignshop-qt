#include "SequenceDetailsPanel.h"
#include "ui_SequenceDetailsPanel.h"
#include <forms/PrimerDesign/SequenceView.h>
#include <PrimerDesign/IPrimerDesignOM.h>

SequenceDetailsPanel::SequenceDetailsPanel(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SequenceDetailsPanel)
{
    ui->setupUi(this);

    ObservablePrimerPairGroupList &groups = IPrimerDesignOMProvider::groups(this);

    QObject::connect(ui->nameValue, SIGNAL(editingFinished()), this, SLOT(onSequenceNameValueEdited()));
    QObject::connect(ui->notesValue, SIGNAL(textChanged()), this, SLOT(onSequenceNotesValueEdited()));
    QObject::connect(groups.notifier(), SIGNAL(currentChanged()), this, SLOT(onCurrentChanged()));

    highlighter_ = new PrimerPairHighlighter(ui->sequenceValue);
}

SequenceDetailsPanel::~SequenceDetailsPanel()
{
    delete ui;
    delete highlighter_;
}

void SequenceDetailsPanel::onCurrentChanged()
{
    ObservableSequence *current = IPrimerDesignOMProvider::currentSequence(this);
    if (current)
    {
        QObject::connect(current, SIGNAL(nameChanged(ObservableSequence*)), this, SLOT(nameChanged()));
        ui->nameValue->setText(current->name());
        ui->sequenceValue->setSequence(current->sequence());
        ui->notesValue->setText(current->notes());
    }

    highlighter_->setCurrentList(IPrimerDesignOMProvider::currentPrimerPairs(this));
}

void SequenceDetailsPanel::onCurrentChanging()
{
    ObservableSequence *current = IPrimerDesignOMProvider::currentSequence(this);
    if (current)
    {
        QObject::disconnect(current, SIGNAL(nameChanged(ObservableSequence*)), this, SLOT(nameChanged()));
        ui->nameLabel->setText("");
        ui->sequenceValue->setSequence("");
        ui->notesValue->setText("");
    }
}

void SequenceDetailsPanel::nameChanged()
{
    ObservableSequence *current = IPrimerDesignOMProvider::currentSequence(this);
    if (current)
    {
        ui->nameValue->setText(current->name());
    }
}

void SequenceDetailsPanel::onSequenceNameValueEdited()
{
    DnaSequence *current = IPrimerDesignOMProvider::currentSequence(this);

    if (current && !current->setName(ui->nameValue->text()))
    {
        ui->nameLabel->setError("Invalid name.");
    }
    else
    {
        ui->nameLabel->clearError();
    }
}

void SequenceDetailsPanel::onSequenceNotesValueEdited()
{
    DnaSequence *current = IPrimerDesignOMProvider::currentSequence(this);

    if (current)
    {
        current->setNotes(ui->notesValue->toPlainText());
    }
}
