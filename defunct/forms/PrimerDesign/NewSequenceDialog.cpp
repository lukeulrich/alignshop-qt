#include "NewSequenceDialog.h"
#include "ui_NewSequenceDialog.h"

#include <QtGui>
#include "global.h"
#include "SequenceValidator.h"

using namespace PrimerDesign;

NewSequenceDialog::NewSequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewSequenceDialog)
{
    ui->setupUi(this);
    QPushButton *okButton = ui->dialogChoices->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);
}

NewSequenceDialog::~NewSequenceDialog()
{
    delete ui;
}

void NewSequenceDialog::enableOkIfValidForm()
{
    QPushButton *okButton = ui->dialogChoices->button(QDialogButtonBox::Ok);
    bool enabled = (validateName() && validateSequence());
    okButton->setEnabled(enabled);
}

bool NewSequenceDialog::validateName()
{
    if (ui->nameValue->text().trimmed().isEmpty())
    {
        ui->nameLabel->setError("Name cannot be blank.");
        return false;
    }

    ui->nameLabel->clearError();
    return true;
}

bool NewSequenceDialog::validateSequence()
{
    SequenceValidator validator;
    QString value = ui->sequenceValue->toPlainText();
    int position = 0;
    bool isValid = (validator.validate(value, position) == QValidator::Acceptable);

    if (!isValid)
    {
        ui->sequenceLabel->setError("The DNA sequence contains invalid characters.");
    }
    else if (value.isEmpty())
    {
        ui->sequenceLabel->setError("The DNA sequence cannot be blank.");
        isValid = false;
    }
    else
    {
        ui->sequenceLabel->clearError();
    }

    if (value != ui->sequenceValue->toPlainText())
    {
        int cursorPosition = ui->sequenceValue->textCursor().position();
        ui->sequenceValue->setText(value);
        QTextCursor cursor = ui->sequenceValue->textCursor();
        cursor.setPosition(qMin(cursorPosition, value.length()));
        ui->sequenceValue->setTextCursor(cursor);
    }

    return isValid;
}

DnaSequence NewSequenceDialog::toSequence()
{
    DnaSequence seq;
    seq.setName(ui->nameValue->text());
    seq.setSequence(ui->sequenceValue->toPlainText());
    seq.setNotes(ui->notesValue->toPlainText());
    return seq;
}
