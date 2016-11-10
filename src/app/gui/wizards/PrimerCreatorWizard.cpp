/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QSplitter>
#include <QtGui/QStyle>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

#include "PrimerCreatorWizard.h"
#include "../models/FilterColumnProxyModel.h"
#include "../widgets/AgDoubleSpinBox.h"
#include "../widgets/SequenceTextView.h"
#include "../Services/DoubleSpinBoxRangeLinker.h"
#include "../Services/HeaderColumnSelector.h"
#include "../Services/SpinBoxRangeLinker.h"
#include "../../core/constants.h"
#include "../../core/macros.h"
#include "../../primer/PrimerPairFinder.h"      // For PrimerPairFinderResult
#include "../../primer/PrimerPairModel.h"
#include "../../primer/RestrictionEnzymeBrowserDialog.h"
#include "../../primer/RestrictionEnzymeLineEdit.h"
#include "../../primer/SignalPrimerPairFinder.h"
#include "../../primer/ThreePrimeInput.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param bioString [const BioString &]
  * @param parent [QWidget *]
  */
PrimerCreatorWizard::PrimerCreatorWizard(const BioString &bioString, QWidget *parent)
    : QWizard(parent)
{
    ASSERT(!bioString.isEmpty());

    primerParameterSetupPage_ = new PrimerParameterSetupPage(bioString);
    constructor_(primerParameterSetupPage_);
}

PrimerCreatorWizard::PrimerCreatorWizard(const BioString &bioString, const PrimerSearchParameters *primerSearchParameters, QWidget *parent)
    : QWizard(parent)
{
    ASSERT(!bioString.isEmpty());

    if (primerSearchParameters != nullptr)
        primerParameterSetupPage_ = new PrimerParameterSetupPage(bioString, *primerSearchParameters);
    else
        primerParameterSetupPage_ = new PrimerParameterSetupPage(bioString);

    constructor_(primerParameterSetupPage_);
}

/**
  * @param primerSearchParameters [const PrimerSearchParameters &]
  * @param parent [QWidget *]
  */
PrimerCreatorWizard::PrimerCreatorWizard(const BioString &bioString, const PrimerSearchParameters &primerSearchParameters, QWidget *parent)
    : QWizard(parent)
{
    ASSERT(!bioString.isEmpty());
    ASSERT(primerSearchParameters.isValid());

    primerParameterSetupPage_ = new PrimerParameterSetupPage(bioString, primerSearchParameters);
    constructor_(primerParameterSetupPage_);
}

PrimerPairVector PrimerCreatorWizard::generatedPrimerPairs() const
{
    PrimerPairVector primerPairs = primerResultsPage_->selectedPrimers();
    autoNamePrimerPairs(primerPairs);
    return primerPairs;
}

void PrimerCreatorWizard::setInitialSearchRange(const ClosedIntRange &range)
{
    primerParameterSetupPage_->setSearchRange(range);
}

/**
  */
void PrimerCreatorWizard::constructor_(PrimerParameterSetupPage *primerParameterSetupPage)
{
    setWindowTitle("New Primer Wizard");

    addPage(primerParameterSetupPage);
    primerSearchingPage_ = new PrimerSearchingPage(primerParameterSetupPage);
    addPage(primerSearchingPage_);
    primerResultsPage_ = new PrimerResultsPage(primerSearchingPage_);
    addPage(primerResultsPage_);
    addPage(new PrimerNamePrefixPage);
}

