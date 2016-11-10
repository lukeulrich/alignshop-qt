#ifndef IMPORTSEQUENCESDIALOG_H
#define IMPORTSEQUENCESDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QFileDialog>

#include "../models/ParsedBioStringTableModel.h"

#include "../DataFormatDetector.h"
#include "../MaxCoverageAlphabetDetector.h"

#include "ui_ImportSequencesDialog.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
namespace Ui {
    class ImportSequencesDialog;
}

class DataFormat;

/**
  * ImportSequencesDialog provides the GUI interface for importing sequence data into AlignShop.
  *
  * The set of accepted data formats are defined upon construction and maintained within a DataFormatDetector
  * instance. Similarly, the alphabet detection process is performed by a MaxCoverageAlphabetDetector.
  *
  * A typical usage involves the user first selecting an appropriate sequence data file after which its
  * data format and alphabet are automatically detected and then its sequences parsed. The resulting dialog
  * then provides the user with several options regarding the import:
  * o May select another file
  * o Change/select the exact data format
  *   Everytime this field changes, the file will be reparsed with respect to this data format and update
  *   the results.
  * o Auto-detect the data-format
  * o Change/select the exact alphabet
  *   Everytime the alphabet is changed, all parsed sequences will be analyzed for their validity with respect
  *   to this alphabet.
  * o Select the destination for the imported sequences
  * o Preview and select valid sequences of interest to import (via checkboxes)
  *
  * Only sequences that are valid according to the given alphabet may be imported.
  *
  * Stylistic details:
  * o Set per-section minimum sizes by intercepting the sectionResized signal and resizing to a fixed limit
  *   if these are exceeded. Specific bounds:
  *   - Column 0 (checkbox / label): Min = 100
  *   - Column 1 (sequence): Min = 100, auto-stretch to larger
  *   - Column 2 (valid): Min/Max = 60
  *
  * At least one valid sequence must be present for the Import button to be enabled. This is toggle in the
  * validateParsedBioStrings method.
  *
  * Depending on the number of sequences loaded with any given file and the lifetime of this dialog class,
  * a significant amount of memory may be utilized for the duration of the program. Thus, it is advisable to
  * clear the model after this dialog is closed.
  *
  * TODO: Test this class!
  */
class ImportSequencesDialog : public QDialog, public Ui::ImportSequencesDialog
{
    Q_OBJECT
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Constructs an instance of this dialog and performs other necessary intialization including: signals/slots setup, detector configuration, and miscellaneous widget details
    explicit ImportSequencesDialog(QWidget *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                              //!< Returns the currently selected alphabet
    void clearModel();                                      //!< Clears all model data
    int countChecked() const;                               //!< Returns the number of sequences that have been checked
    bool isAlignmentChecked() const;                        //!< Returns true if the importAlignmentRadioButton is checked
    void positionSequenceFileDialog(QWidget *widget);       //! Explicitly position the file selection dialog just below and horizontally centered relative to widget
    //! Loads and analyzes file for import purposes using dataFormat; if dataFormat is 0, then the actual file format will be auto-detected
    void processFile(const QString &file, DataFormat *dataFormat = 0);
    QList<ParsedBioString> selectedSequences() const;       //!< Returns the list of selected sequences
    void validateParsedBioStrings(Alphabet alphabet);       //!< Updates the valid status of the model based on whether they conform to alphabet

    // ------------------------------------------------------------------------------------------------
    // Public slots
public slots:
    void changeSequenceFile();                              //!< Opens the file selection dialog box and processes the selected file if it is different than the currently selected one
    QString getSequenceFile();                              //!< Displays a file dialog box for selecting a sequence file and returns the file name if a file is selected or empty QString otherwise
    void onAlphabetActivated(int index);                    //!< Calls validateParsedBioStrings(alphabet) using the alphabet of alphabetComboBox specified by index
    void onFormatActivated(int index);                      //!< Re-parses the sourceFile whenever the formatComboBox is changed to index
    void updateImportButtonDetails() const;                 //!< Enables/disables the importButton relative to the number of selected sequences and the currently selected import action (sequences or alignment)

    // ------------------------------------------------------------------------------------------------
    // Private slots
private slots:
    void onCheckedChange(const QModelIndex &index) const;   //!< Called whenever a sequence checkbox is checked or unchecked via the model setData method
//    void onItemClicked(const QModelIndex &index) const;     //!< Called whenever the user clicks on an item in the table view
    //! Override the section resize signal in order to enforce specific per-column size constraints (see class notes)
    void parsedSectionResize(int logicalIndex, int oldSize, int newSize);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void initializeDataFormatDetector();                    //!< Utility function that prepares the DataFormatDetector with the list of compatible DataFormats and inspectors
    void initializeAlphabetDetector();                      //!< Utility function that prepares the AlphabetDetector with the list of compatible alphabets and inspectors

    QFileDialog importFileSelectionDialog_;                 //!< Dialog used to select a sequence file to import
    DataFormatDetector dataFormatDetector_;
    MaxCoverageAlphabetDetector maxCoverageAlphabetDetector_;
    ParsedBioStringTableModel model_;

    QHash<Alphabet, BioStringValidator> validators_;        //!< Specific validators for revalidating sequence data
    QString sourceFile_;                                    //!< The user-selected source file
};

#endif // IMPORTSEQUENCESDIALOG_H
