/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NEWPRIMERDIALOG_H
#define NEWPRIMERDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class NewPrimerDialog;
}

class BioString;
class Primer;

class NewPrimerDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit NewPrimerDialog(QWidget *parent = 0);
    ~NewPrimerDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    Primer primer() const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onREToolButtonReleased();
    void enableDisableOkButton();
    void recalculateTm();
    void onAcceptRequest();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool requiredFieldsAreSatisfied() const;
    BioString completePrimerSequence() const;
    void setTmLabelToNullValue();
    void setTmLabelWithDouble(const double tm);
    bool primerCoreSequenceLengthIsUnusuallySmall() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::NewPrimerDialog *ui_;
};

#endif // NEWPRIMERDIALOG_H
