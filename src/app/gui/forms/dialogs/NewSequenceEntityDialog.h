/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NEWSEQUENCEENTITYDIALOG_H
#define NEWSEQUENCEENTITYDIALOG_H

#include <QtGui/QDialog>
#include "../../../core/global.h"
#include "../../../core/Entities/IEntity.h"

namespace Ui {
    class NewSequenceEntityDialog;
}

class QMenu;

class BioString;
class DynamicSeqFactory;
class InvalidCharsHighligher;

class NewSequenceEntityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewSequenceEntityDialog(DynamicSeqFactory *dynamicSeqFactory, QWidget *parent = nullptr);
    ~NewSequenceEntityDialog();

    IEntitySPtr sequenceEntity() const;


private Q_SLOTS:
    void removeWhitespaceFromSequence();
    void maskInvalidCharactersInSequence();
    void removeInvalidCharactersFromSequence();

    void enableDisableOkButton();
    void onSequenceChanged();
    void onProteinTypeRadioButtonToggled(bool checked);
    void onDnaTypeRadioButtonToggled(bool checked);


private:
    void setupCleanupMenu();
    QString whitespaceCharacters() const;
    void setValidCharacters(const QString &characters);
    bool requiredFieldsAreSatisfied() const;
    bool hasNonEmptyName() const;
    QString trimmedName() const;
    QString trimmedSource() const;
    BioString bioString() const;
    bool isValidSequence() const;
    QString rawSequenceString() const;
    void updateLengthLabel(const int newLength);
    void updateWarningLabel();
    QChar maskCharacter() const;
    bool isProtein() const;
    void replaceSequenceTextAndPreserveUndo(const QString &newText);

    Ui::NewSequenceEntityDialog *ui_;
    DynamicSeqFactory *dynamicSeqFactory_;
    QMenu *cleanupMenu_;
    InvalidCharsHighligher *invalidCharsHighlighter_;
};

#endif // NEWSEQUENCEENTITYDIALOG_H
