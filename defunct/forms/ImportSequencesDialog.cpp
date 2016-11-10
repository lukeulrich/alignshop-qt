#include "ImportSequencesDialog.h"
#include "ui_ImportSequencesDialog.h"

#include <QtCore/QFile>
#include <QtCore/QStringBuilder>
#include <QtCore/QTextStream>

#include <QtGui/QMessageBox>

#include "AlphabetInspector.h"
#include "BioString.h"
#include "BioStringValidator.h"
#include "ClustalFormatInspector.h"
#include "ClustalParser.h"
#include "DataFormat.h"
#include "FastaFormatInspector.h"
#include "FastaParser.h"
#include "ParseError.h"
#include "Picker.h"
#include "SimpleSeq.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * We specifically initialize a child dialog, importFileSelectioniDialog_, in the constructor so that
  * it may be positioned via positionSequenceFileDialog() before showing the complete ImportSequencesDialog
  * form.
  *
  * @param parent [QWidget *]
  */
ImportSequencesDialog::ImportSequencesDialog(QWidget *parent) : QDialog(parent), importFileSelectionDialog_(this, "Select file to import")
{
    // Prepare the user interface
    setupUi(this);

    // Signals and slots
    QObject::connect(changeButton, SIGNAL(clicked()), this, SLOT(changeSequenceFile()));

    importAlignmentWarningLabel->hide();

    // Initialize the detectors
    initializeDataFormatDetector();
    initializeAlphabetDetector();

    // Add the various formats to the combobox
    formatComboBox->addItem("Auto detect", constants::kAutoDetectDataFormat);
    formatComboBox->insertSeparator(formatComboBox->count());
    foreach (DataFormat *dataFormat, dataFormatDetector_.dataFormats())
        formatComboBox->addItem(dataFormat->name(), dataFormat->type());
    formatComboBox->addItem("Unknown", eUnknownFormatType);

    // Set the default data format to unknown
    formatComboBox->setCurrentIndex(formatComboBox->count() - 1);

    // Add the various alphabets to the combobox
    alphabetComboBox->addItem("Auto detect", constants::kAutoDetectAlphabet);
    alphabetComboBox->insertSeparator(alphabetComboBox->count());
    alphabetComboBox->addItem("Protein", eAminoAlphabet);
    alphabetComboBox->addItem("DNA", eDnaAlphabet);
    alphabetComboBox->addItem("RNA", eRnaAlphabet);
    alphabetComboBox->addItem("Unknown", eUnknownAlphabet);

    // Set the default alphabet to unknown
    alphabetComboBox->setCurrentIndex(3);

    // Connect the onChanged signal to validate the sequences
    QObject::connect(alphabetComboBox, SIGNAL(activated(int)), this, SLOT(onAlphabetActivated(int)));

    // Reparse the file when the format combo box changes
    QObject::connect(formatComboBox, SIGNAL(activated(int)), this, SLOT(onFormatActivated(int)));

    // Configure the file selection dialog to only select a single file and provide relevant name filters
    importFileSelectionDialog_.setFileMode(QFileDialog::ExistingFile);
    importFileSelectionDialog_.setNameFilters(DataFormat::nameFilters(dataFormatDetector_.dataFormats()) << "All files (*)");

    // Setup the parsed table model
    parsedTableView->setModel(&model_);

    // Configure the table view
    parsedTableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    parsedTableView->horizontalHeader()->resizeSection(2, 60);                  // Initially set the valid column to 60 pixels in width
    parsedTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);       // Disable resizing of the rows
    parsedTableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Enforce the minimum and maximum column sizes using a SIGNAL/SLOT
    QObject::connect(parsedTableView->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(parsedSectionResize(int,int,int)));

    // Hook up the checkbox counting to enable/disabling of the importButton
    QObject::connect(&model_, SIGNAL(checkedChange(QModelIndex)), this, SLOT(onCheckedChange(QModelIndex)));

    // Whenever the import{Alignment,Sequences}RadioButton is toggled, update the GUI
    QObject::connect(importAlignmentRadioButton, SIGNAL(toggled(bool)), this, SLOT(updateImportButtonDetails()));
    QObject::connect(importSequencesRadioButton, SIGNAL(toggled(bool)), this, SLOT(updateImportButtonDetails()));

    // Connect the table view clicking event to toggle the checkbox when user single-clicks item label
