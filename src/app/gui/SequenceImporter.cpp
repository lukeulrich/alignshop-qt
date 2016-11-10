/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtCore/QVector>

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

#include "SequenceImporter.h"

#include "forms/SelectGroupNodeDialog.h"
#include "models/AdocTreeModel.h"
#include "../core/Detectors/AlphabetDetector.h"
#include "../core/Detectors/ConsensusAlphabetDetector.h"
#include "../core/Entities/AbstractMsa.h"
#include "../core/Parsers/FastaParser.h"
#include "../core/Parsers/ClustalParser.h"
#include "../core/Parsers/SignalSequenceParser.h"
#include "../core/Repositories/IRepository.h"
#include "../core/Services/PodEntityService.h"
#include "../core/Adoc.h"
#include "../core/AdocTreeNode.h"
#include "../core/BioStringValidator.h"
#include "../core/constants.h"
#include "../core/macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
SequenceImporter::SequenceImporter(QWidget *parent) : QObject(parent)
{
    parseProgressDialog_ = new QProgressDialog(parent);
    parseProgressDialog_->setMinimumDuration(1500);
    // ISSUE
    // Cannot make progress dialog modal because it will overflow the event queue when processEvents is called (at
    // least on windows); however, this also means that users can interact with the other parts of the program while
    // the import operation is underway. Would like a means for preventing this.
    // parseProgressDialog_->setWindowModality(Qt::WindowModal);
    parseProgressDialog_->setWindowTitle("Import file");

    AlphabetDetector *alphabetDetector = new AlphabetDetector(this);
    alphabetDetector->setAlphabets(constants::kStandardAlphabetVector);
    alphabetDetector_ = alphabetDetector;

    dataFormats_ << DataFormat(eFastaFormat,
                               "Fasta",
                               QStringList() << "fa" << "faa" << "fnt" << "fasta",
                               new FastaParser(this));

    dataFormats_ << DataFormat(eClustalFormat,
                               "Clustal",
                               QStringList() << "aln" << "clustal",
                               new ClustalParser(this));

    dataFormatDetector_.setDataFormats(dataFormats_);
}

/**
  */
