/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASEWINDOW_H
#define BLASTDATABASEWINDOW_H

#include <QtCore/QScopedPointer>

#include <QtGui/QWidget>
#include "../../core/global.h"

namespace Ui {
    class BlastDatabaseWindow;
}

class QFileDialog;
class QModelIndex;

class BlastDatabaseModel;
class BlastDatabaseDownloadWindow;
class MakeBlastDatabaseWizard;

/**
  * BlastDatabaseWindow exposes locally available BLAST databases stored in a set of user-defined directories.
  *
  */
class BlastDatabaseWindow : public QWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit BlastDatabaseWindow(QWidget *parent = nullptr);        //!< Standard constructor
    ~BlastDatabaseWindow();                                         //!< Destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Sets the underlying blast database model to blastDatabaseModel
    void setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onAddButtonReleased();                                     //!< Ask user for directory to add to the set of paths
    void onBlastModelReset();                                       //!< Enable/disable buttons according to blast model paths
    void onBlastModelRowsInserted(const QModelIndex &index);        //!< Enable/disable buttons according to blast model insertion
    void onBlastModelRowsRemoved();                                 //!< Enable/disable buttons according to remaining blast model paths
    void onDownloadButtonReleased();                                //!< Open the download window
    void onNewButtonReleased();                                     //!< Open the make blast database wizard
    void onRemoveButtonReleased();                                  //!< Remove the currently selected node if top level path
    void onTreeViewCurrentChanged(const QModelIndex &currentIndex); //!< Enable/disable remove button if top level path is selected
    void resizeTreeViewColumnsToContents();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::BlastDatabaseWindow *ui_;
    BlastDatabaseModel *blastDatabaseModel_;            //!< Underlying model powering the treeview
    QFileDialog *directoryDialog_;                      //!< Dialog for selecting a directory
    //! Lazy-loaded instance of the download window
    QScopedPointer<BlastDatabaseDownloadWindow> blastDatabaseDownloadWindow_;
    MakeBlastDatabaseWizard *makeBlastDatabaseWizard_;  //!< Lazy-loaded instance of the BLAST database format wizard
};

#endif // BLASTDATABASEWINDOW_H
