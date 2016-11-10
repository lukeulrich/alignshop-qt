/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTDATABASEDOWNLOADWINDOW_H
#define BLASTDATABASEDOWNLOADWINDOW_H

#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QTimer>
#include <QtCore/QVector>

#include <QtGui/QWidget>

#include "../../core/global.h"

namespace Ui {
    class BlastDatabaseDownloadWindow;
}

class QFileDialog;
class QSortFilterProxyModel;
class QThread;

class BlastDatabaseModel;
class NcbiBlastDatabaseInstaller;


/**
  * BlastDatabaseDownloadWindow handles downloading BLAST databases to the local filesystem.
  *
  * The user may choose an arbitrary directory to save the BLAST databases. Alternatively, if a blast database model has
  * been configured, one of its paths may be selected as well. The blast model list is dynamically updated as the blast
  * database model changes. For example, if the user has selected a model path and then it is removed before starting
  * the download, the items will be updated to contain a - Select directory - item at the beginning and the start
  * download button will be disabled. If the user chooses other and successfully selects another directory, it will
  * replace the - Select Directory - option.
  *
  * Currently, only NCBI pre-formatted BLAST databases are supported and these are manually specified in Qt Designer.
  * Decompression and inflation are performed on the fly as the relevant files are downloaded from NCBI. To avoid
  * degrading GUI performance, this process is performed in a separate thread.
  */
class BlastDatabaseDownloadWindow : public QWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit BlastDatabaseDownloadWindow(QWidget *parent = nullptr);    //!< Standard constructor
    ~BlastDatabaseDownloadWindow();                                     //!< Destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Sets the underlying blast database model to blastDatabaseModel
    void setBlastDatabaseModel(BlastDatabaseModel *blastDatabaseModel);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onBlastModelReset();
    void onBlastModelRowsInserted(const QModelIndex &parent, int start, int end);
    void onBlastModelRowsRemoved(const QModelIndex &parent, int start, int end);
    void onCancelDownloadButtonReleased();
    void onDatabaseTableCurrentCellChanged(int currentRow);
    void onDestinationComboBoxChanged(int index);
    void onDownloadCanceled();
    void onDownloadError(const QString &errorMessage);
    void onDownloadFinished();
    void onDownloadProgressChanged(qint64 done, qint64 total);
    void onStartButtonReleased();
    void updateElapsedTimeLabel();
    void showDownloadPage();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void clearBlastModelItems();        // Clears the blastModelPathIndices_ and removes these from the combobox
    void loadBlastModelItems();         // Reads the available paths from blast model and updates the combobox


    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::BlastDatabaseDownloadWindow *ui_;

    QDir destinationDirectory_;
    BlastDatabaseModel *blastDatabaseModel_;
    QVector<QPersistentModelIndex> blastModelPathIndices_;
    NcbiBlastDatabaseInstaller *blastDbInstaller_;
    QFileDialog *directoryDialog_;
    QTimer updateTimer_;
    QElapsedTimer elapsedTimer_;
    QThread *thread_;
    QReadWriteLock lock_;

    QTimer successTimer_;
};

#endif // BLASTDATABASEDOWNLOADWINDOW_H
