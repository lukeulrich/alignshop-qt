/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QEventLoop>
#include <QtCore/QThread>
#include <QtCore/QWriteLocker>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTableWidgetItem>

#include <QtDebug>

#include "BlastDatabaseDownloadWindow.h"
#include "ui_BlastDatabaseDownloadWindow.h"
#include "../models/BlastDatabaseModel.h"
#include "../../core/Services/NcbiBlastDatabaseInstaller.h"
#include "../../core/constants.h"
#include "../../core/misc.h"

//! Text for first item in the combo box
static const QString kSelectDirectoryText = "- Select directory -";


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QWidget *]
  */
BlastDatabaseDownloadWindow::BlastDatabaseDownloadWindow(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::BlastDatabaseDownloadWindow),
    blastDatabaseModel_(nullptr),
    blastDbInstaller_(nullptr),
    directoryDialog_(nullptr),
    thread_(nullptr)
{
    // --------------
    // Core UI setup
    ui_->setupUi(this);


    // --------------------------------------------
    // Initialize the NCBI blast database installer
    blastDbInstaller_ = new NcbiBlastDatabaseInstaller("NCBI");
    blastDbInstaller_->setNcbiBlastDbUrl(constants::kNcbiBlastDbFtpUrl_);
    blastDbInstaller_->setPrefix(ui_->databaseTableWidget->item(0, 0)->text());
    connect(blastDbInstaller_, SIGNAL(actionTextChanged(QString)), ui_->actionTextLabel, SLOT(setText(QString)));
    connect(blastDbInstaller_, SIGNAL(progressChanged(qint64,qint64)), SLOT(onDownloadProgressChanged(qint64,qint64)));
    connect(blastDbInstaller_, SIGNAL(finished()), SLOT(onDownloadFinished()));
    connect(blastDbInstaller_, SIGNAL(canceled()), SLOT(onDownloadCanceled()));
    connect(blastDbInstaller_, SIGNAL(error(QString)), SLOT(onDownloadError(QString)));

    // Move the installer to a separate thread
    thread_ = new QThread;
    blastDbInstaller_->moveToThread(thread_);
    thread_->start(QThread::LowPriority);


    // -------------------
    // UI-specific details

    // o Page 1
    ui_->stackedWidget->setCurrentIndex(0);
    ui_->databaseTableWidget->sortItems(0);
    connect(ui_->databaseTableWidget, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(onDatabaseTableCurrentCellChanged(int)));
    ui_->databaseTableWidget->setCurrentCell(0, 0);     // Make sure at least one cell is selected

    // Setup the destination combobox
    ui_->destinationComboBox->addItem(kSelectDirectoryText);
    ui_->destinationComboBox->addItem("Other...");
    ui_->destinationComboBox->setItemData(1, "Opens a dialog for selecting a directory", Qt::ToolTipRole);
    connect(ui_->destinationComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onDestinationComboBoxChanged(int)));

    // Start download
    connect(ui_->startButton, SIGNAL(released()), SLOT(onStartButtonReleased()));

    // o Page 2
    updateTimer_.setInterval(1000);
    connect(&updateTimer_, SIGNAL(timeout()), SLOT(updateElapsedTimeLabel()));

    connect(ui_->cancelDownloadButton, SIGNAL(released()), SLOT(onCancelDownloadButtonReleased()));

    ui_->successButton->hide();
    connect(ui_->successButton, SIGNAL(released()), SLOT(showDownloadPage()));
    successTimer_.setSingleShot(true);
    successTimer_.setInterval(2500);
    connect(&successTimer_, SIGNAL(timeout()), SLOT(showDownloadPage()));
}

/**
  * Attempts to gracefully cancel any outstanding download and kills the ftp request if the cancel has not completed
  * within 5 seconds.
  *
  * Appropriately de-allocates the thread instance.
  */