//    QObject::connect(parsedTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns Alphabet
  */
Alphabet ImportSequencesDialog::alphabet() const
{
    return static_cast<Alphabet>(alphabetComboBox->itemData(alphabetComboBox->currentIndex()).toInt());
}

/**
  */
void ImportSequencesDialog::clearModel()
{
    model_.clear();
}

/**
  * Loops through the list of model ParsedBioString's and counts the number that are checked.
  *
  * @returns int
  */
int ImportSequencesDialog::countChecked() const
{
    int nChecked = 0;
    QList<ParsedBioString> pbss = model_.parsedBioStrings();
    for (int i=0, z=pbss.count(); i<z; ++i)
        if (pbss.at(i).checked_)
            ++nChecked;

    return nChecked;
}

/**
  * @returns bool
  */
bool ImportSequencesDialog::isAlignmentChecked() const
{
    return importAlignmentRadioButton->isChecked();
}

/**
  * The purpose of this function is to strategically position the file selection dialog such that it appears
  * horizontally centered above widget and a small space below widget's title bar. The default functionality
  * is not sufficient because it is desirable for the user to select a file before showing the complete import
  * sequences dialog window. If the file selection window is shown while its parent (importSequencesDialog) is
  * still invisible, the file selection dialog is simply centered on the screen - not on the triggering window.
  *
  * A side effect is that once a dialog is specifically positioned, it always uses these coordinates going forward.
  * Thus, we must call this function everytime before showing the file selection dialog or else it will appear
  * in the last configured position.
  *
  * @param widget [QWidget *]
  */
void ImportSequencesDialog::positionSequenceFileDialog(QWidget *widget)
{
    Q_ASSERT_X(widget, "ImportSequencesDialog::positionSequenceFileDialog", "widget must not be 0");

    QRect src_rect = widget->geometry();

    int x1 = src_rect.x() + (src_rect.width() - importFileSelectionDialog_.width()) / 2;
    int y1 = src_rect.y() + 10;

    importFileSelectionDialog_.move(x1, y1);
}

/**
  * Major data-processing method for importing a sequence data file. Since we are processing a new file,
  * we reset the model and relevant form controls to a default state before processing.
  *
  * If dataFormat is zero, then attempts to detect the file format automatically.
  *
  * @param file [const QString &]
  * @param dataFormat [DataFormat *, default = 0]
  */
