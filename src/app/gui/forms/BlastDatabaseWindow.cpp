/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "BlastDatabaseWindow.h"
#include "ui_BlastDatabaseWindow.h"
#include "BlastDatabaseDownloadWindow.h"
#include "../models/BlastDatabaseModel.h"
#include "../wizards/MakeBlastDatabaseWizard.h"
#include "../../core/constants.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
  * @param parent [QWidget *]
  */
BlastDatabaseWindow::BlastDatabaseWindow(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::BlastDatabaseWindow),
    blastDatabaseModel_(nullptr),
    directoryDialog_(nullptr),
    makeBlastDatabaseWizard_(nullptr)
{
    ui_->setupUi(this);

    connect(ui_->addButton, SIGNAL(released()), SLOT(onAddButtonReleased()));
    connect(ui_->removeButton, SIGNAL(released()), SLOT(onRemoveButtonReleased()));
    connect(ui_->downloadButton, SIGNAL(released()), SLOT(onDownloadButtonReleased()));
    connect(ui_->newButton, SIGNAL(released()), SLOT(onNewButtonReleased()));

    addAction(ui_->actionClose);
}

/**
  */
BlastDatabaseWindow::~BlastDatabaseWindow()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/**
  * @param blastDatabaseModel [BlastDatabaseModel *]
  */
void BlastDatabaseWindow::setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel)
{
    if (blastDatabaseModel_ != nullptr)
    {
        disconnect(blastDatabaseModel_, SIGNAL(modelReset()), this, SLOT(onBlastModelReset()));
        disconnect(blastDatabaseModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(onBlastModelRowsRemoved()));
        disconnect(blastDatabaseModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onBlastModelRowsInserted(QModelIndex)));
        disconnect(ui_->refreshButton, SIGNAL(released()), blastDatabaseModel_, SLOT(refresh()));
        disconnect(ui_->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTreeViewCurrentChanged(QModelIndex)));
    }

    blastDatabaseModel_ = blastDatabaseModel;
    ui_->treeView->setModel(blastDatabaseModel_);
    if (blastDatabaseModel_ != nullptr)
    {
        ui_->addButton->setEnabled(true);
        ui_->refreshButton->setEnabled(blastDatabaseModel_->rowCount());
        connect(blastDatabaseModel_, SIGNAL(modelReset()), SLOT(onBlastModelReset()));
        connect(blastDatabaseModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(onBlastModelRowsRemoved()));
        connect(blastDatabaseModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onBlastModelRowsInserted(QModelIndex)));
        connect(ui_->refreshButton, SIGNAL(released()), blastDatabaseModel_, SLOT(refresh()));
        connect(ui_->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onTreeViewCurrentChanged(QModelIndex)));

        QTimer::singleShot(0, this, SLOT(resizeTreeViewColumnsToContents()));
    }
    else
    {
        ui_->addButton->setEnabled(false);
        ui_->refreshButton->setEnabled(false);
    }

    // Pass this blast database model onto the download window if it has been initialized
    if (blastDatabaseDownloadWindow_)
        blastDatabaseDownloadWindow_->setBlastDatabaseModel(blastDatabaseModel);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void BlastDatabaseWindow::onAddButtonReleased()
{
    ASSERT(blastDatabaseModel_ != nullptr);

    // Fire up a directory selection dialog
    if (directoryDialog_ == nullptr)
    {
        directoryDialog_ = new QFileDialog(this);
        directoryDialog_->setOptions(QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails);
        directoryDialog_->setFileMode(QFileDialog::Directory);
    }

    if (directoryDialog_->exec())
    {
        // Use a try/catch block in case the BlastDatabaseFinder throws an error.
        try
        {
            blastDatabaseModel_->addBlastPath(directoryDialog_->selectedFiles().first());
        }
        catch (QString & /* error */)
        {
            // Show a generic error to the user since the real error is likely too technical to be helpful.
            QMessageBox::warning(this,
                                 "Unable to add path",
                                 QString("An unexpected error occurred while attempting to add the blast database "
                                         "path. Please check that the blastdbcmd program is located at: %1 and try "
                                         "again.").arg(constants::kBlastDbCmdRelativePath),
                                 QMessageBox::Ok);
        }
    }
}