BlastDatabaseDownloadWindow::~BlastDatabaseDownloadWindow()
{
    // Shutdown the main thread - waiting first for any cancel request to finish
    if (blastDbInstaller_->isActive())
    {
        QEventLoop eventLoop;
        connect(blastDbInstaller_, SIGNAL(canceled()), &eventLoop, SLOT(quit()));
        connect(blastDbInstaller_, SIGNAL(finished()), &eventLoop, SLOT(quit()));

        QTimer timer;
        timer.setInterval(5000);
        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
        timer.start();

        // ISSUE: No mutex or write locker?
        blastDbInstaller_->cancel();

        eventLoop.exec();
    }

    ASSERT(thread_->isRunning());
    thread_->quit();
    thread_->wait(5000);
    if (thread_->isRunning())
    {
        thread_->terminate();
        thread_->wait();
    }
    delete thread_;
    delete blastDbInstaller_;

    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param blastDatabaseModel [BlastDatabaseModel *]
  */
void BlastDatabaseDownloadWindow::setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel)
{
    if (blastDatabaseModel_ != nullptr)
    {
        disconnect(blastDatabaseModel_, SIGNAL(modelReset()), this, SLOT(onBlastModelReset()));
        disconnect(blastDatabaseModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(onBlastModelRowsRemoved(QModelIndex,int,int)));
        disconnect(blastDatabaseModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onBlastModelRowsInserted(QModelIndex,int,int)));

        clearBlastModelItems();
    }

    blastDatabaseModel_ = blastDatabaseModel;
    if (blastDatabaseModel_ != nullptr)
    {
        connect(blastDatabaseModel_, SIGNAL(modelReset()), SLOT(onBlastModelReset()));
        connect(blastDatabaseModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onBlastModelRowsRemoved(QModelIndex,int,int)));
        connect(blastDatabaseModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onBlastModelRowsInserted(QModelIndex,int,int)));
        loadBlastModelItems();
    }
    else
    {
        ui_->startButton->setEnabled(false);
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void BlastDatabaseDownloadWindow::onBlastModelReset()
{
    clearBlastModelItems();
    loadBlastModelItems();
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void BlastDatabaseDownloadWindow::onBlastModelRowsInserted(const QModelIndex &parent, int start, int end)
{
    // If parent is valid this indicates that the node is not a top-level path node. In other words, it does not
    // correspond to a local directory.
    if (parent.isValid())
        return;

    // If there are just 2 items ("- Select directory -" and "Other..."), this indicates that the blast database model
    // was previously empty. Now that at least one row has been inserted, add a separator bar.
    if (ui_->destinationComboBox->count() == 2)
        ui_->destinationComboBox->insertSeparator(2);

    // For each new top-level path, add it to the combobox list of available destination paths
    for (int i=start; i<= end; ++i)
    {
        QModelIndex index = blastDatabaseModel_->index(i, BlastDatabaseModel::ePathColumn);
        blastModelPathIndices_ << index;
        // Only show its directory name
        ui_->destinationComboBox->addItem(QDir(index.data().toString()).dirName());
        // But provide the full path in the tool tip
        ui_->destinationComboBox->setItemData(ui_->destinationComboBox->count() - 1, index.data().toString(), Qt::ToolTipRole);
    }
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void BlastDatabaseDownloadWindow::onBlastModelRowsRemoved(const QModelIndex &parent, int start, int end)
{
    // If parent is valid this indicates that the node is not a top-level path node. In other words, it does not
    // correspond to a local directory.
    if (parent.isValid())
        return;

    // The fact that a row has been removed indicates that there was at least one path present, thus there should be a
    // minimum of four items in the destination combo box:
    // 1) "- Select directory -" or user-defined path
    // 2) Other...
    // 3) Separator
    // 4+) Blast model paths
    ASSERT(ui_->destinationComboBox->count() > 3);

    for (int i=start; i<= end; ++i)
    {
        QModelIndex index = blastDatabaseModel_->index(i, BlastDatabaseModel::ePathColumn);
        for (int j=0, z=blastModelPathIndices_.size(); j<z; ++j)
        {
            if (index != blastModelPathIndices_.at(j))
                continue;

            // Remove from the blastModelPathIndices vector
            blastModelPathIndices_.remove(j, 1);

            // +3 = 1 for first item in combobox + 1 for Other... + 1 for the separator
            if (j + 3 == ui_->destinationComboBox->currentIndex())
            {
                // Select the first index and reset it to the default state
                destinationDirectory_ = QDir();
                ui_->destinationComboBox->setItemText(0, kSelectDirectoryText);
                ui_->destinationComboBox->setItemData(0, QString(), Qt::ToolTipRole);
                ui_->destinationComboBox->setCurrentIndex(0);
            }

            ui_->destinationComboBox->removeItem(2 + j);
            break;
        }
    }

    // Remove the separator if there are no more paths in the model
    if (blastDatabaseModel_->rowCount() == 0)
        ui_->destinationComboBox->removeItem(2);
}

/**
  */
void BlastDatabaseDownloadWindow::onCancelDownloadButtonReleased()
{
    ui_->cancelDownloadButton->setEnabled(false);
    QMetaObject::invokeMethod(blastDbInstaller_, "cancel", Qt::QueuedConnection);
    updateTimer_.stop();
}

/**
  * Updates the description label and the database prefix used by the blast database installer.
  *
  * @param currentRow [int]
  */
void BlastDatabaseDownloadWindow::onDatabaseTableCurrentCellChanged(int currentRow)
{
    QTableWidgetItem *descriptionItem = ui_->databaseTableWidget->item(currentRow, 1);
    ui_->databaseDescriptionLabel->setText(descriptionItem->text());

    // Update the prefix for the blast database installer. It should not be possible to change this when downloading
    // because this control will be hidden.
    ASSERT(ui_->stackedWidget->currentIndex() == 0);
    ASSERT(blastDbInstaller_->isActive() == false);
    QTableWidgetItem *nameItem = ui_->databaseTableWidget->item(ui_->databaseTableWidget->currentRow(), 0);
    blastDbInstaller_->setPrefix(nameItem->text());
}

/**
  * @param index [int]
  */
void BlastDatabaseDownloadWindow::onDestinationComboBoxChanged(int index)
{
    // The second index must always be a separator and thus never selectable
    ASSERT(index != 2);

    // The second index in the combox box always refers to the Other... item
    if (index == 1)
    {
        // Lazy load the directory dialog
        if (directoryDialog_ == nullptr)
        {
            directoryDialog_ = new QFileDialog(this);
            directoryDialog_->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails);
            directoryDialog_->setFileMode(QFileDialog::Directory);
        }

        // Request directory from user initialized to the current directory contained by the blast database installer
        directoryDialog_->setDirectory(blastDbInstaller_->destinationDirectory());
        if (directoryDialog_->exec())
        {
            // The user successfully chose a directory, update the first text item to refer to this one
            destinationDirectory_ = directoryDialog_->selectedFiles().first();
            ui_->destinationComboBox->setItemText(0, destinationDirectory_.dirName());
        }

        // By moving this to the first index, it will call this method again with an index value of zero.
        ui_->destinationComboBox->setCurrentIndex(0);
        return;
    }
    // An index greater than 2 indicates a path from the blast database model
    else if (index > 2)
    {
        // Get path from the database model
        ASSERT(blastDatabaseModel_ != nullptr);

        // Subtract three to account for the first item, "Other...", and the separator
        QModelIndex modelIndex = blastModelPathIndices_.at(index - 3);
        ASSERT(modelIndex.isValid());

        QString path = modelIndex.data(BlastDatabaseModel::ePathColumn).toString();
        destinationDirectory_.setPath(path);
    }

    // Now that we have a directory, attempt to configure the blast database installer with this directory, but only if
    // it is not "- Select directory -" (ie. kSelectDirectoryText)
    bool success = false;
    if (ui_->destinationComboBox->currentText() != kSelectDirectoryText)
    {
        // Because the blast database installer lives in another thread, first obtain a write locker instance to update
        // the destination directory. Technically, should never be a problem because setDestinationDirectory will only
        // fail if the installer is active or the directory does not exist. If the installer is active, then it should
        // not be possible via the GUI to change the current item in the combobox. We could check the latter case before
        // calling setDestinationDirectory, yet this duplicates code unnecessarily. Nonetheless, play it safe and obtain
        // the proper lock.
        {
            QWriteLocker locker(&lock_);
            Q_UNUSED(locker);       // To prevent unused warning
            success = blastDbInstaller_->setDestinationDirectory(destinationDirectory_);
        }
        if (!success)
        {
            // Something went wrong - tell the user about it
            destinationDirectory_ = QDir();
            ui_->destinationComboBox->setItemText(0, kSelectDirectoryText);
            ui_->destinationComboBox->setItemData(0, QString(), Qt::ToolTipRole);
            QMessageBox::warning(this,
                                 "Unable to select directory",
                                 QString("An unexpected error has occurred. Please try again."),
                                 QMessageBox::Ok);
        }
    }
    ui_->startButton->setEnabled(success);
}

/**
  * Called from the blast database installer after a cancel request has successfully completed. Any requested files are
  * removed in the cleanUp() method.
  *
  * Restore the original geometry height.
  */
void BlastDatabaseDownloadWindow::onDownloadCanceled()
{
    updateTimer_.stop();

    QWriteLocker writeLocker(&lock_);
    Q_UNUSED(writeLocker);
    blastDbInstaller_->cleanUp();
    blastDbInstaller_->reset();

    QRect geom = geometry();
    geom.setHeight(400);
    setGeometry(geom);

    ui_->stackedWidget->setCurrentIndex(0);
}

/**
  */
void BlastDatabaseDownloadWindow::onDownloadError(const QString &errorMessage)
{
    qDebug() << Q_FUNC_INFO << errorMessage;

    // The installer has encountered and error, clean it up by borrowing the onDownloadCanceled method.
    onDownloadCanceled();
}

/**
  * The download and install has completed successfully. Show the success button and start the timer to close the dialog
  * automatically if the user has not closed it already in the time allotted.
  */
void BlastDatabaseDownloadWindow::onDownloadFinished()
{
    ui_->cancelDownloadButton->setEnabled(false);
    ui_->successButton->show();

    // Stop updating the time elapsed label
    updateTimer_.stop();

    successTimer_.start();
}

/**
  * @param done [qint64]
  * @param total [qint64]
  */
void BlastDatabaseDownloadWindow::onDownloadProgressChanged(qint64 done, qint64 total)
{
    ui_->downloadProgressBar->setValue((100. * done) / total);
}

/**
  */
void BlastDatabaseDownloadWindow::onStartButtonReleased()
{
    ui_->cancelDownloadButton->setEnabled(true);
    ui_->successButton->hide();

    // Update the labels
    QTableWidgetItem *nameItem = ui_->databaseTableWidget->item(ui_->databaseTableWidget->currentRow(), 0);
    ui_->activeDbLabel->setText(nameItem->text());
    ui_->activeDestinationLabel->setText(blastDbInstaller_->destinationDirectory().path());
    ui_->timeElapsedLabel->setText("-");
    ui_->downloadProgressBar->setValue(0);

    ui_->stackedWidget->setCurrentIndex(1);
    QMetaObject::invokeMethod(blastDbInstaller_, "start", Qt::QueuedConnection);
    elapsedTimer_.start();
    updateTimer_.start();

    // Make it look more aesthetically pleasing.
    QRect geom = geometry();
    geom.setHeight(250);
    setGeometry(geom);
}

/**
  */
void BlastDatabaseDownloadWindow::updateElapsedTimeLabel()
{
    ui_->timeElapsedLabel->setText(::formatTimeRunning(elapsedTimer_.elapsed() / 1000));
}

/**
  */
void BlastDatabaseDownloadWindow::showDownloadPage()
{
    // In case the user got here from pressing the success button and our timer is still running.
    successTimer_.stop();

    QRect geom = geometry();
    geom.setHeight(400);
    setGeometry(geom);

    ui_->stackedWidget->setCurrentIndex(0);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  */
void BlastDatabaseDownloadWindow::clearBlastModelItems()
{
    blastModelPathIndices_.clear();
    if (blastDatabaseModel_ != nullptr && blastDatabaseModel_->rowCount() == 0)
    {
        if (ui_->destinationComboBox->count() > 2)
            for (int i=2, z=ui_->destinationComboBox->count(); i<z; ++i)
                ui_->destinationComboBox->removeItem(i);

        ui_->destinationComboBox->setCurrentIndex(0);
    }

    ui_->startButton->setEnabled(false);
}

/**
  */
void BlastDatabaseDownloadWindow::loadBlastModelItems()
{
    if (blastDatabaseModel_ == nullptr)
        return;

    // There is at least one item left in the blast model
    if (blastDatabaseModel_->rowCount() == 0)
        return;

    ui_->destinationComboBox->insertSeparator(2);
    for (int i=0, z=blastDatabaseModel_->rowCount(); i<z; ++i)
    {
        QModelIndex index = blastDatabaseModel_->index(i, BlastDatabaseModel::ePathColumn);
        blastModelPathIndices_ << index;
        ui_->destinationComboBox->addItem(QDir(index.data().toString()).dirName());
        ui_->destinationComboBox->setItemData(ui_->destinationComboBox->count() - 1, index.data().toString(), Qt::ToolTipRole);
    }
}
