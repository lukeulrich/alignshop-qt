/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTimer>

#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFileSystemModel>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QRegExpValidator>
#include <QtGui/QSizePolicy>
#include <QtGui/QSpacerItem>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QTreeView>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include "MakeBlastDatabaseWizard.h"
#include "../models/BlastDatabaseModel.h"
#include "../../core/Services/MakeBlastDatabaseWrapper.h"
#include "../../core/constants/MakeBlastDbConstants.h"
#include "../../core/util/OptionSet.h"
#include "../../core/PODs/Option.h"
#include "../../core/constants.h"
#include "../../core/global.h"
#include "../../core/macros.h"

class ColumnFilterProxyModel : public QSortFilterProxyModel
{
public:
    ColumnFilterProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent),
          showAll_(true)
    {
    }

    void show(int column)
    {
        if (accept_.contains(column))
            return;

        accept_ << column;

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }

    void hide(int column)
    {
        if (!accept_.contains(column))
            return;

        accept_.remove(column);
        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }

    void setShowAll(bool showAll)
    {
        if (showAll_ == showAll)
            return;

        emit layoutAboutToBeChanged();
        showAll_ = showAll;
        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }

    void hideAll()
    {
        if (showAll_ || accept_.size())
        {
            showAll_ = false;
            accept_.clear();
            emit layoutAboutToBeChanged();
            emit layoutChanged();
        }
    }

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex & /* source_parent */) const
    {
        if (showAll_)
            return true;

        if (accept_.contains(source_column))
            return true;

        return false;
    }

private:
    bool showAll_;
    QSet<int> accept_;
};

/**
  * @param blastDatabaseModel [BlastDatabaseModel *]
  * @param parent [QWidget *]
  */
MakeBlastDatabaseWizard::MakeBlastDatabaseWizard(BlastDatabaseModel *blastDatabaseModel, QWidget *parent) :
    QWizard(parent)
{
    setWindowTitle("New BLAST Database Wizard");

    addPage(new SelectFilePage);
    addPage(new DatabaseConfigurationPage);

    formatDatabasePage_ = new FormatDatabasePage(blastDatabaseModel);
    addPage(formatDatabasePage_);
}

/**
  */
void MakeBlastDatabaseWizard::reject()
{
    if (formatDatabasePage_->isActive())
    {
        QMessageBox msgBox(formatDatabasePage_);
        msgBox.setWindowTitle("Confirm Cancel");
        msgBox.setText("The BLAST database has not yet finished being formatted. Are you sure you want to cancel?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.button(QMessageBox::Yes)->setText("Yes, Cancel");
        msgBox.button(QMessageBox::No)->setText("Continue Format");
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() == QMessageBox::No)
            return;

        // Stop the formatting
        formatDatabasePage_->cancelFormat();
    }

    QWizard::reject();
}


SelectFilePage::SelectFilePage(QWidget *parent)
    : QWizardPage(parent),
      complete_(false)
{
    setTitle("Select Input File");
    setSubTitle("Using the file explorer below, select a FASTA-formatted sequence file to format "
                "into a BLAST database.");

    fileSystemModel_ = new QFileSystemModel(this);
    fileSystemModel_->setRootPath("/");

    proxyModel_ = new ColumnFilterProxyModel(this);
    proxyModel_->hideAll();
    proxyModel_->show(0);       // The name column
    proxyModel_->setSourceModel(fileSystemModel_);

    treeView_ = new QTreeView;
    treeView_->setModel(proxyModel_);
    treeView_->setSelectionBehavior(QAbstractItemView::SelectItems);
    connect(treeView_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex)));
    connect(treeView_, SIGNAL(doubleClicked(QModelIndex)), SLOT(onIndexDoubleClicked(QModelIndex)));

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(treeView_);
    setLayout(verticalLayout);

    // Use custom property defined on this class
    registerField("inputFile", this, "inputFile", SIGNAL(inputFileChanged()));

    QModelIndex index = proxyModel_->mapFromSource(fileSystemModel_->index(QDir::currentPath()));
