/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QMenu>

#include "NewSequenceEntityDialog.h"
#include "ui_NewSequenceEntityDialog.h"
#include "../../Services/InvalidCharsHighlighter.h"
#include "../../../core/factories/DynamicSeqFactory.h"
#include "../../../core/constants.h"
#include "../../../core/macros.h"
#include "../../../core/misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
NewSequenceEntityDialog::NewSequenceEntityDialog(DynamicSeqFactory *dynamicSeqFactory, QWidget *parent)
    : QDialog(parent),
      ui_(new Ui::NewSequenceEntityDialog),
      dynamicSeqFactory_(dynamicSeqFactory),
      cleanupMenu_(nullptr),
      invalidCharsHighlighter_(nullptr)
{
    ui_->setupUi(this);
    setupCleanupMenu();
    ui_->cleanupButton_->setMenu(cleanupMenu_);
    ui_->buttonBox_->button(QDialogButtonBox::Ok)->setText("Add Sequence");
    invalidCharsHighlighter_ = new InvalidCharsHighligher(ui_->sequenceTextEdit_);

    // Signals
    connect(ui_->nameLineEdit_, SIGNAL(textChanged(QString)), SLOT(enableDisableOkButton()));
    connect(ui_->sequenceTextEdit_, SIGNAL(textChanged()), SLOT(onSequenceChanged()));
    connect(ui_->proteinTypeRadioButton_, SIGNAL(toggled(bool)), SLOT(onProteinTypeRadioButtonToggled(bool)));
    connect(ui_->dnaTypeRadioButton_, SIGNAL(toggled(bool)), SLOT(onDnaTypeRadioButtonToggled(bool)));

    // Miscellaneous first-time setup:
    ui_->warningLabel_->hide();
    ui_->numCharsLabel_->hide();
    ui_->buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(false);

    // Trigger the valid characters by default
    ui_->proteinTypeRadioButton_->setChecked(true);
    onProteinTypeRadioButtonToggled(true);
}

