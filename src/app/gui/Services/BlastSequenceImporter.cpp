/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>

#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

#include <QtDebug>

#include "BlastSequenceImporter.h"

#include "../../core/Entities/Astring.h"
#include "../../core/Entities/AminoSeq.h"
#include "../../core/Entities/Dstring.h"
#include "../../core/Entities/DnaSeq.h"

#include "../../core/Repositories/IRepository.h"
#include "../../core/Repositories/IAnonSeqRepository.h"
#include "../../core/Services/BlastSequenceFetcher.h"
#include "../../core/Adoc.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/BioString.h"
#include "../../core/constants.h"
#include "../../core/macros.h"

#include "../forms/SelectGroupNodeDialog.h"
#include "../models/AdocTreeModel.h"
#include "../models/BlastReportModel.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
BlastSequenceImporter::BlastSequenceImporter(QObject *parent)
    : QObject(parent),
      adoc_(nullptr),
      adocTreeModel_(nullptr),
      blastSequenceFetcher_(nullptr)
{
}

Adoc *BlastSequenceImporter::adoc() const
{
    return adoc_;
}

void BlastSequenceImporter::setAdoc(Adoc *adoc)
{
    adoc_ = adoc;
}

void BlastSequenceImporter::setAdocTreeModel(AdocTreeModel *adocTreeModel)
{
    adocTreeModel_ = adocTreeModel;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param request [const BlastImportRequest &]
  */
void BlastSequenceImporter::import(const BlastImportRequest &request)
{
    if (adoc_ == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "No adoc configured";
        return;
    }
    if (adocTreeModel_ == nullptr)
    {
        qDebug() << Q_FUNC_INFO << "No adoc tree model configured";
        return;
    }

    if (!isValidRequest(request))
    {
        // TODO: Display a more informative message
        qDebug() << Q_FUNC_INFO << "Invalid blast request";
        return;
    }

    QVector<IEntitySPtr> entities;
    AdocTreeNodeVector adocTreeNodes;

    try
    {
        QStringList blastIds = getBlastIds(request.blastIndices_);
        ASSERT(blastIds.size() == request.blastIndices_.size());
        BioStringVector bioStrings = fetchSequences(blastIds, request.blastDatabase_, request.grammar_);
        if (bioStrings.isEmpty())
        {
            displayWarning("No sequences were able to be fetched from the BLAST database");
            return;
        }
        ASSERT(blastIds.size() == bioStrings.size());
        if (containsEmptyBioString(bioStrings))
        {
            displayWarning("One or more sequences could not be retrieved");
            return;
        }

        // Because the dialog to select a destination index provides the option to cancel, get this and confirm that the
        // user wants to continue before allocating memory and inserting nodes into the repository
        QModelIndex destinationTreeIndex = getDestinationIndex(request.destinationIndex_);

        entities = createEntities(request.blastIndices_,
                                  bioStrings);

        adocTreeNodes = createAdocTreeNodes(entities);
        if (!addToRepository(entities))
        {
            displayWarning("An error occurred while attempting to import sequences into the repository. Please "
                           "try again.");
            return;
        }

        if (!adocTreeModel_->appendRows(adocTreeNodes, destinationTreeIndex))
        {
            qDeleteAll(adocTreeNodes);
            displayWarning("An error occurred while adding sequences to the data organizer tree.");
            return;
        }
    }
    catch(...)
    {
        qDeleteAll(adocTreeNodes);

        return;
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void BlastSequenceImporter::onBlastImportError(int /* id */, const QString &message)
{
    displayWarning(message);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param entities [const QVector<IEntitySPtr &> &]
  * @returns bool
  */
bool BlastSequenceImporter::addToRepository(const QVector<IEntitySPtr> &entities) const
{
    ASSERT(adoc_ != nullptr);

    IRepository *repository = adoc_->repository(entities.first());
    ASSERT(repository != nullptr);
    return repository->addGeneric(entities, false);
}

/**
  * @param bioStringVector [const QVector<BioString> &]
  * @returns bool
  */
bool BlastSequenceImporter::containsEmptyBioString(const QVector<BioString> &bioStringVector) const
{
    foreach (const BioString &bioString, bioStringVector)
        if (bioString.isEmpty())
            return true;

    return false;
}

/**
  * @param entities [const QVector<IEntitySPtr &> &]
  * @returns AdocTreeNodeVector
  */
AdocTreeNodeVector BlastSequenceImporter::createAdocTreeNodes(const QVector<IEntitySPtr> &entities)
{
    AdocTreeNodeVector adocTreeNodes;
    adocTreeNodes.reserve(entities.size());
    foreach (const IEntitySPtr &entity, entities)
        adocTreeNodes << new AdocTreeNode(entity);

    return adocTreeNodes;
}

/**
  * @param blastIndices_ [const QModelIndexList &]
  * @param bioStringVector [const QVector<BioString> &]
  * @returns QVector<IEntitySPtr>
  */
QVector<IEntitySPtr> BlastSequenceImporter::createAminoSeqEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const
{
    ASSERT(adoc_ != nullptr);

    QVector<IEntitySPtr> entities;
    entities.reserve(blastIndices_.size());

    AnonSeqRepository<Astring> *astringRepository = adoc_->astringRepository();
    ASSERT(astringRepository != nullptr);
    for (int i=0, z=blastIndices_.size(); i<z; ++i)
    {
        const QModelIndex &index = blastIndices_.at(i);
        const BioString &bioString = bioStringVector.at(i);

        const AstringSPtr &astring = astringRepository->findBySeqOrCreate(bioString);
        ASSERT(astring);

        // Extract the name, start, stop, description values from the blast data
        QString name = index.sibling(index.row(), BlastReportModel::eIdColumn).data().toString();
        int start = index.sibling(index.row(), BlastReportModel::eHitFromColumn).data().toInt();
        int stop = index.sibling(index.row(), BlastReportModel::eHitToColumn).data().toInt();
        QString description = index.sibling(index.row(), BlastReportModel::eDefinitionColumn).data().toString();

        // TODO: Check if start/stop are valid; perhaps should throw an exception in these cases

        entities << AminoSeqSPtr(AminoSeq::createEntity(start, stop, name, QString(), description, "Imported from BLAST", astring));
    }

    return entities;
}

/**
  * @param blastIndices_ [const QModelIndexList &]
  * @param bioStringVector [const QVector<BioString> &]
  * @returns QVector<IEntitySPtr>
  */
QVector<IEntitySPtr> BlastSequenceImporter::createDnaSeqEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const
{
    ASSERT(adoc_ != nullptr);

    QVector<IEntitySPtr> entities;
    entities.reserve(blastIndices_.size());

    AnonSeqRepository<Dstring> *dstringRepository = adoc_->dstringRepository();
    ASSERT(dstringRepository != nullptr);
    for (int i=0, z=blastIndices_.size(); i<z; ++i)
    {
        const QModelIndex &index = blastIndices_.at(i);
        const BioString &bioString = bioStringVector.at(i);

        const DstringSPtr &dstring = dstringRepository->findBySeqOrCreate(bioString);
        ASSERT(dstring != nullptr);

        // Extract the name, start, stop, description values from the blast data
        QString name = index.sibling(index.row(), BlastReportModel::eIdColumn).data().toString();
        int start = index.sibling(index.row(), BlastReportModel::eHitFromColumn).data().toInt();
        int stop = index.sibling(index.row(), BlastReportModel::eHitToColumn).data().toInt();
        QString description = index.sibling(index.row(), BlastReportModel::eDefinitionColumn).data().toString();

        // TODO: Check if start/stop are valid
        entities << DnaSeqSPtr(DnaSeq::createEntity(start, stop, name, QString(), description, "Imported from BLAST", dstring));
    }

    return entities;
}

/**
  * @param blastIndices_ [const QModelIndexList &]
  * @param bioStringVector [const QVector<BioString> &]
  * @returns QVector<IEntitySPtr &>
  */
QVector<IEntitySPtr> BlastSequenceImporter::createEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const
{
    ASSERT(adoc_ != nullptr);
    ASSERT(bioStringVector.size() > 0);
    ASSERT(bioStringVector.first().grammar() == eAminoGrammar || bioStringVector.first().grammar() == eDnaGrammar);

    if (bioStringVector.first().grammar() == eAminoGrammar)
        return createAminoSeqEntities(blastIndices_, bioStringVector);

    return createDnaSeqEntities(blastIndices_, bioStringVector);
}

/**
  * @param message [const QString &]
  */
void BlastSequenceImporter::displayWarning(const QString &message) const
{
    QMessageBox::warning(parentWidget(), "Import error", message, QMessageBox::Ok);
}

/**
  * @param blastIds [const QStringList &]
  * @param blastDatabase_ [const QString &]
  * @returns BioStringVector
  */
BioStringVector BlastSequenceImporter::fetchSequences(const QStringList &blastIds, const QString &blastDatabase_, const Grammar &grammar)
{
    ASSERT(blastIds.size() > 0);
    ASSERT(blastDatabase_.isEmpty() == false);

    if (blastSequenceFetcher_ == nullptr)
        initializeFetcher();

    QEventLoop eventLoop;

    QProgressDialog progressDialog(parentWidget());
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setLabelText("Importing sequences...");
    progressDialog.setRange(0, blastIds.size());
    progressDialog.setMinimumDuration(1500);

    connect(blastSequenceFetcher_, SIGNAL(progressChanged(int,int)), &progressDialog, SLOT(setValue(int)));
    connect(&progressDialog, SIGNAL(canceled()), blastSequenceFetcher_, SLOT(kill()));
    connect(blastSequenceFetcher_, SIGNAL(error(int,QString)), SLOT(onBlastImportError(int,QString)));
    connect(blastSequenceFetcher_, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
    connect(blastSequenceFetcher_, SIGNAL(finished(BioStringVector)), &eventLoop, SLOT(quit()));

    blastSequenceFetcher_->fetch(blastIds, blastDatabase_, grammar);
    eventLoop.exec();

    if (progressDialog.wasCanceled())
        throw "Import canceled";      // This is another execution path which will avoid displaying any user messages/warnings

    return blastSequenceFetcher_->bioStrings();
}

/**
  * @param blastIndices [const QModelIndexList &]
  * @returns QStringList
  */
QStringList BlastSequenceImporter::getBlastIds(const QModelIndexList &blastIndices) const
{
    QStringList blastIds;
    foreach (const QModelIndex &index, blastIndices)
        blastIds << index.sibling(index.row(), BlastReportModel::eIdColumn).data().toString();

    return blastIds;
}

/**
  * @param initialIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex BlastSequenceImporter::getDestinationIndex(const QModelIndex &initialIndex)
{
    SelectGroupNodeDialog selectGroupNodeDialog(parentWidget());
    selectGroupNodeDialog.setAdocTreeModel(adocTreeModel_);
    selectGroupNodeDialog.setSelectedGroup(initialIndex);
    selectGroupNodeDialog.setLabelText("Select import location:");
    if (selectGroupNodeDialog.exec() == QDialog::Accepted)
        return selectGroupNodeDialog.selectedGroupIndex();

    throw "User canceled selection";
}

/**
  */
void BlastSequenceImporter::initializeFetcher()
{
    ASSERT(blastSequenceFetcher_ == nullptr);

    blastSequenceFetcher_ = new BlastSequenceFetcher(this);
    try
    {
        blastSequenceFetcher_->setBlastDbCmdPath(QCoreApplication::applicationDirPath() +
                                                 QDir::separator() +
                                                 constants::kBlastDbCmdRelativePath);
    }
    catch (QString &error)
    {
        qDebug() << Q_FUNC_INFO << "Warning:" << error;
        throw error;
    }
}

/**
  * @param blastImportRequest [const BlastImportRequest &]
  * @returns bool
  */
bool BlastSequenceImporter::isValidRequest(const BlastImportRequest &blastImportRequest) const
{
    return !blastImportRequest.blastDatabase_.isEmpty() &&
           !blastImportRequest.blastIndices_.isEmpty();
}

/**
  * @returns QWidget *
  */
QWidget *BlastSequenceImporter::parentWidget() const
{
    QObject *p = parent();
    if (p && p->isWidgetType())
        return static_cast<QWidget *>(p);

    return nullptr;
}