//    QDir currentDir(QDir::currentPath());
//    QStringList readableFiles = currentDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
//    QModelIndex index = (readableFiles.size() > 0) ? fileSystemModel_->index(currentDir.filePath(readableFiles.first()))
//                                                   : fileSystemModel_->index(currentDir.currentPath());
    treeView_->setCurrentIndex(index);
    treeView_->expand(index);
}

/**
  */
SelectFilePage::~SelectFilePage()
{
}

/**
  * @returns bool
  */
bool SelectFilePage::isComplete() const
{
    return complete_;
}

/**
  * @returns QString
  */
QString SelectFilePage::inputFile() const
{
    return inputFile_;
}

/**
  */
void SelectFilePage::setInputFile(const QString &inputFile)
{
    inputFile_ = inputFile;
    emit inputFileChanged();
}

void SelectFilePage::initializePage()
{
    // Trigger a row change and thus update the inputFile field. Mostly relevant for when this wizard is re-opened a
    // second time.
    onCurrentRowChanged(treeView_->selectionModel()->selectedRows().first());

    // Add back in the cancel button (in case it was removed during the last invocation)
    wizard()->setOption(QWizard::NoCancelButton, false);

    QTimer::singleShot(50, this, SLOT(scrollToSelectedItem()));
}

void SelectFilePage::scrollToSelectedItem()
{
    treeView_->scrollTo(treeView_->selectionModel()->selectedRows().first(), QAbstractItemView::PositionAtCenter);
}

/**
  * @param currentIndex [const QModelIndex &]
  */
void SelectFilePage::onCurrentRowChanged(const QModelIndex &currentIndex)
{
    ASSERT(currentIndex.model() == proxyModel_);
    QModelIndex fileIndex = proxyModel_->mapToSource(currentIndex);
    setComplete(!fileSystemModel_->isDir(fileIndex));

    setInputFile(fileSystemModel_->filePath(fileIndex));
}

/**
  * @param index [const QModelIndex &]
  */
void SelectFilePage::onIndexDoubleClicked(const QModelIndex & /* index */)
{
    // Attempt to move to the next page in the wizard
    wizard()->button(QWizard::NextButton)->click();
}

/**
  * @param newComplete [bool]
  */
void SelectFilePage::setComplete(bool newComplete)
{
    if (newComplete != complete_)
    {
        complete_ = newComplete;
        emit completeChanged();
    }
}


