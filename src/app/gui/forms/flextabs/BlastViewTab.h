/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTVIEWTAB_H
#define BLASTVIEWTAB_H

#include <QtCore/QPersistentModelIndex>
#include <QtGui/QWidget>
#include "../../../core/Entities/BlastReport.h"

class QItemSelection;
class QPushButton;
class QTableView;

class BlastDatabaseWindow;
class BlastDatabaseModel;
class BlastReportModel;
class BlastSequenceImporter;

namespace Ui {
    class BlastViewTab;
}

class BlastViewTab : public QWidget
{
    Q_OBJECT

public:
    explicit BlastViewTab(QWidget *parent = 0);
    ~BlastViewTab();

    void setBlastDatabaseWindow(BlastDatabaseWindow *blastDatabaseWindow);
    void setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel);
    void setBlastSequenceImporter(BlastSequenceImporter *blastSequenceImporter);
    void setBlastReport(const BlastReportSPtr &blastReport);


public Q_SLOTS:
    void setDefaultImportIndex(const QModelIndex &index);


private Q_SLOTS:
    void onBlastTableViewSelectionChanged(const QItemSelection &selected);
    void importSelectedBlastHits();

    void onReset();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void loadBlastReport(const BlastReportSPtr &blastReport);


    Ui::BlastViewTab *ui_;
    BlastDatabaseModel *blastDatabaseModel_;
    BlastDatabaseWindow *blastDatabaseWindow_;
    BlastReportModel *blastReportModel_;
    BlastSequenceImporter *blastSequenceImporter_;
    QPersistentModelIndex defaultImportIndex_;
};

#endif // BLASTVIEWTAB_H
