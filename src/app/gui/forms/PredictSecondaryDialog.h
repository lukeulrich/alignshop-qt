/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PREDICTSECONDARYDIALOG_H
#define PREDICTSECONDARYDIALOG_H

#include <QtCore/QVector>
#include <QtGui/QDialog>

#include "../../core/util/OptionSet.h"
#include "../../core/global.h"

namespace Ui {
    class PredictSecondaryDialog;
}

class QModelIndex;
class QPersistentModelIndex;

// TODO: Consolidate the functionality shared by this dialog and BlastDialog
class PredictSecondaryDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit PredictSecondaryDialog(QWidget *parent = nullptr);
    ~PredictSecondaryDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QModelIndex blastDatabaseIndex() const;             //!< Returns the currently selected blast database index
    int nThreads() const;
    OptionSet psiBlastOptions() const;
    void setBlastDatabaseIndices(const QVector<QPersistentModelIndex> &blastDatabaseIndices);
    void setMaxThreads(int nThreads);


private:
    Ui::PredictSecondaryDialog *ui_;
    QVector<QPersistentModelIndex> blastDatabaseIndices_;
};

#endif // PREDICTSECONDARYDIALOG_H