SequenceImporter::~SequenceImporter()
{
    if (thread_.isRunning())
    {
        QMetaObject::invokeMethod(&thread_, "quit");

        // Give it a max of 30 seconds to complete and terminate it if it is not finished by then
        if (!thread_.wait(30))
            thread_.terminate();
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<DataFormat>
  */
QVector<DataFormat> SequenceImporter::dataFormats() const
{
    return dataFormats_;
}

/**
  * ISSUE: Because this method uses threads and temporary variables (adoc, ...) per thread execution, it should not be
  * called again until all other invocations have completed.
  *
  * @param fileName [const QString &]
  * @param adoc [Adoc *]
  * @param adocTreeModel [AdocTreeModel *]
  * @param destination [const QModelIndex &]
  * @throws QString
  */
void SequenceImporter::importFile(const QString &fileName, Adoc *adoc, AdocTreeModel *adocTreeModel, const QModelIndex &destination)
{
    ASSERT(thread_.isRunning() == false);

    QFile file(fileName);
    ASSERT(file.exists());
    ASSERT(adoc != nullptr);
    ASSERT(adocTreeModel != nullptr);

    // ----------------------------------------------------
    // o Determine the dataformat
    DataFormat dataFormat = dataFormatDetector_.formatFromFile(file);
    if (dataFormat == DataFormat())
        // Did not recognize the data format
        // TODO: ask the user for a specific data format
        throw QString("Unrecognized data format");

    // ----------------------------------------------------
    // o Parse the file (and have a progress dialog)
    parseProgressDialog_->setLabelText(QString("Importing %1...").arg(QFileInfo(fileName).fileName()));
    ISequenceParser *parser = dataFormat.parser();
    ASSERT(parser != nullptr);

    // Save state variables
    importContext_.importFileName_ = fileName;
    importContext_.adoc_ = adoc;
    importContext_.adocTreeModel_ = adocTreeModel;
    importContext_.destination_ = destination;

    // Begin parsing in separate thread
    SignalSequenceParser *signalParser = new SignalSequenceParser(parser->clone());
    signalParser->moveToThread(&thread_);

    // It is vital to use a direct connection when cancel is called! Essentially we are calling a pointer from a
    // separate thread.
    connect(parseProgressDialog_, SIGNAL(canceled()), signalParser, SLOT(cancel()), Qt::DirectConnection);
    connect(signalParser, SIGNAL(progressChanged(int,int)), SLOT(onParserProgressChanged(int,int)));
    connect(signalParser, SIGNAL(parseSuccess(SequenceParseResultPod)), SLOT(onParseSuccess(SequenceParseResultPod)));
    connect(signalParser, SIGNAL(parseError(QString)), SIGNAL(importError(QString)));
    connect(signalParser, SIGNAL(parseOver()), signalParser, SLOT(deleteLater()));
    connect(signalParser, SIGNAL(parseOver()), &thread_, SLOT(quit()));
    connect(signalParser, SIGNAL(parseOver()), parseProgressDialog_, SLOT(reset()));
    thread_.start();

    // Call the parse file method - use a queued connection so that it crosses threads
    QMetaObject::invokeMethod(signalParser, "parseFile", Qt::QueuedConnection, Q_ARG(QString, fileName));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param parsePod [SequenceParseResultPod]
  */
void SequenceImporter::onParseSuccess(SequenceParseResultPod parsePod)
{
    parseProgressDialog_->reset();
    upperCaseSequenceData(parsePod);

    // ----------------------------------------------------
    // o Determine the group grammar
    ConsensusAlphabetDetector<QVector<SimpleSeqPod> > consensusAlphabetDetector(alphabetDetector_);
    Alphabet consensusAlphabet = consensusAlphabetDetector.detectConsensusAlphabet(parsePod.simpleSeqPods_, 50);
    if (consensusAlphabet == Alphabet())
    {
        // Failed to automatically determine the sequence alphabet
        consensusAlphabet = askUserForAlphabet();
        if (consensusAlphabet == Alphabet())
        {
            emit importCanceled();
            return;
        }
    }

    // ----------------------------------------------------
    // o Validate each of the sequences
    Alphabet ambiguousAlphabet = ::ambiguousAlphabetFromGrammar(consensusAlphabet.grammar());
    ASSERT(ambiguousAlphabet.grammar() != eUnknownGrammar);
    parsePod.grammar_ = ambiguousAlphabet.grammar();

    bool oneOrMoreInvalid = false;
    BioStringValidator validator(ambiguousAlphabet.allCharacters() + constants::kGapCharacters);
    QVector<SimpleSeqPod>::Iterator it = parsePod.simpleSeqPods_.begin();
    for (; it != parsePod.simpleSeqPods_.end(); ++it)
    {
        SimpleSeqPod &pod = *it;

        // Sequence must have the right characters and contain at least one non-gap character
        pod.isValid_ = validator.isValid(pod.sequence_) && pod.sequence_.hasNonGaps();
        if (pod.isValid_)
        {
            pod.sequence_.setGrammar(parsePod.grammar_);
            continue;
        }

        oneOrMoreInvalid = true;
        break;
    }
    if (oneOrMoreInvalid)
    {
        // TODO: Present dialog to user asking what to do with the one or more invalid entries
        // Unable to determine the alphabet
        emit importError("One or more sequences are invalid. Please remove any invalid characters from your data "
                         "file and try again.");
        return;
    }

    // ----------------------------------------------------
    // o Is this an alignment? If all the sequences have identical lengths, then assume so?
    if (parsePod.isAlignment_ == eUnknown &&
            SimpleSeqPod::identicalSequenceLengths(parsePod.simpleSeqPods_) &&
            parsePod.simpleSeqPods_.size() > 1)
    {
        QMessageBox msgBox(qobject_cast<QWidget *>(parent()));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Import as alignment?");
        msgBox.setText("All sequences have identical lengths. Is this an alignment?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Yes)
            parsePod.isAlignment_ = eTrue;
    }

    // ----------------------------------------------------
    // o Get the import destination
    SelectGroupNodeDialog selectGroupNodeDialog(static_cast<QWidget *>(parent()));
    selectGroupNodeDialog.setAdocTreeModel(importContext_.adocTreeModel_);
    selectGroupNodeDialog.setSelectedGroup(importContext_.destination_);
    selectGroupNodeDialog.setLabelText("Select import location:");
    if (selectGroupNodeDialog.exec() == QDialog::Rejected)
        return;

    // ----------------------------------------------------
    // o Create the entities and add to the repository
    PodEntityService podEntityService(importContext_.adoc_);
    QVector<IEntitySPtr> entities = podEntityService.convertToEntities(parsePod.simpleSeqPods_,
                                                                       parsePod.isAlignment_,
                                                                       parsePod.grammar_);
    ASSERT(entities.size() > 0);

    // o Special case: if an alignment was imported, it should be given a name, for this, we simple use the filename
    if (parsePod.isAlignment_ == eTrue)
    {
        ASSERT(entities.size() == 1);
        ASSERT(boost::shared_dynamic_cast<AbstractMsa>(entities.first()));
        AbstractMsaSPtr abstractMsa = boost::shared_static_cast<AbstractMsa>(entities.first());
        QFileInfo fileInfo(importContext_.importFileName_);
        QString msaName = fileInfo.fileName();
        if (!fileInfo.suffix().isEmpty())
            msaName.chop(fileInfo.suffix().length() + 1);
        msaName.replace("\\s+", " ");
        abstractMsa->setName(msaName);
    }

    IRepository *repository = importContext_.adoc_->repository(static_cast<EntityType>(entities.first()->type()));
    ASSERT(repository != nullptr);
    if (!repository->addGeneric(entities, false))
    {
        emit importError("Unable to add sequences to the repository");
        return;
    }

    // Note: By adding the entity to the repository it now has a refcount of 1, technically, we should unfind these
    // because entities will be popped off the stack and no longer have a reference to the pointer.
    //
    // However, since we are creating a TreeNode with a pointer to each of these entities, the final reference count
    // will be one per entity anyways. Thus, we do not "unfind" the entities here.

    // ----------------------------------------------------
    // o Create the AdocTreeNodes and add to the tree
    QVector<AdocTreeNode *> adocTreeNodes;
    adocTreeNodes.reserve(entities.size());
    foreach (const IEntitySPtr &entity, entities)
        adocTreeNodes << new AdocTreeNode(entity);
    if (!importContext_.adocTreeModel_->appendRows(adocTreeNodes, selectGroupNodeDialog.selectedGroupIndex()))
    {
        qDeleteAll(adocTreeNodes);
        emit importError("Error appending nodes to tree");
        return;
    }

    // ----------------------------------------------------
    // o Update the modified status
    importContext_.adoc_->setModified(true);

    emit importSuccessful(selectGroupNodeDialog.selectedGroupIndex());
}

/**
  * @param currentStep [int]
  * @param totalSteps [int]
  */
void SequenceImporter::onParserProgressChanged(int currentStep, int totalSteps)
{
    ASSERT(parseProgressDialog_ != nullptr);

    // To prevent the dialog box from flickering :)
    if (parseProgressDialog_->wasCanceled())
        return;

    if (parseProgressDialog_->maximum() != totalSteps)
        parseProgressDialog_->setMaximum(totalSteps);

    parseProgressDialog_->setValue(currentStep);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns Alphabet
  */
Alphabet SequenceImporter::askUserForAlphabet() const
{
    bool ok = false;
    QString item = QInputDialog::getItem(0,
                                         "Sequence type",
                                         "Select the sequence type:",
                                         QStringList() << "DNA" << "Protein (Amino)",
                                         0,
                                         false,
                                         &ok);
    if (!ok)
        return Alphabet();

    if (item == "DNA")
        return constants::kDnaBasicAlphabet;

    return constants::kAminoBasicAlphabet;
}

/**
  * @param parsePod [SequenceParseResultPod &]
  */
void SequenceImporter::upperCaseSequenceData(SequenceParseResultPod &parsePod) const
{
    for (int i=0, z=parsePod.simpleSeqPods_.size(); i<z; ++i)
    {
        SimpleSeqPod &simpleSeqPod = parsePod.simpleSeqPods_[i];
        simpleSeqPod.sequence_ = simpleSeqPod.sequence_.toUpper();
    }
}