DatabaseConfigurationPage::DatabaseConfigurationPage(QWidget *parent)
    : QWizardPage(parent)
{
    inputFileLabel_ = new QLabel("Sequence File:");
    actualInputFileLabel_ = new QLabel("Put real file name here!");
    outputDirectoryLabel_ = new QLabel("Output Directory:");
    databaseNameLabel_ = new QLabel("Database Name:");
    titleLabel_ = new QLabel("Title:");
    typeLabel_ = new QLabel("Type:");

    outputDirectoryLineEdit_ = new QLineEdit;
    databaseNameLineEdit_ = new QLineEdit;
    outputDirectoryPushButton_ = new QPushButton("...");
    titleLineEdit_ = new QLineEdit;
    proteinRadioButton_ = new QRadioButton("Protein");
    nucleotideRadioButton_ = new QRadioButton("Nucleotide");
    parseSeqIdsCheckBox_ = new QCheckBox("Parse sequence ids (recommended)");
    addPathCheckBox_ = new QCheckBox("Add output path to BLAST Database manager");

    QSizePolicy sizePolicy = outputDirectoryLineEdit_->sizePolicy();
    sizePolicy.setHorizontalStretch(1);
    outputDirectoryLineEdit_->setSizePolicy(sizePolicy);
    databaseNameLineEdit_->setPlaceholderText("(Optional)");
    titleLineEdit_->setPlaceholderText("(Optional)");
    proteinRadioButton_->setChecked(true);
    parseSeqIdsCheckBox_->setChecked(true);
    addPathCheckBox_->setChecked(true);

    // Prevent directory separators from being displayed on the output file name
    QRegExpValidator *validator = new QRegExpValidator(QRegExp(QString("^[A-Za-z0-9][^%1]*$").arg(QDir::separator())), this);
    databaseNameLineEdit_->setValidator(validator);


    QGridLayout *gridLayout = new QGridLayout;
    setLayout(gridLayout);
    gridLayout->setColumnStretch(1, 1);

    gridLayout->addWidget(inputFileLabel_, 0, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(actualInputFileLabel_, 0, 1, 1, 2);

    gridLayout->addWidget(outputDirectoryLabel_, 1, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(outputDirectoryLineEdit_, 1, 1);
    gridLayout->addWidget(outputDirectoryPushButton_, 1, 2);

    gridLayout->addWidget(databaseNameLabel_, 2, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(databaseNameLineEdit_, 2, 1);

    gridLayout->addWidget(titleLabel_, 3, 0, 1, 1, Qt::AlignRight);
    gridLayout->addWidget(titleLineEdit_, 3, 1);

    gridLayout->addWidget(typeLabel_, 4, 0, 1, 1, Qt::AlignRight);
    QHBoxLayout *horizLayout = new QHBoxLayout;
    horizLayout->addWidget(proteinRadioButton_);
    horizLayout->addWidget(nucleotideRadioButton_);
    horizLayout->insertStretch(2, 1);
    gridLayout->addLayout(horizLayout, 4, 1, 1, 2);

    gridLayout->addWidget(parseSeqIdsCheckBox_, 5, 1);
    gridLayout->addWidget(addPathCheckBox_, 6, 1);

    setTitle("Database parameters");
    setSubTitle("Specify a database title, type, and other options.");

    registerField("outputDirectory*", outputDirectoryLineEdit_);
    registerField("outputFile", databaseNameLineEdit_);
    registerField("title", titleLineEdit_);
    registerField("moleculeType", proteinRadioButton_);
    registerField("parseSeqIds", parseSeqIdsCheckBox_);
    registerField("addPathToBlastManager", addPathCheckBox_);

    connect(outputDirectoryPushButton_, SIGNAL(released()), SLOT(onOutputDirectoryButtonReleased()));
}

void DatabaseConfigurationPage::initializePage()
{
    QString inputFile = field("inputFile").toString();
    actualInputFileLabel_->setText(inputFile);

    QFileInfo fileInfo(inputFile);
    outputDirectoryLineEdit_->setText(fileInfo.canonicalPath());
    databaseNameLineEdit_->setText(fileInfo.fileName());
}

/**
  * @returns bool
  */
bool DatabaseConfigurationPage::validatePage()
{
    // Check that the output directory exists and if it does not, ask user if they want to create it.
    ASSERT(!outputDirectoryLineEdit_->text().isEmpty());
    QDir outDirectory(outputDirectoryLineEdit_->text());
    if (outDirectory.exists())
        return true;

    // Output directory does not exist. Attempt to create it
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Create directory?");
        msgBox.setText(QString("The output directory, %1, does not exist. Would you like to create it?").arg(outDirectory.absolutePath()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.button(QMessageBox::Yes)->setText("Create Directory");
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Cancel)
            return false;
    }

    // Attempt to create the directory
    if (!outDirectory.mkpath(outDirectory.absolutePath()))
    {
        QMessageBox::warning(this,
                             "Error creating directory",
                             QString("The directory, %1, could not be created. Please select another output "
                                     "directory and try again.").arg(outDirectory.absolutePath()),
                             QMessageBox::Ok);
        return false;
    }

    return true;
}

/**
  */
void DatabaseConfigurationPage::onOutputDirectoryButtonReleased()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Select Output Directory",
                                                    outputDirectoryLineEdit_->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::HideNameFilterDetails);
    if (dir.isEmpty())
        return;

    outputDirectoryLineEdit_->setText(dir);
}


/**
  * @param blastDatabaseModel [BlastDatabaseModel *]
  * @param parent [QWidget *]
  */
FormatDatabasePage::FormatDatabasePage(BlastDatabaseModel *blastDatabaseModel, QWidget *parent)
    : QWizardPage(parent),
      blastDatabaseModel_(blastDatabaseModel),
      makeBlastDatabaseWrapper_(nullptr),
      complete_(false)
{
    setTitle("Formatting BLAST Database");
    setSubTitle("Depending on the size of the input sequence file this may take several minutes to complete. "
                "You may stop the format process by pressing the Cancel Format button.");

    formatLabel_ = new QLabel("Formatted:");
    actualFormatLabel_ = new QLabel;
    cancelButton_ = new QPushButton("Cancel Format");
    progressBar_ = new QProgressBar;
    logLabel_ = new QLabel("Log:");
    formatLogTextEdit_ = new QPlainTextEdit;

    progressBar_->setTextVisible(false);
    formatLogTextEdit_->setReadOnly(true);

    QHBoxLayout *horizLayout = new QHBoxLayout;
    horizLayout->addWidget(formatLabel_);
    horizLayout->setAlignment(formatLabel_, Qt::AlignVCenter);
    horizLayout->addWidget(actualFormatLabel_);
    horizLayout->insertStretch(2, 1);
    horizLayout->addWidget(cancelButton_);

    QVBoxLayout *vertLayout = new QVBoxLayout;
    vertLayout->addLayout(horizLayout);
    vertLayout->addWidget(progressBar_);
    vertLayout->insertSpacing(2, 10);
    vertLayout->addWidget(logLabel_);
    vertLayout->addWidget(formatLogTextEdit_);

    setLayout(vertLayout);

    connect(cancelButton_, SIGNAL(released()), SLOT(cancelFormat()));
}

/**
  * @returns bool
  */
bool FormatDatabasePage::isActive() const
{
    return makeBlastDatabaseWrapper_ != nullptr &&
           makeBlastDatabaseWrapper_->isRunning();
}

/**
  * @returns bool
  */
bool FormatDatabasePage::isComplete() const
{
    return complete_;
}

/**
  */
void FormatDatabasePage::cancelFormat()
{
    ASSERT(makeBlastDatabaseWrapper_ != nullptr);
    if (!makeBlastDatabaseWrapper_->isRunning())
        return;

    makeBlastDatabaseWrapper_->kill("Canceled format. To choose another sequence file to format, press the Back "
                                    "button.");

    cancelButton_->setEnabled(false);

    wizard()->button(QWizard::FinishButton)->setEnabled(false);
}

/**
  */
void FormatDatabasePage::initializePage()
{
    complete_ = false;
    actualFormatLabel_->setText("-");
    cancelButton_->setEnabled(true);
    progressBar_->setRange(0, 0);               // Simply show a busy progress bar
    formatLogTextEdit_->clear();
    wizard()->setOption(QWizard::NoBackButtonOnLastPage, false);        // For the second time round

    // Lazy creation
    if (makeBlastDatabaseWrapper_ == nullptr)
    {
        makeBlastDatabaseWrapper_ = new MakeBlastDatabaseWrapper(this);
        makeBlastDatabaseWrapper_->setFastaTick(1000);
        makeBlastDatabaseWrapper_->setProgram(qApp->applicationDirPath() + QDir::separator() + constants::kMakeBlastDbRelativePath);
        connect(makeBlastDatabaseWrapper_, SIGNAL(progressChanged(int)), SLOT(onMakeBlastDatabaseProgressChanged(int)));
        connect(makeBlastDatabaseWrapper_, SIGNAL(formatFininshed(QString)), SLOT(onFormatFinished(QString)));
        connect(makeBlastDatabaseWrapper_, SIGNAL(error(int,QString)), SLOT(onMakeBlastDatabaseError(int,QString)));
    }

    QString inputFile = field("inputFile").toString();
    ASSERT(inputFile.isEmpty() == false);

    OptionSet options = getOptions();
    // Configure a log file
    QTemporaryFile tempFile(QDir::tempPath() + QDir::separator() + "makeblastdb.log.XXXXXX");
    tempFile.open();
    tempFile.close();
    tempFile.setAutoRemove(false);
    logFile_ = tempFile.fileName();
    options.set(constants::MakeBlastDb::kLogFileOpt, logFile_);
    if (!makeBlastDatabaseWrapper_->setOptions(options))
    {
        onMakeBlastDatabaseError(0, "Unable to set one or more options. Please contact support.");
        return;
    }

    // Kick off the request
    try
    {
        makeBlastDatabaseWrapper_->formatDatabase(inputFile);
    }
    catch (QString &errorMessage)
    {
        onMakeBlastDatabaseError(0, errorMessage);
    }

    // Timer to tweak the buttons status after this virtual method has completed
    QTimer::singleShot(0, this, SLOT(queuedInitializePage()));
}

/**
  */
void FormatDatabasePage::queuedInitializePage()
{
    wizard()->button(QWizard::BackButton)->setEnabled(false);
}

/**
  * @param databaseFile [const QString &]
  */
void FormatDatabasePage::onFormatFinished(const QString & /* databaseFile */)
{
    // Successfully finished
    progressBar_->setRange(0, 100);
    progressBar_->setValue(100);

    cancelButton_->setEnabled(false);
    wizard()->setOption(QWizard::NoBackButtonOnLastPage, true);

    setComplete(true);
    wizard()->setOption(QWizard::NoCancelButton, true);

    // Read in the log file and update the text edit
    QFile file(logFile_);
    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);
        formatLogTextEdit_->setPlainText(file.readAll().trimmed());
        file.close();

        file.remove();
        logFile_.clear();
    }

    // Add database to blast database manager
    if (field("addPathToBlastManager").toBool() &&
        blastDatabaseModel_ != nullptr)
    {
        QString outputDirectory = field("outputDirectory").toString();
        if (blastDatabaseModel_->addBlastPath(outputDirectory))
            blastDatabaseModel_->refresh(outputDirectory);
    }
}