void ImportSequencesDialog::processFile(const QString &file, DataFormat *dataFormat)
{
    // Clear the model
    model_.clear();

    // Reset the relevant form controls to a default state
    parsedTableView->verticalHeader()->hide();      // For aesthetic purposes
    parsedLabel_->setText("");
    alphabetComboBox->setEnabled(false);
    parsedTableView->setEnabled(false);
    importButton->setEnabled(false);
    importSequencesRadioButton->setChecked(true);
    importSequencesRadioButton->setEnabled(false);
    importAlignmentRadioButton->setEnabled(false);

    // ---------------------------------
    // Attempt to detect the file format
    QFile sequenceFile(file);
    if (!dataFormat)
    {
        dataFormat = dataFormatDetector_.formatFromFile(sequenceFile);
        Q_ASSERT_X(dataFormat, "ImportSequencesDialog::processFile", "detector should always return a non null pointer via formatFromFile");

        // Update the format combobox to the detected value
        formatComboBox->setCurrentIndex(formatComboBox->findData(dataFormat->type()));
        if (dataFormat->type() == eUnknownFormatType)
        {
            QMessageBox::warning(this, tr("Unable to determine file format"), QString("'%1' ").arg(file) % tr("does not have a recognized sequence data format.\n\nPlease select the appropriate data format."), QMessageBox::Ok);
            return;
        }
    }

    // ----------------------------------------------------------------
    // Because there is an unknown item present, check for its presence
    if (dataFormat->type() == eUnknownFormatType)
    {
        parsedLabel_->setText("No sequences found");
        return;
    }

    // ---------------------------------
    // Check that the sequence file is open
    bool can_open = sequenceFile.open(QFile::ReadOnly);
    Q_ASSERT_X(can_open, "ImportSequencesDialog::processFile", "Unable to open file. Should have been previously verified in getSequenceFile");
    if (!can_open)  // Should only be possible in release compile, debug will catch this with the above assert
    {
        QMessageBox::warning(this, tr("Unable to open file"), QString("'%1' ").arg(file) % tr("could not be read.\n\nPlease check if the file is valid."), QMessageBox::Ok);
        return;
    }

    // ---------------------------------
    // Attempt to read in the simpleseqs
    QTextStream in(&sequenceFile);
    QList<SimpleSeq> simpleSeqs;
    ParseError e;
    // Attempt to read all the simple seqs
    if (!dataFormat->parser()->readAll(in, simpleSeqs, e))  // Unable to parse data file
    {
        parsedLabel_->setText("No sequences found");
        QMessageBox::warning(this, tr("Import error"), QString("'%1' could not be parsed.\n\nCheck that it is valid %2 file. Please select the appropriate data format from the format drop down box or select another file.\n\nError: ").arg(file).arg(dataFormat->name()) % e.message(), QMessageBox::Ok);
        return;
    }

    if (simpleSeqs.count() == 0)    // No sequences found inside data file
    {
        parsedLabel_->setText("No sequences found");
        QMessageBox::warning(this, tr("No sequences found"), QString("'%1' ").arg(file) % tr("does not contain any sequence data in this format."), QMessageBox::Ok);
        return;
    }

    // Close the file if it is open
    if (sequenceFile.isOpen())
        sequenceFile.close();

    // At least one sequence is available, update the GUI labels and controls
    bool allSequenceLengthsEqual = true;
    importSequencesRadioButton->setEnabled(true);
    parsedLabel_->setText(QString("Sequence count: %L1").arg(simpleSeqs.count()));
    parsedTableView->verticalHeader()->show();
    alphabetComboBox->setEnabled(true);
    parsedTableView->setEnabled(true);

    // Attempt to determine the alphabet and also check the sequence lengths
    QList<BioString> bioStrings;
    QList<ParsedBioString> pbss;
    for (int i=0, z=simpleSeqs.count(); i<z; ++i)
    {
        bioStrings.append(BioString(simpleSeqs.at(i).sequence()));
        pbss.append(ParsedBioString(bioStrings.last(), simpleSeqs.at(i).header(), false));

        if (allSequenceLengthsEqual
            && i > 0
            && bioStrings.at(i).length() != bioStrings.at(i-1).length())
        {
            allSequenceLengthsEqual = false;
        }
    }
    model_.setParsedBioStrings(pbss);
    parsedTableView->resizeColumnToContents(0);

    Alphabet majorityAlphabet = pickMostFrequent(maxCoverageAlphabetDetector_.detectAlphabets(bioStrings)).at(0);
    alphabetComboBox->setCurrentIndex(alphabetComboBox->findData(majorityAlphabet));
    if (majorityAlphabet != eUnknownAlphabet)
    {
        // Do other processing
        validateParsedBioStrings(majorityAlphabet);
    }
    else // Unable to determine alphabet
        QMessageBox::warning(this, tr("Unable to determine sequence alphabet"), QString("'%1' ").arg(file) % tr("does not have a recognized sequence alphabet."), QMessageBox::Ok);

    // Update the alignment radio button (if possible)
    if (allSequenceLengthsEqual && pbss.count() > 1)
    {
        importAlignmentRadioButton->setEnabled(true);

        // If the data format represents an alignment, automatically check the import alignment radio button
        if (dataFormat->type() == eClustalType)
            importAlignmentRadioButton->setChecked(true);
    }
}

/**
  * Returns a list of those sequences that are checked.
  *
  * @returns QList<ParsedBioString>
  */
QList<ParsedBioString> ImportSequencesDialog::selectedSequences() const
{
    QList<ParsedBioString> selection;

    QList<ParsedBioString> pbss = model_.parsedBioStrings();
    for (int i=0, z=pbss.size(); i<z; ++i)
    {
        if (pbss.at(i).checked_)
            selection.append(pbss.at(i));
    }

    return selection;
}

/**
  * Loops through all ParsedBioString's in the associated model and re-validates them relative to alphabet. If at
  * least one valid sequence is found, then the importButton is enabled, otherwise it is disabled.
  *
  * @param alphabet [Alphabet]
  */
