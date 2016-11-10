/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSOPTIONSDIALOG_H
#define CONSENSUSOPTIONSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class ConsensusOptionsDialog;
}

class QCheckBox;
class QSpinBox;

class ConsensusOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConsensusOptionsDialog(QWidget *parent = 0);
    ~ConsensusOptionsDialog();


    QVector<double> thresholds() const;
    void setThresholds(const QVector<double> &newThresholds);


private:
    Ui::ConsensusOptionsDialog *ui_;
    QVector<QSpinBox *> thresholdSpinBoxes_;
    QVector<QCheckBox *> enabledCheckBoxes_;
};

#endif // CONSENSUSOPTIONSDIALOG_H
