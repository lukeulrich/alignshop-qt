/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDIALOG_H
#define BLASTDIALOG_H

#include <QtCore/QVector>

#include <QtGui/QDialog>

#include "../../core/util/OptionSet.h"

namespace Ui {
    class BlastDialog;
}

class QCloseEvent;
class QModelIndex;
class QPersistentModelIndex;
class QTableWidget;

class BlastDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and desctructor
    explicit BlastDialog(QWidget *parent = 0);
    ~BlastDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QModelIndex blastDatabaseIndex() const;             //!< Returns the currently selected blast database index
    OptionSet blastOptions() const;
    int nThreads() const;
    QTableWidget *sequenceTableWidget() const;
    void setBlastDatabaseIndices(const QVector<QPersistentModelIndex> &blastDatabaseIndices);
    void setMaxThreads(int nThreads);


private:
    Ui::BlastDialog *ui_;
    QVector<QPersistentModelIndex> blastDatabaseIndices_;
};

#endif // BLASTDIALOG_H