void ImportSequencesDialog::validateParsedBioStrings(Alphabet alphabet)
{
    QList<ParsedBioString> pbss = model_.parsedBioStrings();
    if (alphabet != eUnknownAlphabet)
    {
        Q_ASSERT_X(validators_.contains(alphabet), "ImportSequencesDialog::validateParsedBioStrings", "alphabet must exist in the validators hash");

        // Count the number of validSequences to determine whether to
        int validSequences = 0;
        const BioStringValidator validator = validators_[alphabet];
        for (int i=0, z=pbss.size(); i< z; ++i)
        {
            bool valid = validator.isValid(pbss.at(i).bioString_);
            if (valid)
                ++validSequences;
            model_.setData(model_.index(i, 2), valid);
        }

        if (validSequences)
            importButton->setEnabled(true);
    }
    else
    {
        for (int i=0, z=pbss.size(); i< z; ++i)
            model_.setData(model_.index(i, 2), false);
        importButton->setEnabled(false);
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * This method is called when the user clicks on the 'Select another file...' button. If the user
  * selects a different file than the currently selected file (sourceFile_), it will attempt to load
  * this file.
  *
  * @see processFile()
  */
void ImportSequencesDialog::changeSequenceFile()
{
    // Position the file selection box relative to ourselves
    positionSequenceFileDialog(this);

    // Remember the currently selected file because we only need to process the selected file if it is different
    // from what we already have.
    QString old_file = sourceFile_;
    QString sequenceFile = getSequenceFile();

    // Conditions for processing the file:
    // o The user selected a file (!sequenceFile.isEmpty()) AND
    //   1. There was no previous file selected OR
    //   2. sequenceFile is different than the old file
    if (!sequenceFile.isEmpty() && (old_file.isEmpty() || sequenceFile != old_file))
        processFile(sequenceFile);
}

/**
  * After user has selected a single file will continue forever unless:
  * o User selects a non empty file that can be opened for reading
  * o Cancels the operation
  *
  * Sets the window title to the full path of the file being imported.
  *
  * An empty QString is returned in the event the user cancels the operation.
  *
  * @returns QString
  */
QString ImportSequencesDialog::getSequenceFile()
{
    QString sequenceFile;
    forever
    {
        // Ask the user for a sequence file - the filters and so forth are configured within the constructor
        if (importFileSelectionDialog_.exec() == QDialog::Rejected)
            return "";

        // Only one file is considered and this is the first one
        QString filename = importFileSelectionDialog_.selectedFiles().at(0);

        // Make sure that the file is not empty
        QFile file(filename);
        if (file.size() == 0)
        {
            QMessageBox::warning(this, tr("Empty file"), QString("'%1' ").arg(filename) % tr("does not contain any data."),    QMessageBox::Ok);
            continue;
        }

        // Attempt to open the file for reading
        if (!file.open(QFile::ReadOnly))
        {
            QMessageBox::warning(this, tr("Unable to open file"), QString("'%1' ").arg(filename) % tr("could not be read.\n\nPlease check if the file is valid."), QMessageBox::Ok);
            continue;
        }

        file.close();
        sequenceFile = filename;

        this->setWindowTitle(QString("Sequence import: %1").arg(sequenceFile));

        // Update the label of the file to import
        sourceFile_ = sequenceFile;
        QFileInfo fi(file);
        QString label = fi.baseName();

        // Only append suffix if it is present
        if (!fi.completeSuffix().isEmpty())
            label += '.' % fi.completeSuffix();
        sourceFileLabel_->setText(label);

        break;
    }

    // Return the selected sequence file
    return sequenceFile;
}

/**
  * This method should be triggered when the user changes the selection in the alphabetComboBox and ultimately calls
  * validateParsedBioStrings. If the user selects the Auto Detect option, the list of BioStrings is passed to the max
  * alphabet coverage detector to determine the most likely alphabet.
  *
  * @param index [int]
  */
void ImportSequencesDialog::onAlphabetActivated(int index)
{
    // Sanity check to ensure that index is not out of range
    Q_ASSERT_X(alphabetComboBox->count(), "ImportSequencesDialog::onAlphabetChanged", "There must be at least one alphabet in the alphabet combo box");
    Q_ASSERT_X(index >= 0, "ImportSequencesDialog::onAlphabetChanged", "index must be greater than 0");
    Q_ASSERT_X(index < alphabetComboBox->count(), "ImportSequencesDialog::onAlphabetChanged", "index must be less than alphabetComboBox->count()");

    Alphabet alphabet = eUnknownAlphabet;
    if (alphabetComboBox->itemData(index).toInt() == constants::kAutoDetectAlphabet)
    {
        QList<BioString> bioStrings;
        QList<ParsedBioString> pbss = model_.parsedBioStrings();
        for (int i=0, z=pbss.size(); i<z; ++i)
            bioStrings.append(pbss.at(i).bioString_);

        alphabet = pickMostFrequent(maxCoverageAlphabetDetector_.detectAlphabets(bioStrings)).at(0);
        if (alphabet == eUnknownAlphabet)
            QMessageBox::warning(this, tr("Unable to determine sequence alphabet"), tr("Please select the appropriate alphabet manually."), QMessageBox::Ok);

        alphabetComboBox->setCurrentIndex(alphabetComboBox->findData(alphabet));
    }
    else
        alphabet = static_cast<Alphabet>(alphabetComboBox->itemData(index).toInt());

    validateParsedBioStrings(alphabet);
}

/**
  * This method is called when the user changes the formatComboBox selection and ultimately calls processFile
  * with the relevant data format.
  *
  * @index [int]
  */
void ImportSequencesDialog::onFormatActivated(int index)
{
    // Sanity check to ensure that index is not out of range
    Q_ASSERT_X(formatComboBox->count(), "ImportSequencesDialog::onFormatChanged", "There must be at least one format in the format combo box");
    Q_ASSERT_X(index >= 0, "ImportSequencesDialog::onFormatChanged", "index must be greater than 0");
    Q_ASSERT_X(index < formatComboBox->count(), "ImportSequencesDialog::onFormatChanged", "index must be less than formatComboBox->count()");

    // Get the file that to be processed
    Q_ASSERT_X(!sourceFile_.isEmpty(), "ImportSequencesDialog::onFormatChanged", "sourceFile_ must not be empty");

    if (formatComboBox->itemData(index).toInt() == constants::kAutoDetectDataFormat)
    {
        // Calling processFile with only one argument will trigger it to auto-detect the format
        processFile(sourceFile_);
        return;
    }

    // The data formats (setup in the constructor via initializeDataFormatDetector) were added to the formatComboBox in
    // order of the formats of dataFormatDetector_. Thus, to obtain the corresponding parser, we simply have to index into
    // this list using the corresponding formatComboBox index.
    DataFormat *dataFormat = 0;
    DataFormatType dataFormatType = static_cast<DataFormatType>(formatComboBox->itemData(index).toInt());
    if (dataFormatType != eUnknownFormatType)
    {
        // Find the data format in the list contained by the detector
        QList<DataFormat *> formats = dataFormatDetector_.dataFormats();
        foreach (DataFormat *format, formats)
        {
            if (dataFormatType == format->type())
            {
                dataFormat = format;
                break;
            }
        }
    }
    else
    {
        dataFormat = dataFormatDetector_.unknownFormat();
    }
    processFile(sourceFile_, dataFormat);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * Updates the import button controls whenever the number of checked sequences changes.
  *
  * @param index [const QModelIndex &]
  */
void ImportSequencesDialog::onCheckedChange(const QModelIndex &index) const
{
    // Must be a valid index at column zero
    if (!index.isValid() || index.column() != 0)
        return;

    updateImportButtonDetails();
}

/**
  * Enables or disables the importButton depending on the number of sequences that have been checked
  * by the user and whether the user is importing purely sequence data (importSequencesRadioButton)
  * or an alignment (importAlignmentRadioButton).
  *
  * The importButton is enabled if:
  * o importing sequences and there is at least one selected sequence
  * o importing an alignment and there is at least two selected sequences
  *
  * Also shows/hides the alignment warning label ("Select at least 2 sequences") as appropriate.
  */
void ImportSequencesDialog::updateImportButtonDetails() const
{
    if (importSequencesRadioButton->isChecked())
    {
        importAlignmentWarningLabel->hide();
        importButton->setEnabled(countChecked() > 0);
    }
    else // if (importAlignmentRadioButton->isChecked())
    {
        if (countChecked() >= 2)
        {
            importButton->setEnabled(true);
            importAlignmentWarningLabel->hide();
        }
        else
        {
            importButton->setEnabled(false);
            importAlignmentWarningLabel->show();
        }
    }
}

/**
  * This method enforces custom size constraints on the header by intercepting its resize event and
  * resizing the relevant header section to its boundary size if exceeded. The specific rules are below:
  * o Label column: min = 100, max = viewport - 60 (valid column) and 150 for the sequence and row number
  * o Valid column: fixed = 60
  *
  * @param logicalIndex [int]
  * @param oldSize [int]
  * @param newSize [int]
  */
void ImportSequencesDialog::parsedSectionResize(int logicalIndex, int /* oldSize */, int newSize)
{
    switch (logicalIndex)
    {
    case ParsedBioStringTableModel::kLabelColumn:
        {
            int tw = parsedTableView->horizontalHeader()->viewport()->width();
            if (newSize < 100)
                parsedTableView->horizontalHeader()->resizeSection(0, 100);
            else if (newSize > tw - 210)
                parsedTableView->horizontalHeader()->resizeSection(0, tw - 210);
        }
        break;
    case ParsedBioStringTableModel::kValidColumn:
        if (newSize != 60)
            parsedTableView->horizontalHeader()->resizeSection(2, 60);
        break;
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * Configures the detector with the appropriate data formats. These are all allocated on the stack; however,
  * the appropriate de-allocation is all handled by the detector instance, which ultimately owns these pointers.
  * Thus it is not necessary to call delete.
  *
  * We associate with each data format type, a friendly name, a list of file extensions, a specific format
  * inspector, and a specific simple seq parser for parsing this data into simpleseqs.
  */
void ImportSequencesDialog::initializeDataFormatDetector()
{
    // Import data format
    QList<DataFormat *> importFormats;

    // o Fasta files
    importFormats.append(new DataFormat(eFastaType,
                                        "Fasta",
                                        QStringList() << "fa" << "faa" << "fnt" << "fasta",
                                        new FastaFormatInspector(),
                                        new FastaParser()));

    // o Clustal files
    importFormats.append(new DataFormat(eClustalType,
                                        "Clustal",
                                        QStringList() << "aln" << "clustal",
                                        new ClustalFormatInspector(),
                                        new ClustalParser()));

    // Takes ownership of DataFormat * lists
    dataFormatDetector_.setDataFormats(importFormats);
}

/**
  * In this case, we configure two BioStringValidators for each relevant alphabet (Amino, DNA, RNA) - one for the
  * base character set and another for the expanded character set. The stop codon character (i.e. *) is permitted
  * DNA and RNA sequences.
  *
  * A copy of the validator for each expanded character set is mapped to each alphabet for use when re-validating
  * sequences on-demand.
  */
void ImportSequencesDialog::initializeAlphabetDetector()
{
    QList<AlphabetInspector> inspectors;

    // Amino acid
    inspectors.append(AlphabetInspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters)));
    inspectors.append(AlphabetInspector(eAminoAlphabet, BioStringValidator(constants::kAminoExpandedCharacters)));
    validators_[eAminoAlphabet] = inspectors.last().validator();

    // DNA
    inspectors.append(AlphabetInspector(eDnaAlphabet, BioStringValidator(constants::kDnaCharacters)));
    inspectors.append(AlphabetInspector(eDnaAlphabet, BioStringValidator(QString(constants::kDnaExpandedCharacters) % constants::kStopCodonCharacter)));
    validators_[eDnaAlphabet] = inspectors.last().validator();

    // RNA
    inspectors.append(AlphabetInspector(eRnaAlphabet, BioStringValidator(constants::kRnaCharacters)));
    inspectors.append(AlphabetInspector(eRnaAlphabet, BioStringValidator(QString(constants::kRnaExpandedCharacters) % constants::kStopCodonCharacter)));
    validators_[eRnaAlphabet] = inspectors.last().validator();

    maxCoverageAlphabetDetector_.setInspectors(inspectors);
}






/**
  * If the user single-clicks on the item label, toggle the checkbox.
  *
  * @param index [const QModelIndex &]
  */
//void ImportSequencesDialog::onItemClicked(const QModelIndex &index) const
//{
//    if (index.column() == )
//    qDebug() << "Clicked!";
//}

