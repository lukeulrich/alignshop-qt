/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MAKEBLASTDATABASEWIZARD_H
#define MAKEBLASTDATABASEWIZARD_H

#include <QtCore/QString>

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>

class QCheckBox;
class QFileSystemModel;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QTreeView;

class BlastDatabaseModel;
class ColumnFilterProxyModel;
class FormatDatabasePage;
class MakeBlastDatabaseWrapper;

class OptionSet;

class MakeBlastDatabaseWizard : public QWizard
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    MakeBlastDatabaseWizard(BlastDatabaseModel *blastDatabaseModel, QWidget *parent = 0);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void reject();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    FormatDatabasePage *formatDatabasePage_;    // So we can check if it is ok to close the dialog
};


class SelectFilePage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(QString inputFile READ inputFile WRITE setInputFile NOTIFY inputFileChanged)

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit SelectFilePage(QWidget *parent = 0);
    ~SelectFilePage();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isComplete() const;
    QString inputFile() const;
    void setInputFile(const QString &inputFile);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void inputFileChanged();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void initializePage();


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void scrollToSelectedItem();
    void onCurrentRowChanged(const QModelIndex &currentIndex);
    void onIndexDoubleClicked(const QModelIndex &index);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setComplete(bool newComplete);


    // ------------------------------------------------------------------------------------------------
    // Private members
    QFileSystemModel *fileSystemModel_;
    ColumnFilterProxyModel *proxyModel_;
    QTreeView *treeView_;
    bool complete_;

    QString inputFile_;
};


class DatabaseConfigurationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit DatabaseConfigurationPage(QWidget *parent = 0);

protected:
    void initializePage();
    bool validatePage();

private Q_SLOTS:
    void onOutputDirectoryButtonReleased();

private:
    QLabel *inputFileLabel_;            // "Sequence file:"
    QLabel *actualInputFileLabel_;      // "/home/ulrich/my_sequences.faa"
    QLabel *outputDirectoryLabel_;
    QLabel *actualOutputDirectoryLabel_;
    QLabel *databaseNameLabel_;
    QLabel *titleLabel_;
    QLabel *typeLabel_;

    QLineEdit *outputDirectoryLineEdit_;
    QLineEdit *databaseNameLineEdit_;
    QPushButton *outputDirectoryPushButton_;
    QLineEdit *titleLineEdit_;
    QRadioButton *proteinRadioButton_;
    QRadioButton *nucleotideRadioButton_;
    QCheckBox *parseSeqIdsCheckBox_;
    QCheckBox *addPathCheckBox_;
};


class FormatDatabasePage : public QWizardPage
{
    Q_OBJECT

public:
    FormatDatabasePage(BlastDatabaseModel *blastDatabaseModel, QWidget *parent = 0);

    bool isActive() const;              // Returns true if a format is underway
    bool isComplete() const;


public Q_SLOTS:
    void cancelFormat();


protected:
    void initializePage();


private Q_SLOTS:
    void queuedInitializePage();        // Because the wizard will reset all the buttons after initializePage has
                                        // finished, this method is called via a timer after intializePage() and
                                        // allows for further fine-tuning of the buttons/etc.
                                        // http://www.qtcentre.org/threads/42142-QWizard-Temporarily-disable-the-Back-button

    void onFormatFinished(const QString &databaseFile);
    void onMakeBlastDatabaseProgressChanged(int formattedSequences);
    void onMakeBlastDatabaseError(int id, const QString &errorMessage);


private:
    OptionSet getOptions() const;
    void setComplete(bool newComplete);

    QLabel *formatLabel_;
    QLabel *actualFormatLabel_;
    QPushButton *cancelButton_;
    QProgressBar *progressBar_;
    QLabel *logLabel_;
    QPlainTextEdit *formatLogTextEdit_;

    BlastDatabaseModel *blastDatabaseModel_;
    MakeBlastDatabaseWrapper *makeBlastDatabaseWrapper_;
    bool complete_;
    QString logFile_;
};

#endif // MAKEBLASTDATABASEWIZARD_H
