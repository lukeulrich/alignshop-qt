/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERCREATORWIZARD_H
#define PRIMERCREATORWIZARD_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>

#include "../../core/global.h"
#include "../../primer/PrimerPair.h"
#include "../../primer/PrimerSearchParameters.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QDoubleSpinBox;
class QGroupBox;
class QIntValidator;
class QLabel;
class QModelIndex;
class QProgressBar;
class QSortFilterProxyModel;
class QSpinBox;
class QSplitter;
class QTableView;
class QThread;
class QToolButton;
class QLineEdit;

class AgDoubleSpinBox;
class BioString;
class FilterColumnProxyModel;
class PrimerPairFinderResult;
class PrimerPairModel;
class PrimerParameterSetupPage;
class PrimerSearchingPage;
class PrimerResultsPage;
class RestrictionEnzymeLineEdit;
class SequenceTextView;
class SignalPrimerPairFinder;
class ThreePrimeInput;

/**
  * PrimerCreatorWizard walks users through generating new primers from a given DNA sequence.
  */
class PrimerCreatorWizard : public QWizard
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerCreatorWizard(const BioString &bioString, QWidget *parent = nullptr);
    PrimerCreatorWizard(const BioString &bioString, const PrimerSearchParameters *primerSearchParameters, QWidget *parent);
    PrimerCreatorWizard(const BioString &bioString, const PrimerSearchParameters &primerSearchParameters, QWidget *parent);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    PrimerPairVector generatedPrimerPairs() const;
    void setInitialSearchRange(const ClosedIntRange &range);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void reject();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void constructor_(PrimerParameterSetupPage *primerParameterSetupPage);
    void autoNamePrimerPairs(PrimerPairVector &primerPairs) const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    PrimerParameterSetupPage *primerParameterSetupPage_;
    PrimerSearchingPage *primerSearchingPage_;
    PrimerResultsPage *primerResultsPage_;
};


/**
  * PrimerParameterSetupPage gathers the various input parameters used to generate novel primers
  */
class PrimerParameterSetupPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(BioString bioString READ bioString)

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerParameterSetupPage(const BioString &bioString, QWidget *parent = nullptr);
    PrimerParameterSetupPage(const BioString &bioString, const PrimerSearchParameters &primerSearchParameters, QWidget *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    BioString bioString() const;
    void setSearchRange(const ClosedIntRange &range);
    void initializePage();
    bool validatePage();
    PrimerSearchParameters parametersFromForm() const;              //!< Returns a PrimerSearchParameters by extracting data from the registered fields


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onSequenceTextViewSelectionFinished();
    void onFivePrimeForwardButtonReleased();
    void onFivePrimeReverseButtonReleased();
    void updateSearchLength();
    void updateSelectionStart();
    void updateSelectionStop();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void constructor_();
    QString rebaseFile() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    ClosedIntRange initialSearchRange_;
    BioString sourceSequence_;
    PrimerSearchParameters primerSearchParameters_;

    QSpinBox *sequenceSearchStartSpinBox_;
    QSpinBox *sequenceSearchStopSpinBox_;
    QLabel *searchLengthLabel_;
    SequenceTextView *sequenceTextView_;
    QSpinBox *ampliconRangeMinSpinBox_;
    QSpinBox *ampliconRangeMaxSpinBox_;
    QSpinBox *primerLengthMinSpinBox_;
    QSpinBox *primerLengthMaxSpinBox_;
    QDoubleSpinBox *tmRangeMinDoubleSpinBox_;
    QDoubleSpinBox *tmRangeMaxDoubleSpinBox_;
    QDoubleSpinBox *maximumDeltaTmDoubleSpinBox_;
    QSpinBox *saltMolarityDoubleSpinBox_;
    AgDoubleSpinBox *primerDnaMolarityDoubleSpinBox_;
    RestrictionEnzymeLineEdit *fivePrimeForwardRELineEdit_;
    RestrictionEnzymeLineEdit *fivePrimeReverseRELineEdit_;
    QToolButton *fivePrimeForwardButton_;
    QToolButton *fivePrimeReverseButton_;
    ThreePrimeInput *threePrimeForward_;
    ThreePrimeInput *threePrimeReverse_;
};


/**
  * PrimerSearchingPage displays the parameters used to search for compatible primers and a progress bar as the search
  * is performed.
  */
class PrimerSearchingPage : public QWizardPage
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit PrimerSearchingPage(PrimerParameterSetupPage *primerParameterSetupPage, QWidget *parent = nullptr);
    ~PrimerSearchingPage();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isActive() const;
    bool isComplete() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void cancelSearch();


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void initializePage();


private Q_SLOTS:
    void searchFinished(const PrimerPairVector &primerPairs);
    void stopButtonReleased();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString prefixSuffixLabel(const BioString &bioString) const;    //!< Returns a human friendly label for bioString for prefix or suffix labels; pure convenience method


    // ------------------------------------------------------------------------------------------------
    // Private members
    PrimerParameterSetupPage *primerParameterSetupPage_;
    SignalPrimerPairFinder *signalPrimerPairFinder_;
    QThread *thread_;
    bool active_;
    bool canceled_;
    bool complete_;
    QVector<PrimerPair> matchingPrimerPairs_;   //!< Vector of all found primer pairs

    // Widgets
    QLabel *stateLabel_;                    //!< Label above progress bar that indicates the current status (e.g. Processing..., Done, etc.)
    QPushButton *stopButton_;
    QProgressBar *progressBar_;

    QLabel *sequenceSearchSpaceLabel_;
    QLabel *ampliconRangeLabel_;
    QLabel *primerLengthRangeLabel_;
    QLabel *tmRangeLabel_;
    QLabel *maximumDeltaTmLabel_;
    QLabel *sodiumConcentrationLabel_;
    QLabel *primerDnaConcentrationLabel_;

    QLabel *fivePrimeForwardLabel_;
    QLabel *fivePrimeReverseLabel_;
    QLabel *threePrimeForwardLabel_;
    QLabel *threePrimeReverseLabel_;

    friend class PrimerResultsPage;
};


/**
  * PrimerResultsPage contains the results of a successful primer search that returned at least one primer matching the
  * user specified conditions.
  */
class PrimerResultsPage : public QWizardPage
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    PrimerResultsPage(PrimerSearchingPage *primerSearchingPage, QWidget *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isComplete() const;
    PrimerPairVector selectedPrimers() const;


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void initializePage();
    void cleanupPage();
    bool validatePage();


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onCurrentRowIndexChanged(const QModelIndex &currentRowIndex);
    void updateViewArrangement();   // Helper function called with single shot timer to balance the table view's look


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    PrimerSearchingPage *primerSearchingPage_;
    PrimerPairModel *primerPairModel_;
    FilterColumnProxyModel *sortFilterProxyModel_;
    QTableView *primerPairTableView_;
    SequenceTextView *sequenceTextView_;
    QSplitter *splitter_;
};

/**
  */
class PrimerNamePrefixPage : public QWizardPage
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit PrimerNamePrefixPage(QWidget *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void cleanupPage();
    void initializePage();
    bool isComplete() const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void updateExamplesLabelText();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QString namePrefix_;
    QLineEdit *namePrefixLineEdit_;
    QLabel *forwardExample1Label_;
    QLabel *reverseExample1Label_;
    QLabel *forwardExample2Label_;
    QLabel *reverseExample2Label_;
};

#endif // PRIMERCREATORWIZARD_H