void PrimerCreatorWizard::autoNamePrimerPairs(PrimerPairVector &primerPairs) const
{
    QString namePrefix = field("namePrefix").toString();
    for (int i=0, z=primerPairs.size(); i<z; ++i)
    {
        PrimerPair &primerPair = primerPairs[i];
        primerPair.setForwardPrimerName(QString("%1 forward %2").arg(namePrefix).arg(i+1));
        primerPair.setReversePrimerName(QString("%1 reverse %2").arg(namePrefix).arg(i+1));
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * Check if a primer search is active and confirm that user really wants to cancel if this is the case.
  */
void PrimerCreatorWizard::reject()
{
    if (primerSearchingPage_->isActive())
    {
        QMessageBox msgBox(primerSearchingPage_);
        msgBox.setWindowTitle("Confirm Cancel");
        msgBox.setText("The primer search has not finished. Are you sure you want to cancel?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.button(QMessageBox::Yes)->setText("Yes, Cancel");
        msgBox.button(QMessageBox::No)->setText("Continue search");
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() == QMessageBox::No)
            return;

        // Stop the formatting
        primerSearchingPage_->cancelSearch();
    }

    QWizard::reject();
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param bioString [const BioString &]
  * @param parent [QWidget *]
  */
PrimerParameterSetupPage::PrimerParameterSetupPage(const BioString &bioString, QWidget *parent)
    : QWizardPage(parent),
      sourceSequence_(bioString)
{
    constructor_();
}

/**
  * @param bioString [const BioString &]
  * @param primerSearchParameters [const PrimerSearchParameters &]
  * @param parent [QWidget *]
  */
PrimerParameterSetupPage::PrimerParameterSetupPage(const BioString &bioString, const PrimerSearchParameters &primerSearchParameters, QWidget *parent)
    : QWizardPage(parent),
      sourceSequence_(bioString),
      primerSearchParameters_(primerSearchParameters)
{
    constructor_();
}

/**
  */
void PrimerParameterSetupPage::constructor_()
{
    setTitle("Parameters");
    setSubTitle("Please specify the primer search parameters using the form below and then push the Search button "
                "to begin the search.");

    sequenceSearchStartSpinBox_ = new QSpinBox;
    // Labeled fiveDigitWidth even though six digits are used in the calculation becuase empirical tests indicate
    // that 8 digits are required in the calculation to show 5 digits (spinbox). Clearly, this calculation is off base, but this
    // works for the time being.
    QFontMetrics fontMetrics(sequenceSearchStartSpinBox_->font());
    int fiveDigitWidth = fontMetrics.width("12345678") + 2 * qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    sequenceSearchStartSpinBox_->setFixedWidth(fiveDigitWidth);
    sequenceSearchStartSpinBox_->setAccelerated(true);
    sequenceSearchStopSpinBox_ = new QSpinBox;
    sequenceSearchStopSpinBox_->setFixedWidth(fiveDigitWidth);
    sequenceSearchStopSpinBox_->setAccelerated(true);
    searchLengthLabel_ = new QLabel("###");
    sequenceTextView_ = new SequenceTextView;
    sequenceTextView_->setFont(QFont("monospace"));
    ampliconRangeMinSpinBox_ = new QSpinBox;
    ampliconRangeMinSpinBox_->setFixedWidth(fiveDigitWidth);
    ampliconRangeMinSpinBox_->setAccelerated(true);
    ampliconRangeMaxSpinBox_ = new QSpinBox;
    ampliconRangeMaxSpinBox_->setFixedWidth(fiveDigitWidth);
    ampliconRangeMaxSpinBox_->setAccelerated(true);
    primerLengthMinSpinBox_ = new QSpinBox;
    primerLengthMinSpinBox_->setFixedWidth(fiveDigitWidth);
    primerLengthMinSpinBox_->setAccelerated(true);
    primerLengthMaxSpinBox_ = new QSpinBox;
    primerLengthMaxSpinBox_->setFixedWidth(fiveDigitWidth);
    primerLengthMaxSpinBox_->setAccelerated(true);
    tmRangeMinDoubleSpinBox_ = new QDoubleSpinBox;
    tmRangeMinDoubleSpinBox_->setFixedWidth(fiveDigitWidth);
    tmRangeMinDoubleSpinBox_->setDecimals(1);
    tmRangeMinDoubleSpinBox_->setAccelerated(true);
    tmRangeMaxDoubleSpinBox_ = new QDoubleSpinBox;
    tmRangeMaxDoubleSpinBox_->setDecimals(1);
    tmRangeMaxDoubleSpinBox_->setFixedWidth(fiveDigitWidth);
    tmRangeMaxDoubleSpinBox_->setAccelerated(true);

    maximumDeltaTmDoubleSpinBox_ = new QDoubleSpinBox;
    maximumDeltaTmDoubleSpinBox_->setDecimals(1);
    maximumDeltaTmDoubleSpinBox_->setFixedWidth(fiveDigitWidth);
    maximumDeltaTmDoubleSpinBox_->setAccelerated(true);

    saltMolarityDoubleSpinBox_ = new QSpinBox;
    saltMolarityDoubleSpinBox_->setFixedWidth(fiveDigitWidth);
    saltMolarityDoubleSpinBox_->setMinimum(1.);
    saltMolarityDoubleSpinBox_->setAccelerated(true);

    primerDnaMolarityDoubleSpinBox_ = new AgDoubleSpinBox;
    primerDnaMolarityDoubleSpinBox_->setFixedWidth(fiveDigitWidth);
    primerDnaMolarityDoubleSpinBox_->setVisibleDecimals(1);
    primerDnaMolarityDoubleSpinBox_->setMinimum(1.);
    primerDnaMolarityDoubleSpinBox_->setAccelerated(true);

    fivePrimeForwardRELineEdit_ = new RestrictionEnzymeLineEdit;
    fivePrimeForwardButton_ = new QToolButton;
    fivePrimeForwardButton_->setText("...");
    fivePrimeForwardButton_->setToolTip("Browse restriction enzymes");
    fivePrimeReverseRELineEdit_ = new RestrictionEnzymeLineEdit;
    fivePrimeReverseButton_ = new QToolButton;
    fivePrimeReverseButton_->setText("...");
    fivePrimeReverseButton_->setToolTip("Browse restriction enzymes");
    threePrimeForward_ = new ThreePrimeInput;
    threePrimeReverse_ = new ThreePrimeInput;

    new SpinBoxRangeLinker(sequenceSearchStartSpinBox_, sequenceSearchStopSpinBox_, sequenceSearchStartSpinBox_);
    new SpinBoxRangeLinker(primerLengthMinSpinBox_, primerLengthMaxSpinBox_, primerLengthMinSpinBox_);
    new SpinBoxRangeLinker(ampliconRangeMinSpinBox_, ampliconRangeMaxSpinBox_, ampliconRangeMinSpinBox_);
    new DoubleSpinBoxRangeLinker(tmRangeMinDoubleSpinBox_, tmRangeMaxDoubleSpinBox_, tmRangeMinDoubleSpinBox_);

    // -------
    // Signals
    connect(sequenceSearchStartSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateSearchLength()));
    connect(sequenceSearchStartSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateSelectionStart()));
    connect(sequenceSearchStopSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateSearchLength()));
    connect(sequenceSearchStopSpinBox_, SIGNAL(valueChanged(int)), SLOT(updateSelectionStop()));
    connect(sequenceTextView_, SIGNAL(selectionFinished()), SLOT(onSequenceTextViewSelectionFinished()));
    connect(fivePrimeForwardButton_, SIGNAL(released()), SLOT(onFivePrimeForwardButtonReleased()));
    connect(fivePrimeReverseButton_, SIGNAL(released()), SLOT(onFivePrimeReverseButtonReleased()));

    // ---------
    // Left pane

    // Search space horizontal layout
    QGridLayout *searchSpaceLayout = new QGridLayout;
    searchSpaceLayout->addWidget(new QLabel("Sequence Search Space:"),  1, 0, 1, 1);
    searchSpaceLayout->addWidget(new QLabel("Start:"),                  0, 1, 1, 1);
    searchSpaceLayout->addWidget(sequenceSearchStartSpinBox_,           1, 1, 1, 1);
    searchSpaceLayout->addWidget(new QLabel("to"),                      1, 2, 1, 1);
    searchSpaceLayout->addWidget(new QLabel("Stop:"),                   0, 3, 1, 1);
    searchSpaceLayout->addWidget(sequenceSearchStopSpinBox_,            1, 3, 1, 1);
    searchSpaceLayout->addItem(new QSpacerItem(10, 1),                  1, 4, 1, 1);
    searchSpaceLayout->addWidget(new QLabel("Length:"),                 1, 5, 1, 1);
    searchSpaceLayout->addWidget(searchLengthLabel_,                    1, 6, 1, 1);
    searchSpaceLayout->setColumnStretch(6, 1);

    // Left pane vertical layout
    QVBoxLayout *leftPaneLayout = new QVBoxLayout;
    leftPaneLayout->addLayout(searchSpaceLayout);
    leftPaneLayout->addWidget(sequenceTextView_);


    // ----------
    // Right pane
    QGridLayout *ampliconPrimerTmGridLayout = new QGridLayout;
    ampliconPrimerTmGridLayout->setColumnStretch(3, 1);

    // Row 0
    ampliconPrimerTmGridLayout->addWidget(new QLabel("Min:"),                         0, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("Max:"),                         0, 3, 1, 1);

    // Row 1
    ampliconPrimerTmGridLayout->addWidget(new QLabel("Amplicon Size Range:"),         1, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(ampliconRangeMinSpinBox_,                   1, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("to"),                           1, 2, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(ampliconRangeMaxSpinBox_,                   1, 3, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("bp"),                           1, 4, 1, 1);

    // Row 2
    ampliconPrimerTmGridLayout->addWidget(new QLabel("Primer Length Range:"),         2, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(primerLengthMinSpinBox_,                    2, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("to"),                           2, 2, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(primerLengthMaxSpinBox_,                    2, 3, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("bp"),                           2, 4, 1, 1);

    // Row 3
    QLabel *tmLabel = new QLabel("T<sub>m</sub> Range:");
    tmLabel->setToolTip("Melting temperature (degrees Celsius)");
    ampliconPrimerTmGridLayout->addWidget(tmLabel,                                    3, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(tmRangeMinDoubleSpinBox_,                   3, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("to"),                           3, 2, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(tmRangeMaxDoubleSpinBox_,                   3, 3, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("°C"),                           3, 4, 1, 1);

    // Row 4
    QLabel *maxDeltaTmLabel = new QLabel(QString::fromUtf8("Maximum Î”T<sub>m</sub>"));
    maxDeltaTmLabel->setToolTip("Maximum absolute difference in melting temperatures for a primer pair (degrees Celsius)");
    ampliconPrimerTmGridLayout->addWidget(maxDeltaTmLabel,                            4, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(maximumDeltaTmDoubleSpinBox_,               4, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("°C"),                           4, 2, 1, 1);

    // Row 5
    QLabel *sodiumLabel = new QLabel("[Na<sup>+</sup>]:");
    sodiumLabel->setToolTip("Sodium concentration (milliMolar)");
    ampliconPrimerTmGridLayout->addWidget(sodiumLabel,                                5, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(saltMolarityDoubleSpinBox_,                 5, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("mM"),                           5, 2, 1, 1);

    // Row 6
    QLabel *primerDnaConcLabel = new QLabel("[Primer DNA] (C<sub>T</sub>):");
    primerDnaConcLabel->setToolTip("Primer DNA concentration (microMolar)");
    ampliconPrimerTmGridLayout->addWidget(primerDnaConcLabel,                         6, 0, 1, 1, Qt::AlignRight);
    ampliconPrimerTmGridLayout->addWidget(primerDnaMolarityDoubleSpinBox_,            6, 1, 1, 1);
    ampliconPrimerTmGridLayout->addWidget(new QLabel("uM"),                           6, 2, 1, 1);

    QGroupBox *fivePrimeAdditionGroupBox = new QGroupBox("5' Additions");
    QGridLayout *fivePrimeAdditionLayout = new QGridLayout;
    fivePrimeAdditionLayout->addWidget(new QLabel("Forward:"),      0, 0, 1, 1, Qt::AlignRight);
    fivePrimeAdditionLayout->addWidget(fivePrimeForwardRELineEdit_,   0, 1, 1, 1);
    fivePrimeAdditionLayout->addWidget(fivePrimeForwardButton_,     0, 2, 1, 1);
    fivePrimeAdditionLayout->addWidget(new QLabel("Reverse:"),      1, 0, 1, 1, Qt::AlignRight);
    fivePrimeAdditionLayout->addWidget(fivePrimeReverseRELineEdit_,   1, 1, 1, 1);
    fivePrimeAdditionLayout->addWidget(fivePrimeReverseButton_,     1, 2, 1, 1);
    fivePrimeAdditionGroupBox->setLayout(fivePrimeAdditionLayout);

    QGroupBox *threePrimeSpecificationGroupBox = new QGroupBox("3' Specifications");
    QGridLayout *threePrimeSpecLayout = new QGridLayout;
    threePrimeSpecLayout->addWidget(new QLabel("Forward:"),      0, 0, 1, 1, Qt::AlignRight);
    threePrimeSpecLayout->addWidget(threePrimeForward_,          0, 1, 1, 1);
    threePrimeSpecLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding),    0, 2, 1, 1);
    threePrimeSpecLayout->addWidget(new QLabel("Reverse:"),      1, 0, 1, 1, Qt::AlignRight);
    threePrimeSpecLayout->addWidget(threePrimeReverse_,          1, 1, 1, 1);
    threePrimeSpecificationGroupBox->setLayout(threePrimeSpecLayout);

    // Vertical layout of the right pane
    QVBoxLayout *rightPaneLayout = new QVBoxLayout;
    rightPaneLayout->addLayout(ampliconPrimerTmGridLayout);
    rightPaneLayout->addWidget(fivePrimeAdditionGroupBox);
    rightPaneLayout->addWidget(threePrimeSpecificationGroupBox);
    rightPaneLayout->addSpacerItem(new QSpacerItem(1, 11, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // ----------------
    // Aggregate layout
    QHBoxLayout *compositeLayout = new QHBoxLayout;
    setLayout(compositeLayout);
    compositeLayout->addLayout(leftPaneLayout);
    compositeLayout->addSpacing(25);
    compositeLayout->addLayout(rightPaneLayout);
    compositeLayout->setStretch(0, 1);              // Make the left pane absorb any extra horizontal space

    // Expose the sequence as a custom property
    registerField("bioString", this, "bioString");

    registerField("searchStart", sequenceSearchStartSpinBox_);
    registerField("searchStop", sequenceSearchStopSpinBox_);
    registerField("ampliconRangeMin", ampliconRangeMinSpinBox_);
    registerField("ampliconRangeMax", ampliconRangeMaxSpinBox_);
    registerField("primerLengthMin", primerLengthMinSpinBox_);
    registerField("primerLengthMax", primerLengthMaxSpinBox_);
    registerField("tmRangeMin", tmRangeMinDoubleSpinBox_, "value");
    registerField("tmRangeMax", tmRangeMaxDoubleSpinBox_, "value");
    registerField("maximumDeltaTm", maximumDeltaTmDoubleSpinBox_, "value");
    registerField("saltMolarity", saltMolarityDoubleSpinBox_, "value");
    registerField("primerDnaMolarity", primerDnaMolarityDoubleSpinBox_, "value");
    registerField("fivePrimeForwardAddition", fivePrimeForwardRELineEdit_, "restrictionEnzyme");
    registerField("fivePrimeReverseAddition", fivePrimeReverseRELineEdit_, "restrictionEnzyme");
    registerField("threePrimeForwardText", threePrimeForward_, "text");
    registerField("threePrimeForwardPattern", threePrimeForward_, "dnaPattern");
    registerField("threePrimeReverseText", threePrimeReverse_, "text");
    registerField("threePrimeReversePattern", threePrimeReverse_, "dnaPattern");
}

/**
  * @returns QString
  */
QString PrimerParameterSetupPage::rebaseFile() const
{
    return qApp->applicationDirPath() + QDir::separator() + constants::kRestrictionEnzymeEmbossPath;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns BioString
  */
BioString PrimerParameterSetupPage::bioString() const
{
    return sourceSequence_;
}

void PrimerParameterSetupPage::setSearchRange(const ClosedIntRange &range)
{
    initialSearchRange_ = range;
}

void PrimerParameterSetupPage::initializePage()
{
    sequenceSearchStartSpinBox_->setMinimum(1);
    sequenceSearchStopSpinBox_->setMaximum(bioString().length());
    sequenceSearchStopSpinBox_->setValue(bioString().length());
    sequenceSearchStartSpinBox_->setValue(1);

    bool isValidRange = initialSearchRange_.begin_ > 0 &&
                        initialSearchRange_.begin_ <= initialSearchRange_.end_ &&
                        initialSearchRange_.end_ <= bioString().length();
    if (isValidRange)
    {
        sequenceSearchStopSpinBox_->setValue(initialSearchRange_.end_);
        sequenceSearchStartSpinBox_->setValue(initialSearchRange_.begin_);
    }

    sequenceTextView_->setSequence(bioString().asByteArray());

    primerLengthMinSpinBox_->setMinimum(5);
    primerLengthMaxSpinBox_->setMaximum(100);
    // Note: the order in which these are set is important! If the min spin box is set first, it can prevent
    // setting its default properly.
    primerLengthMaxSpinBox_->setValue(primerSearchParameters_.primerLengthRange_.end_);
    primerLengthMinSpinBox_->setValue(primerSearchParameters_.primerLengthRange_.begin_);

    ampliconRangeMinSpinBox_->setMinimum(1);
    ampliconRangeMaxSpinBox_->setMaximum(sequenceSearchStopSpinBox_->value() - sequenceSearchStartSpinBox_->value() + 1);
    ampliconRangeMaxSpinBox_->setValue(sequenceSearchStopSpinBox_->value());
    ampliconRangeMinSpinBox_->setValue(sequenceSearchStopSpinBox_->value() - 20);

    tmRangeMinDoubleSpinBox_->setMinimum(-100.);
    tmRangeMaxDoubleSpinBox_->setMaximum(500.);
    tmRangeMaxDoubleSpinBox_->setValue(primerSearchParameters_.individualPrimerTmRange_.end_);
    tmRangeMinDoubleSpinBox_->setValue(primerSearchParameters_.individualPrimerTmRange_.begin_);

    maximumDeltaTmDoubleSpinBox_->setMinimum(0.);
    maximumDeltaTmDoubleSpinBox_->setMaximum(100.);
    maximumDeltaTmDoubleSpinBox_->setValue(primerSearchParameters_.maximumPrimerPairDeltaTm_);

    // Convert from Molar to milliMolar
    saltMolarityDoubleSpinBox_->setMinimum(1);
    saltMolarityDoubleSpinBox_->setMaximum(1000.);
    saltMolarityDoubleSpinBox_->setValue(primerSearchParameters_.milliMolarSodiumConcentration());

    primerDnaMolarityDoubleSpinBox_->setMinimum(.001);
    primerDnaMolarityDoubleSpinBox_->setMaximum(1000.);
    primerDnaMolarityDoubleSpinBox_->setValue(primerSearchParameters_.microMolarDnaConcentration());

    fivePrimeForwardRELineEdit_->setRestrictionEnzyme(primerSearchParameters_.forwardRestrictionEnzyme_);
    fivePrimeReverseRELineEdit_->setRestrictionEnzyme(primerSearchParameters_.reverseRestrictionEnzyme_);

    threePrimeForward_->setPattern(primerSearchParameters_.forwardTerminalPattern_);
    threePrimeReverse_->setPattern(primerSearchParameters_.reverseTerminalPattern_);

    updateSelectionStart();
    updateSelectionStop();
}

bool PrimerParameterSetupPage::validatePage()
{
    PrimerSearchParameters primerSearchParameters = parametersFromForm();
    if (!primerSearchParameters.isValid())
    {
        // Technically, getting to this page should not occur because the input controls should limit input to only
        // those cases that are valid.
        QMessageBox::warning(this,
                             "Invalid parameter(s)",
                             primerSearchParameters.errorMessage(),
                             QMessageBox::Ok);
        return false;
    }

    return true;
}

/**
  * @returns PrimerSearchParameters
  */
PrimerSearchParameters PrimerParameterSetupPage::parametersFromForm() const
{
    // Build a PrimerSearchParameter object from the field data
    PrimerSearchParameters primerSearchParameters;
    primerSearchParameters.ampliconLengthRange_.begin_ = field("ampliconRangeMin").toInt();
    primerSearchParameters.ampliconLengthRange_.end_ = field("ampliconRangeMax").toInt();
    primerSearchParameters.primerLengthRange_.begin_ = field("primerLengthMin").toInt();
    primerSearchParameters.primerLengthRange_.end_ = field("primerLengthMax").toInt();
    primerSearchParameters.forwardRestrictionEnzyme_ = qvariant_cast<RestrictionEnzyme>(field("fivePrimeForwardAddition"));
    primerSearchParameters.forwardTerminalPattern_ = qvariant_cast<DnaPattern>(field("threePrimeForwardPattern"));
    primerSearchParameters.reverseRestrictionEnzyme_ = qvariant_cast<RestrictionEnzyme>(field("fivePrimeReverseAddition"));
    primerSearchParameters.reverseTerminalPattern_ = qvariant_cast<DnaPattern>(field("threePrimeReversePattern"));
    primerSearchParameters.individualPrimerTmRange_.begin_ = field("tmRangeMin").toDouble();
    primerSearchParameters.individualPrimerTmRange_.end_ = field("tmRangeMax").toDouble();
    primerSearchParameters.maximumPrimerPairDeltaTm_ = field("maximumDeltaTm").toDouble();
    primerSearchParameters.setSodiumConcentrationFromMilliMoles(field("saltMolarity").toDouble());
    primerSearchParameters.setPrimerDnaConcentrationFromMicroMoles(field("primerDnaMolarity").toDouble());
    return primerSearchParameters;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void PrimerParameterSetupPage::onSequenceTextViewSelectionFinished()
{
    Selection *selection = sequenceTextView_->currentSelection();
    bool isValidSelection = selection != nullptr &&
                            selection->start_ + 1 > 0 &&
                            selection->start_ <= selection->stop_ &&
                            selection->stop_ + 1 <= bioString().length();
    if (isValidSelection)
    {
        sequenceSearchStartSpinBox_->setValue(selection->start_ + 1);
        sequenceSearchStopSpinBox_->setValue(selection->stop_ + 1);

        // Note: order of setting these values is critical
        int selectionLength = selection->stop_ - selection->start_ + 1;
        ampliconRangeMaxSpinBox_->setValue(selectionLength);
        ampliconRangeMinSpinBox_->setValue(selectionLength);
    }
}

/**
  */
void PrimerParameterSetupPage::onFivePrimeForwardButtonReleased()
{
    RestrictionEnzymeBrowserDialog dialog(this);
    dialog.setWindowTitle("Select Forward Restriction Enzyme");
    dialog.setRebaseFile(rebaseFile());

    if (dialog.exec())
        fivePrimeForwardRELineEdit_->setRestrictionEnzyme(dialog.selectedRestrictionEnzyme());
}

/**
  */
void PrimerParameterSetupPage::onFivePrimeReverseButtonReleased()
{
    RestrictionEnzymeBrowserDialog dialog(this);
    dialog.setWindowTitle("Select Reverse Restriction Enzyme");
    dialog.setRebaseFile(rebaseFile());

    if (dialog.exec())
        fivePrimeReverseRELineEdit_->setRestrictionEnzyme(dialog.selectedRestrictionEnzyme());
}

/**
  */
void PrimerParameterSetupPage::updateSearchLength()
{
    int searchLength = sequenceSearchStopSpinBox_->value() - sequenceSearchStartSpinBox_->value() + 1;
    searchLengthLabel_->setText(QString::number(searchLength));

    // Now update the amplicon range
    if (ampliconRangeMinSpinBox_->value() > searchLength)
        ampliconRangeMinSpinBox_->setValue(searchLength);

    ampliconRangeMaxSpinBox_->setMaximum(searchLength);
}

/**
  */
void PrimerParameterSetupPage::updateSelectionStart()
{
    Selection *selection = sequenceTextView_->currentSelection();
    if (selection != nullptr)
    {
        if (selection->start_ != sequenceSearchStartSpinBox_->value() - 1)
        {
            selection->start_ = sequenceSearchStartSpinBox_->value() - 1;
            sequenceTextView_->viewport()->update();
        }
    }
    else
    {
        sequenceTextView_->addSelection(sequenceSearchStartSpinBox_->value() - 1, sequenceSearchStopSpinBox_->value() - 1);
    }
}

/**
  */
void PrimerParameterSetupPage::updateSelectionStop()
{
    Selection *selection = sequenceTextView_->currentSelection();
    if (selection != nullptr)
    {
        if (selection->stop_ != sequenceSearchStopSpinBox_->value() - 1)
        {
            selection->stop_ = sequenceSearchStopSpinBox_->value() - 1;
            sequenceTextView_->viewport()->update();
        }
    }
    else
    {
        sequenceTextView_->addSelection(sequenceSearchStartSpinBox_->value() - 1, sequenceSearchStopSpinBox_->value() - 1);
    }
}




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param parent [QWidget *]
  */
PrimerSearchingPage::PrimerSearchingPage(PrimerParameterSetupPage *primerParameterSetupPage, QWidget *parent)
    : QWizardPage(parent),
      primerParameterSetupPage_(primerParameterSetupPage),
      signalPrimerPairFinder_(nullptr),
      thread_(nullptr),
      active_(false),
      canceled_(false),
      complete_(false)
{
    ASSERT(primerParameterSetupPage != nullptr);

    setTitle("Generating Primers");
    setSubTitle("This process may take a few seconds to a few minutes depending on the search configuration. To stop "
                "the process and view the currently found primers, press the Stop button. Otherwise, please wait until "
                "the search completes.");

    // Widget setup - no parents are defined here, because they are added to the layout which reparents them
    stateLabel_ = new QLabel;
    stopButton_ = new QPushButton("Stop");
    progressBar_ = new QProgressBar;
    progressBar_->setTextVisible(false);

    sequenceSearchSpaceLabel_ = new QLabel;
    ampliconRangeLabel_ = new QLabel;
    primerLengthRangeLabel_ = new QLabel;
    tmRangeLabel_ = new QLabel;
    maximumDeltaTmLabel_ = new QLabel;
    sodiumConcentrationLabel_ = new QLabel;
    primerDnaConcentrationLabel_ = new QLabel;
    fivePrimeForwardLabel_ = new QLabel;
    fivePrimeReverseLabel_ = new QLabel;
    threePrimeForwardLabel_ = new QLabel;
    threePrimeReverseLabel_ = new QLabel;

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addWidget(stateLabel_,                 0, 0, 1, 1);
    topLayout->addWidget(stopButton_,                 0, 1, 1, 1, Qt::AlignRight);
    topLayout->addWidget(progressBar_,                1, 0, 1, 2);

    QGridLayout *basicParameterLayout = new QGridLayout;
    basicParameterLayout->setColumnStretch(1, 1);
    basicParameterLayout->addWidget(new QLabel("Sequence Search Space:"),   0, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(sequenceSearchSpaceLabel_,              0, 1, 1, 1);

    basicParameterLayout->addWidget(new QLabel("Amplicon Size Range (bp):"),1, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(ampliconRangeLabel_,                    1, 1, 1, 1);

    basicParameterLayout->addWidget(new QLabel("Primer Length Range (bp):"),2, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(primerLengthRangeLabel_,                2, 1, 1, 1);

    QLabel *tmLabel = new QLabel("T<sub>m</sub> Range (°C):");
    tmLabel->setToolTip("Melting temperature (degrees Celsius)");
    basicParameterLayout->addWidget(tmLabel,                                3, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(tmRangeLabel_,                          3, 1, 1, 1);

    QLabel *maxDeltaTmLabel = new QLabel(QString::fromUtf8("Maximum Î”T<sub>m</sub>:"));
    maximumDeltaTmLabel_->setToolTip("Maximum absolute difference in melting temperatures for a primer pair (degrees Celsius)");
    basicParameterLayout->addWidget(maxDeltaTmLabel,                        4, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(maximumDeltaTmLabel_,                   4, 1, 1, 1);

    QLabel *sodiumLabel = new QLabel("[Na<sup>+</sup>] (mM):");
    sodiumLabel->setToolTip("Sodium concentration (milliMolar)");
    basicParameterLayout->addWidget(sodiumLabel,                            5, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(sodiumConcentrationLabel_,              5, 1, 1, 1);

    QLabel *primerConcLabel = new QLabel("[Primer DNA] (C<sub>T</sub>):");
    primerConcLabel->setToolTip("Primer DNA concentration (microMolar)");
    basicParameterLayout->addWidget(primerConcLabel,                        6, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(primerDnaConcentrationLabel_,           6, 1, 1, 1);

    basicParameterLayout->addItem(new QSpacerItem(1, 10), 7, 0, 1, 2);
    basicParameterLayout->addWidget(new QLabel("<em>5' Additions:</em>"),   8, 0, 1, 1);
    basicParameterLayout->addWidget(new QLabel("Forward:"),                 9, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(fivePrimeForwardLabel_,                 9, 1, 1, 1);
    basicParameterLayout->addWidget(new QLabel("Reverse:"),                 10, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(fivePrimeReverseLabel_,                 10, 1, 1, 1);

    basicParameterLayout->addItem(new QSpacerItem(1, 10), 11, 0, 1, 2);
    basicParameterLayout->addWidget(new QLabel("<em>3' Specifications:</em>"), 12, 0, 1, 1);
    basicParameterLayout->addWidget(new QLabel("Forward:"),                 13, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(threePrimeForwardLabel_,                13, 1, 1, 1);
    basicParameterLayout->addWidget(new QLabel("Reverse:"),                 14, 0, 1, 1, Qt::AlignRight);
    basicParameterLayout->addWidget(threePrimeReverseLabel_,                14, 1, 1, 1);

    QVBoxLayout *compositeLayout = new QVBoxLayout;
    setLayout(compositeLayout);
    compositeLayout->addLayout(topLayout);
    compositeLayout->addSpacing(10);
    compositeLayout->addWidget(new QLabel("<strong>Parameters</strong>"));
    compositeLayout->addLayout(basicParameterLayout);

    // Signals
    connect(stopButton_, SIGNAL(released()), SLOT(stopButtonReleased()));
}

/**
  */
PrimerSearchingPage::~PrimerSearchingPage()
{
    if (thread_ == nullptr)
        return;

    if (thread_->isRunning())
        thread_->quit();

    thread_->wait(1000);
    if (thread_->isRunning())
        thread_->terminate();
}

/**
  * @returns bool
  */
bool PrimerSearchingPage::isActive() const
{
    return active_;
}

/**
  * @returns bool
  */
bool PrimerSearchingPage::isComplete() const
{
    return complete_;
}

/**
  */
void PrimerSearchingPage::cancelSearch()
{
    if (!active_)
        return;

    canceled_ = true;
    ASSERT(signalPrimerPairFinder_ != nullptr);
    signalPrimerPairFinder_->cancel();

    // Progress will continue with onSearchFinished() slot
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void PrimerSearchingPage::initializePage()
{
    PrimerSearchParameters primerSearchParameters = primerParameterSetupPage_->parametersFromForm();
    ASSERT(primerSearchParameters.isValid());

    // Find the range
    ClosedIntRange range(field("searchStart").toInt(), field("searchStop").toInt());


    // Reset the form details
    matchingPrimerPairs_.clear();
    canceled_ = false;
    complete_ = false;
    stopButton_->setEnabled(true);
    progressBar_->setRange(0, 0);       // To make it a busy indicator

    stateLabel_->setText("Processing...");

    // Update the parameters
    QString sequenceSearchString = QString("%1 .. %2")
                                   .arg(range.begin_)
                                   .arg(range.end_);
    if (range.begin_ != 1)
        sequenceSearchString += QString(" [%3 bp]").arg(range.length());
    sequenceSearchSpaceLabel_->setText(sequenceSearchString);
    ampliconRangeLabel_->setText(QString("%1 .. %2")
                                 .arg(primerSearchParameters.ampliconLengthRange_.begin_)
                                 .arg(primerSearchParameters.ampliconLengthRange_.end_));
    primerLengthRangeLabel_->setText(QString("%1 .. %2")
                                     .arg(primerSearchParameters.primerLengthRange_.begin_)
                                     .arg(primerSearchParameters.primerLengthRange_.end_));
    tmRangeLabel_->setText(QString("%1 .. %2")
                           .arg(QString::number(primerSearchParameters.individualPrimerTmRange_.begin_, 'f', 1))
                           .arg(QString::number(primerSearchParameters.individualPrimerTmRange_.end_, 'f', 1)));
    maximumDeltaTmLabel_->setText(QString::number(primerSearchParameters.maximumPrimerPairDeltaTm_, 'f', 1));
    sodiumConcentrationLabel_->setText(QString::number(primerSearchParameters.milliMolarSodiumConcentration(), 'f', 1));
    primerDnaConcentrationLabel_->setText(QString::number(primerSearchParameters.microMolarDnaConcentration(), 'f', 1));

    RestrictionEnzyme forwardRestrictionEnzyme = qvariant_cast<RestrictionEnzyme>(field("fivePrimeForwardAddition"));
    RestrictionEnzyme reverseRestrictionEnzyme = qvariant_cast<RestrictionEnzyme>(field("fivePrimeReverseAddition"));
    fivePrimeForwardLabel_->setText(prefixSuffixLabel(forwardRestrictionEnzyme.recognitionSite()));
    fivePrimeReverseLabel_->setText(prefixSuffixLabel(reverseRestrictionEnzyme.recognitionSite()));

    threePrimeForwardLabel_->setText(field("threePrimeForwardText").toString());
    threePrimeReverseLabel_->setText(field("threePrimeReverseText").toString());

    // Lazy initialization of the thread and signal primer pair finder
    if (thread_ == nullptr)
    {
        thread_ = new QThread(this);
        thread_->start();
    }

    if (signalPrimerPairFinder_ == nullptr)
    {
        signalPrimerPairFinder_ = new SignalPrimerPairFinder;
        connect(signalPrimerPairFinder_, SIGNAL(finished(PrimerPairVector)), SLOT(searchFinished(PrimerPairVector)));
        signalPrimerPairFinder_->moveToThread(thread_);
    }

    // Invoke the slot in the other thread
    active_ = true;
    QMetaObject::invokeMethod(signalPrimerPairFinder_,
                              "findPrimerPairs",
                              Q_ARG(BioString, qvariant_cast<BioString>(field("bioString"))),
                              Q_ARG(ClosedIntRange, range),
                              Q_ARG(PrimerSearchParameters, primerSearchParameters));

}

bool ascendingScore(const PrimerPair &primerPair1, const PrimerPair &primerPair2)
{
    return primerPair1.score() < primerPair2.score();
}

/**
  * @param primerPairs [const PrimerPairVector &]
  */
void PrimerSearchingPage::searchFinished(const PrimerPairVector &primerPairs)
{
    active_ = false;
    progressBar_->setMaximum(100);        // To stop the progress bar
    stopButton_->setEnabled(false);

    // Do nothing if the search was canceled
    if (canceled_)
    {
        stateLabel_->setText("Canceled");
        return;
    }

    stateLabel_->setText("Finished");
    progressBar_->setValue(100);    // To show that the progress is complete

    /*
    if (primerPairFinderResult.isError)
    {
        QMessageBox::warning(this,
                             "Primer Search Error",
                             primerPairFinderResult.errorMessage,
                             QMessageBox::Ok);
        wizard()->back();
        return;
    }
    */
    if (primerPairs.isEmpty())
    {
        QMessageBox::information(this,
                                 "Primer Search Message",
                                 "No suitable primers were found that match the given criteria. Please adjust the "
                                 "parameters and try again.",
                                 QMessageBox::Ok);
        wizard()->back();
        return;
    }

    matchingPrimerPairs_ = primerPairs;

    // Take only the top 10
    qStableSort(matchingPrimerPairs_.begin(), matchingPrimerPairs_.end(), ascendingScore);
    matchingPrimerPairs_ = matchingPrimerPairs_.mid(0, 10);

    complete_ = true;
    emit completeChanged();
    wizard()->next();
}

/**
  */
void PrimerSearchingPage::stopButtonReleased()
{
    stateLabel_->setText("Stopping...");
    signalPrimerPairFinder_->cancel();
    stopButton_->setEnabled(false);

    // Progress will continue with onSearchFinished() slot
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param bioString [const BioString &]
  * @returns BioString
  */
QString PrimerSearchingPage::prefixSuffixLabel(const BioString &bioString) const
{
    if (bioString.isEmpty())
        return "(none)";

    return bioString.asByteArray();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param parent [QWidget *]
  */
PrimerResultsPage::PrimerResultsPage(PrimerSearchingPage *primerSearchingPage, QWidget *parent)
    : QWizardPage(parent),
      primerSearchingPage_(primerSearchingPage)
{
    ASSERT(primerSearchingPage_ != nullptr);

    setTitle("Matching Primers");
    setSubTitle("Select one or more primers from the table below to continue. The corresponding primer pair will be "
                "displayed in the sequence window as it is selected.");

    // Model setup
    primerPairModel_ = new PrimerPairModel(this);
    sortFilterProxyModel_ = new FilterColumnProxyModel(this);
    sortFilterProxyModel_->setSourceModel(primerPairModel_);

    // Prevent any visual access to this column (even by right-clicking on header and selecting the column there)
    sortFilterProxyModel_->excludeColumn(PrimerPairModel::eNameColumn);

    // Widget setup
    primerPairTableView_ = new QTableView;
    primerPairTableView_->setSortingEnabled(true);
    primerPairTableView_->verticalHeader()->hide();
    primerPairTableView_->setAlternatingRowColors(true);
    primerPairTableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    primerPairTableView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    primerPairTableView_->setModel(sortFilterProxyModel_);
    primerPairTableView_->hideColumn(sortFilterProxyModel_->mapFromSource(PrimerPairModel::eForwardPrimerSequenceColumn));
    primerPairTableView_->hideColumn(sortFilterProxyModel_->mapFromSource(PrimerPairModel::eReversePrimerSequenceColumn));
    primerPairTableView_->hideColumn(sortFilterProxyModel_->mapFromSource(PrimerPairModel::eForwardTmColumn));
    primerPairTableView_->hideColumn(sortFilterProxyModel_->mapFromSource(PrimerPairModel::eReverseTmColumn));
    primerPairTableView_->horizontalHeader()->setHighlightSections(false);

    connect(primerPairTableView_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(completeChanged()));
    connect(primerPairTableView_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowIndexChanged(QModelIndex)));

    new HeaderColumnSelector(primerPairTableView_->horizontalHeader(), primerPairTableView_);

    sequenceTextView_ = new SequenceTextView;
    sequenceTextView_->setFont(QFont("monospace"));
    sequenceTextView_->setAllowSelect(false);

    splitter_ = new QSplitter;
    splitter_->setChildrenCollapsible(false);
    splitter_->setOrientation(Qt::Vertical);
    splitter_->addWidget(primerPairTableView_);
    splitter_->addWidget(sequenceTextView_);

    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    layout->addWidget(splitter_);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool PrimerResultsPage::isComplete() const
{
    return primerPairTableView_->selectionModel()->selection().size() > 0;
}

PrimerPairVector PrimerResultsPage::selectedPrimers() const
{
    PrimerPairVector primerPairs;
    QModelIndexList selectedRowIndices = primerPairTableView_->selectionModel()->selectedRows();
    foreach (const QModelIndex &selectedRowIndex, selectedRowIndices)
    {
        QModelIndex sourceIndex = sortFilterProxyModel_->mapToSource(selectedRowIndex);
        primerPairs << primerPairModel_->primerPairFromIndex(sourceIndex);
    }
    return primerPairs;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void PrimerResultsPage::initializePage()
{
    BioString bioString = qvariant_cast<BioString>(field("bioString"));
    ASSERT(primerSearchingPage_->matchingPrimerPairs_.size() > 0);
    primerPairModel_->setBioString(bioString);
    primerPairModel_->setPrimerPairs(primerSearchingPage_->matchingPrimerPairs_);
    primerPairTableView_->sortByColumn(sortFilterProxyModel_->mapFromSource(PrimerPairModel::eScoreColumn), Qt::AscendingOrder);

    sequenceTextView_->setSequence(bioString.asByteArray());
    primerPairTableView_->selectRow(0);

    QTimer::singleShot(0, this, SLOT(updateViewArrangement()));

    // Make the CustomButton1 restart the process
//    wizard()->setOption(QWizard::HaveCustomButton1);
//    wizard()->setButtonText(QWizard::CustomButton1, "&Restart");
//    connect(wizard()->button(QWizard::CustomButton1), SIGNAL(released()), wizard(), SLOT(restart()), Qt::UniqueConnection);
}

/**
  */
void PrimerResultsPage::cleanupPage()
{
    primerPairModel_->clear();
//    wizard()->setOption(QWizard::HaveCustomButton1, false);
}

bool PrimerResultsPage::validatePage()
{
//    wizard()->setOption(QWizard::HaveCustomButton1, false);

    return true;
}


// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param currentRowIndex [const QModelIndex &]
  */
void PrimerResultsPage::onCurrentRowIndexChanged(const QModelIndex &currentRowIndex)
{
    sequenceTextView_->clearSelections();

    const PrimerPair &currentPrimerPair = primerPairModel_->primerPairFromIndex(currentRowIndex);

    BioString sequence = sequenceTextView_->sequence().toAscii();
    ClosedIntRange forwardPrimerLocation = currentPrimerPair.forwardPrimer().locateCoreSequenceIn(sequence);
    sequenceTextView_->addSelection(forwardPrimerLocation);

    ClosedIntRange reversePrimerLocation = currentPrimerPair.reversePrimer().locateCoreSequenceInCognateStrand(sequence);
    Selection reverseSelection(reversePrimerLocation);
    reverseSelection.color_ = QColor("#000");
    reverseSelection.backColor_ = QColor("#AAE0B3");
    sequenceTextView_->addSelection(reverseSelection);
}

/**
  */
void PrimerResultsPage::updateViewArrangement()
{
    primerPairTableView_->resizeColumnsToContents();
    primerPairTableView_->resizeRowsToContents();
    // Swine attempt to get a more balanced look of columns; stretch the primer pair column, get this width, and reset
    // to interactive, and then specify the width of the primer pair column when it was stretched.

    int mappedCombinedPrimerSequencesColumn = sortFilterProxyModel_->mapFromSource(PrimerPairModel::eCombinedPrimerSequencesColumn);
    primerPairTableView_->horizontalHeader()->setResizeMode(mappedCombinedPrimerSequencesColumn, QHeaderView::Stretch);
    int width = primerPairTableView_->horizontalHeader()->sectionSize(mappedCombinedPrimerSequencesColumn);
    primerPairTableView_->horizontalHeader()->setResizeMode(mappedCombinedPrimerSequencesColumn, QHeaderView::Interactive);
    primerPairTableView_->horizontalHeader()->resizeSection(mappedCombinedPrimerSequencesColumn, width);
}






// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
PrimerNamePrefixPage::PrimerNamePrefixPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Name primers and finish");
    setSubTitle("Optionally provide a prefix to be applied to the names of each of the selected primers. After this "
                "wizard completes you may rename them from within the Primers table.");

    QHBoxLayout *horizLayout = new QHBoxLayout;
    QLabel *namePrefixLabel = new QLabel("Common primer name prefix:");
    horizLayout->addWidget(namePrefixLabel);
    namePrefixLineEdit_ = new QLineEdit;
    namePrefixLineEdit_->setText("New Primer");
    connect(namePrefixLineEdit_, SIGNAL(textChanged(QString)), SLOT(updateExamplesLabelText()));
    horizLayout->addWidget(namePrefixLineEdit_);
    horizLayout->addStretch(1);

    QVBoxLayout *vertLayout = new QVBoxLayout;
    setLayout(vertLayout);
    vertLayout->addLayout(horizLayout);
    vertLayout->addWidget(new QLabel("Examples:"));
    forwardExample1Label_ = new QLabel;
    vertLayout->addWidget(forwardExample1Label_);
    reverseExample1Label_ = new QLabel;
    vertLayout->addWidget(reverseExample1Label_);
    forwardExample2Label_ = new QLabel;
    vertLayout->addWidget(forwardExample2Label_);
    reverseExample2Label_ = new QLabel;
    vertLayout->addWidget(reverseExample2Label_);
    vertLayout->addWidget(new QLabel("..."));

    updateExamplesLabelText();

    registerField("namePrefix", namePrefixLineEdit_);
}

void PrimerNamePrefixPage::cleanupPage()
{
    // Defined so as to prevent the wizard from resetting the line edit to its original value. Just in case, the
    // user has already changed the name prefix but wanted to look over the primer list again.
}

void PrimerNamePrefixPage::initializePage()
{
    namePrefixLineEdit_->selectAll();

    // On the Mac, the focus does not automatically go to this control...
    namePrefixLineEdit_->setFocus();
}

bool PrimerNamePrefixPage::isComplete() const
{
    return !namePrefixLineEdit_->text().trimmed().isEmpty();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void PrimerNamePrefixPage::updateExamplesLabelText()
{
    QString namePrefix = namePrefixLineEdit_->text().trimmed();

    forwardExample1Label_->setText(namePrefix + " forward 1");
    reverseExample1Label_->setText(namePrefix + " reverse 1");
    forwardExample2Label_->setText(namePrefix + " forward 2");
    reverseExample2Label_->setText(namePrefix + " reverse 2");
}