/**
  * @param formattedSequenced [int]
  */
void FormatDatabasePage::onMakeBlastDatabaseProgressChanged(int formattedSequences)
{
    actualFormatLabel_->setText(QString("%1 sequences").arg(formattedSequences));
}

/**
  * @param id [int]
  * @param errorMessage [const QByteArray &]
  */
void FormatDatabasePage::onMakeBlastDatabaseError(int /* id */, const QString &errorMessage)
{
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);

    cancelButton_->setEnabled(false);
    if (!errorMessage.isEmpty())
    {
        formatLogTextEdit_->setPlainText(errorMessage);
    }
    else
    {
        // ISSUE: This should not happen, but for some reason, it is proving impossible to capture exception messages
        //        dumped by the NCBI toolkit
        formatLogTextEdit_->setPlainText(QString("An error occurred while attempting to read the sequence file. Please "
                                                 "verify that the file, %1, contains FASTA formatted sequence data.")
                                         .arg(field("inputFile").toString()));
    }

    // Restore the back button
    wizard()->setOption(QWizard::NoBackButtonOnLastPage, false);

    // Re-enable the back button
    wizard()->button(QWizard::BackButton)->setEnabled(true);

    // Remove any potential log file
    if (!logFile_.isEmpty())
    {
        QFile::remove(logFile_);
        logFile_.clear();
    }
}

