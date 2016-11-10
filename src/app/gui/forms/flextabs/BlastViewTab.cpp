/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QAbstractButton>
#include <QtGui/QMessageBox>
#include <QtGui/QSortFilterProxyModel>

#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>

#include "BlastViewTab.h"
#include "ui_BlastViewTab.h"
#include "../../delegates/MultiLineDelegate.h"
#include "../../forms/BlastDatabaseWindow.h"
#include "../../models/BlastReportModel.h"
#include "../../models/BlastDatabaseModel.h"
#include "../../Services/BlastSequenceImporter.h"
#include "../../../core/Parsers/xml/BlastXmlHandler.h"
#include "../../../core/Adoc.h"
#include "../../../core/global.h"
#include "../../../core/macros.h"

BlastViewTab::BlastViewTab(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::BlastViewTab),
    blastDatabaseModel_(nullptr),
    blastDatabaseWindow_(nullptr),
    blastReportModel_(nullptr),
    blastSequenceImporter_(nullptr)
{
    ui_->setupUi(this);
    blastReportModel_ = new BlastReportModel(this);
    connect(blastReportModel_, SIGNAL(modelReset()), this, SLOT(onReset()));

    QSortFilterProxyModel *blastReportSortModel = new QSortFilterProxyModel(this);
    blastReportSortModel->setSourceModel(blastReportModel_);
    ui_->blastReportTableView_->setModel(blastReportSortModel);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eScoreColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eQueryFrameColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eHitFrameColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eIdentitiesColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::ePositivesColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eGapsColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eQueryAlignColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eHitAlignColumn);
    ui_->blastReportTableView_->hideColumn(BlastReportModel::eMidlineColumn);
    ui_->blastReportTableView_->setItemDelegateForColumn(BlastReportModel::eAlignmentColumn, new MultiLineDelegate(this));

    ui_->blastReportTableView_->setModel(blastReportSortModel);
    connect(ui_->blastReportTableView_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onBlastTableViewSelectionChanged(QItemSelection)));

    connect(ui_->actionImportSelectedSequences, SIGNAL(triggered()), SLOT(importSelectedBlastHits()));
}

BlastViewTab::~BlastViewTab()
{
    delete ui_;
}

void BlastViewTab::setBlastDatabaseWindow(BlastDatabaseWindow *blastDatabaseWindow)
{
    blastDatabaseWindow_ = blastDatabaseWindow;
}

void BlastViewTab::setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel)
{
    blastDatabaseModel_ = blastDatabaseModel;
}

void BlastViewTab::setBlastSequenceImporter(BlastSequenceImporter *blastSequenceImporter)
{
    blastSequenceImporter_ = blastSequenceImporter;
}

void BlastViewTab::setBlastReport(const BlastReportSPtr &blastReport)
{
    loadBlastReport(blastReport);
    blastReportModel_->setBlastReport(blastReport);
    ui_->blastReportTableView_->sortByColumn(BlastReportModel::eEvalueColumn, Qt::AscendingOrder);
}

void BlastViewTab::setDefaultImportIndex(const QModelIndex &index)
{
    defaultImportIndex_ = index;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void BlastViewTab::onBlastTableViewSelectionChanged(const QItemSelection &selected)
{
    ui_->actionImportSelectedSequences->setEnabled(blastDatabaseModel_ != nullptr &&
                                                   blastSequenceImporter_ != nullptr &&
                                                   selected.size() > 0);
}

void BlastViewTab::importSelectedBlastHits()
{
    ASSERT(blastReportModel_->blastReport() != nullptr);

    // Create the request
    BlastSequenceImporter::BlastImportRequest request;
    QModelIndex sourceBlastDatabase = blastDatabaseModel_->find(blastReportModel_->blastReport()->databaseSpec());
    if (!sourceBlastDatabase.isValid())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Import BLAST sequences error");
        msgBox.setText(QString("Unable to locate the exact source BLAST database that was used for this search (%1, "
                               "%2 sequences).Please check that it is listed in the BLAST Database Manager and try "
                               "again.")
                       .arg(blastReportModel_->blastReport()->databaseSpec().databaseName_)
                       .arg(blastReportModel_->blastReport()->databaseSpec().nSequences_));
        msgBox.setIcon(QMessageBox::Warning);
        if (blastDatabaseWindow_ != nullptr)
        {
            msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Ok);
            msgBox.button(QMessageBox::Open)->setText("Open BLAST Database Manager");
            msgBox.setDefaultButton(QMessageBox::Open);
        }
        else
        {
            msgBox.setStandardButtons(QMessageBox::Ok);
        }
        if (msgBox.exec() == QMessageBox::Open)
        {
            blastDatabaseWindow_->show();
            blastDatabaseWindow_->raise();
            blastDatabaseWindow_->activateWindow();
        }

        return;
    }
    request.blastDatabase_ = blastDatabaseModel_->fullDatabasePath(sourceBlastDatabase);
    request.blastIndices_ = ui_->blastReportTableView_->selectionModel()->selectedRows();
    request.destinationIndex_ = defaultImportIndex_;
    request.grammar_ = (blastReportModel_->blastReport()->databaseSpec().isProtein_) ? eAminoGrammar : eDnaGrammar;

    blastSequenceImporter_->import(request);
}

void BlastViewTab::onReset()
{
    ui_->actionImportSelectedSequences->setDisabled(true);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void BlastViewTab::loadBlastReport(const BlastReportSPtr &blastReport)
{
    if (blastReport == nullptr || blastReport->isLoaded())
        return;

    BlastXmlHandler blastXmlHandler(blastReport->query().grammar());
    QXmlSimpleReader reader;
    reader.setContentHandler(&blastXmlHandler);
    reader.setErrorHandler(&blastXmlHandler);

    QFile file(blastSequenceImporter_->adoc()->blastDataPath().filePath(blastReport->sourceFile()));
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this,
                             "BLAST file open error",
                             QString("Unable to open file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return;
    }

    QXmlInputSource xmlInputSource(&file);
    if (!reader.parse(&xmlInputSource))
    {
        QMessageBox::warning(this, "BLAST parse error", blastXmlHandler.errorString());
        return;
    }

    blastReport->setHits(blastXmlHandler.hits());
    blastReport->setLoaded(true);
}