/**
  */
void BlastDatabaseWindow::onBlastModelReset()
{
    ui_->removeButton->setEnabled(blastDatabaseModel_->rowCount());
    ui_->refreshButton->setEnabled(blastDatabaseModel_->rowCount());
}

/**
  * @param index [const QModelIndex &]
  */
void BlastDatabaseWindow::onBlastModelRowsInserted(const QModelIndex &index)
{
    ui_->removeButton->setEnabled(true);
    ui_->refreshButton->setEnabled(true);

    // Always show the tree path to index
    ui_->treeView->expand(index);
}

/**
  */
void BlastDatabaseWindow::onBlastModelRowsRemoved()
{
    if (blastDatabaseModel_->rowCount() == 0)
    {
        ui_->removeButton->setEnabled(false);
        ui_->refreshButton->setEnabled(false);
    }
}

/**
  */
void BlastDatabaseWindow::onDownloadButtonReleased()
{
    // Lazy load
    if (!blastDatabaseDownloadWindow_)
    {
        blastDatabaseDownloadWindow_.reset(new BlastDatabaseDownloadWindow);
        blastDatabaseDownloadWindow_->setBlastDatabaseModel(blastDatabaseModel_);
    }

    // If the window is already visible simply bring it to the top
    if (blastDatabaseDownloadWindow_->isVisible())
    {
        blastDatabaseDownloadWindow_->raise();
        blastDatabaseDownloadWindow_->activateWindow();
    }
    // Otherwise, unhide it and position it horizontally centered and with the header just below this window
    else
    {
        blastDatabaseDownloadWindow_->show();

        QRect thisGeom = geometry();
        QRect thatGeom = blastDatabaseDownloadWindow_->geometry();
        thatGeom.moveLeft(thisGeom.left() + (thisGeom.width() - thatGeom.width()) / 2);
        thatGeom.moveTop(thisGeom.top() + 50);

        blastDatabaseDownloadWindow_->setGeometry(thatGeom);
    }
}

/**
  */
void BlastDatabaseWindow::onNewButtonReleased()
{
    if (makeBlastDatabaseWizard_ == nullptr)
        makeBlastDatabaseWizard_ = new MakeBlastDatabaseWizard(blastDatabaseModel_, this);

    if (makeBlastDatabaseWizard_->isHidden())
        makeBlastDatabaseWizard_->restart();

    makeBlastDatabaseWizard_->show();
    makeBlastDatabaseWizard_->raise();
    makeBlastDatabaseWizard_->activateWindow();
}

/**
  */
void BlastDatabaseWindow::onRemoveButtonReleased()
{
    ASSERT(blastDatabaseModel_ != nullptr);

    QModelIndex index = ui_->treeView->currentIndex();
    if (!index.isValid())
        return;

    ASSERT(blastDatabaseModel_->isTopLevelPath(index));

    blastDatabaseModel_->removeRows(index.row(), 1, index.parent());
}

/**
  * @param currentIndex [const QModelIndex &]
  */
void BlastDatabaseWindow::onTreeViewCurrentChanged(const QModelIndex &currentIndex)
{
    ASSERT(blastDatabaseModel_ != nullptr);

    // May only remove top level paths
    ui_->removeButton->setEnabled(blastDatabaseModel_->isTopLevelPath(currentIndex));
}

void BlastDatabaseWindow::resizeTreeViewColumnsToContents()
{
    ui_->treeView->resizeColumnToContents(BlastDatabaseModel::ePathColumn);
    ui_->treeView->setColumnWidth(BlastDatabaseModel::eNoteColumn, 150);
}