/**
  * @returns OptionSet
  */
OptionSet FormatDatabasePage::getOptions() const
{
    OptionSet options;

    using namespace constants::MakeBlastDb;

    // Input file is handled by passing this parameter directly

    // Output directory
    QString outputDirectory = field("outputDirectory").toString();
    ASSERT(!outputDirectory.isEmpty());
    ASSERT(QDir(outputDirectory).exists());

    // Output file name
    QString outputFile = field("outputFile").toString();
    if (outputFile.isEmpty())
    {
        QString inputFile = field("inputFile").toString();
        QFileInfo fileInfo(inputFile);
        outputFile = fileInfo.fileName();
    }
    options << Option(kOutFileOpt, outputDirectory + QDir::separator() + outputFile);

    // Title
    QString title = field("title").toString();
    if (!title.isEmpty())
        options << Option(kTitleOpt, title);

    // Molecule type
    bool isProtein = field("moleculeType").toBool();
    options << Option(kMoleculeTypeOpt, (isProtein) ? kMoleculeTypeProtein : kMoleculeTypeNucleotide);

    // Parse seq ids
    if (field("parseSeqIds").toBool())
        options << Option(kParseSeqIdsOpt);

    return options;
}

/**
  * @param newComplete [bool]
  */
void FormatDatabasePage::setComplete(bool newComplete)
{
    if (newComplete == complete_)
        return;

    complete_ = newComplete;
    emit completeChanged();
}