NewSequenceEntityDialog::~NewSequenceEntityDialog()
{
    delete ui_;
    delete cleanupMenu_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
IEntitySPtr NewSequenceEntityDialog::sequenceEntity() const
{
    if (dynamicSeqFactory_ == nullptr)
        return IEntitySPtr();

    return dynamicSeqFactory_->makeSeq(trimmedName(), trimmedSource(), bioString());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void NewSequenceEntityDialog::removeWhitespaceFromSequence()
{
    replaceSequenceTextAndPreserveUndo(rawSequenceString());
}

void NewSequenceEntityDialog::maskInvalidCharactersInSequence()
{
    QString newText = ui_->sequenceTextEdit_->toPlainText();
    newText.replace(invalidCharsHighlighter_->regExpForInvalidCharacters(), maskCharacter());
    replaceSequenceTextAndPreserveUndo(newText);
}

void NewSequenceEntityDialog::removeInvalidCharactersFromSequence()
{
    QString newText = ui_->sequenceTextEdit_->toPlainText();
    newText.replace(invalidCharsHighlighter_->regExpForInvalidCharacters(), "");
    replaceSequenceTextAndPreserveUndo(newText);
}

void NewSequenceEntityDialog::enableDisableOkButton()
{
    ui_->buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(requiredFieldsAreSatisfied());
}

void NewSequenceEntityDialog::onSequenceChanged()
{
    QString rawSequence = rawSequenceString();
    updateLengthLabel(rawSequence.length());
    updateWarningLabel();
    enableDisableOkButton();
}

void NewSequenceEntityDialog::onProteinTypeRadioButtonToggled(bool checked)
{
    if (!checked)
        return;

    QString allowedAminoCharacters = QString("%1%2%3")
                                     .arg(constants::kAminoAmbiguousCharacters)
                                     .arg(constants::kGapCharacters)
                                     .arg(whitespaceCharacters());
    setValidCharacters(allowedAminoCharacters);
}

void NewSequenceEntityDialog::onDnaTypeRadioButtonToggled(bool checked)
{
    if (!checked)
        return;

    QString allowedDnaCharacters = QString("%1%2%3")
                                   .arg(constants::kDnaAmbiguousCharacters)
                                   .arg(constants::kGapCharacters)
                                   .arg(whitespaceCharacters());
    setValidCharacters(allowedDnaCharacters);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
void NewSequenceEntityDialog::setupCleanupMenu()
{
    ASSERT(cleanupMenu_ == nullptr);

    cleanupMenu_ = new QMenu;
    QAction *removeWhitespaceAction = cleanupMenu_->addAction("Remove whitespace");
    QAction *maskAction = cleanupMenu_->addAction("Mask invalid characters");
    QAction *removeInvalidAction = cleanupMenu_->addAction("Remove invalid characters");

    connect(removeWhitespaceAction, SIGNAL(triggered()), SLOT(removeWhitespaceFromSequence()));
    connect(maskAction, SIGNAL(triggered()), SLOT(maskInvalidCharactersInSequence()));
    connect(removeInvalidAction, SIGNAL(triggered()), SLOT(removeInvalidCharactersFromSequence()));
}

QString NewSequenceEntityDialog::whitespaceCharacters() const
{
    return " \t\n\r";
}

void NewSequenceEntityDialog::setValidCharacters(const QString &characters)
{
    invalidCharsHighlighter_->setValidCharacters(characters);
    updateWarningLabel();
}

bool NewSequenceEntityDialog::requiredFieldsAreSatisfied() const
{
    return hasNonEmptyName() && isValidSequence();
}

bool NewSequenceEntityDialog::hasNonEmptyName() const
{
    return trimmedName().size() > 0;
}

QString NewSequenceEntityDialog::trimmedName() const
{
    return ui_->nameLineEdit_->text().trimmed();
}

QString NewSequenceEntityDialog::trimmedSource() const
{
    return ui_->organismLineEdit_->text().trimmed();
}

BioString NewSequenceEntityDialog::bioString() const
{
    Grammar grammar = (isProtein()) ? eAminoGrammar : eDnaGrammar;
    return BioString(rawSequenceString().toAscii(), grammar);
}

bool NewSequenceEntityDialog::isValidSequence() const
{
    QString rawSequence = rawSequenceString();
    return rawSequence.size() > 0 && invalidCharsHighlighter_->stringIsValid(rawSequence);
}

QString NewSequenceEntityDialog::rawSequenceString() const
{
    QString rawSequence = ui_->sequenceTextEdit_->toPlainText();
    ::removeWhiteSpace(rawSequence);
    return rawSequence;
}

void NewSequenceEntityDialog::updateLengthLabel(const int newLength)
{
    if (newLength == 0)
    {
        ui_->numCharsLabel_->hide();
        return;
    }

    QString typeSuffix = (isProtein()) ? "Amino acid" : "Nucleotide";
    QString text = QString("%1 %2").arg(newLength).arg(typeSuffix);
    if (newLength > 1)
        text += "s";

    ui_->numCharsLabel_->setText(text);
    ui_->numCharsLabel_->show();
}

void NewSequenceEntityDialog::updateWarningLabel()
{
    if (ui_->sequenceTextEdit_->toPlainText().trimmed().isEmpty())
    {
        ui_->warningLabel_->hide();
        return;
    }

    ui_->warningLabel_->setVisible(!isValidSequence());
}

QChar NewSequenceEntityDialog::maskCharacter() const
{
    if (ui_->proteinTypeRadioButton_->isChecked())
        return constants::kAminoAnyCharacter;
    else
        return constants::kDnaAnyCharacter;
}

bool NewSequenceEntityDialog::isProtein() const
{
    return ui_->proteinTypeRadioButton_->isChecked();
}

void NewSequenceEntityDialog::replaceSequenceTextAndPreserveUndo(const QString &newText)
{
    // Originally, to update the text, I simply called setText or setPlainText; however, this also clears the undo
    // history. To work around this, selectAll and then call insertPlainText as below.
    // Reference: http://www.qtforum.org/article/27857/preserving-undo-redo-on-text-replacement-in-qtextedit.html
    ui_->sequenceTextEdit_->selectAll();
    ui_->sequenceTextEdit_->insertPlainText(newText);
}
