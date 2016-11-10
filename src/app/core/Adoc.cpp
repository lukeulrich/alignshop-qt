/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "Adoc.h"

#include "DataMappers/AminoSeqMapper.h"
#include "DataMappers/AminoMsaMapper.h"
#include "DataMappers/DnaSeqMapper.h"
#include "DataMappers/DnaMsaMapper.h"
#include "Entities/AbstractMsa.h"
#include "Entities/AminoMsa.h"
#include "Entities/AminoSeq.h"
#include "Entities/Astring.h"
#include "Entities/BlastReport.h"
#include "Entities/DnaSeq.h"
#include "Entities/Dstring.h"
#include "Entities/TransientTask.h"
#include "Repositories/IRepository.h"
#include "AdocTreeNode.h"


IncrementNumberGenerator Adoc::temporaryDocumentNumberIncrementor_(0);

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
Adoc::Adoc(QObject *parent) : QObject(parent)
{
    entityTreeRoot_ = nullptr;
    astringMapper_ = nullptr;
    aminoSeqMapper_ = nullptr;
    astringRepository_ = nullptr;
    aminoSeqRepository_ = nullptr;

    dstringMapper_ = nullptr;
    dnaSeqMapper_ = nullptr;
    dstringRepository_ = nullptr;
    dnaSeqRepository_ = nullptr;

    aminoMsaMapper_ = nullptr;
    aminoMsaRepository_ = nullptr;

    dnaMsaMapper_ = nullptr;
    dnaMsaRepository_ = nullptr;

    transientTaskRepository_ = nullptr;

    blastReportMapper_ = nullptr;
    blastReportRepository_ = nullptr;

    modified_ = false;
    temporary_ = false;
    temporaryDocumentNumber_ = 0;
}

/**
  */
Adoc::~Adoc()
{
    close();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QDir
  */
QDir Adoc::blastDataPath() const
{
    return QDir(dataPath() + QDir::separator() + "blast");
}

/**
  * @returns QDir
  */
QString Adoc::dataPath() const
{
    if (isTemporary())
        return QString();

    QFileInfo fileInfo(fileName());

    return fileInfo.path() + QDir::separator() + fileInfo.completeBaseName() + dataPathSuffix();
}

QString Adoc::dataPathSuffix() const
{
    return "-data";
}

/**
  * @returns QDir
  */
QDir Adoc::documentDirectory() const
{
    QFileInfo fileInfo(fileName());

    return QDir(fileInfo.path());
}

/**
  * @returns QString
  */
QString Adoc::fileName() const
{
    if (!isTemporary())
        return sqliteAdocSource_.fileName();

    return temporaryDocumentName();
}

/**
  * @returns bool
  */
bool Adoc::isModified() const
{
    return modified_;
}

/**
  * @returns bool
  */
bool Adoc::isOpen() const
{
    return sqliteAdocSource_.isOpen();
}

/**
  * @returns bool
  */
bool Adoc::isTemporary() const
{
    return temporary_;
}

/**
  * @returns IAdocSource *
  */
IAdocSource *Adoc::adocSource()
{
    return &sqliteAdocSource_;
}

/**
  * @returns AdocTreeNode *
  */
AdocTreeNode *Adoc::entityTree() const
{
    return entityTreeRoot_;
}

void Adoc::vacuum()
{
    sqliteAdocSource_.vacuum();
}

/**
  * @returns AnonSeqRepository<Astring> *
  */
AnonSeqRepository<Astring> *Adoc::astringRepository() const
{
    return astringRepository_;
}

/**
  * @returns GenericRepository<AminoSeq> *
  */
GenericRepository<AminoSeq> *Adoc::aminoSeqRepository() const
{
    return aminoSeqRepository_;
}

/**
  * @returns AnonSeqRepository<Dstring> *
  */
AnonSeqRepository<Dstring> *Adoc::dstringRepository() const
{
    return dstringRepository_;
}

/**
  * @returns GenericRepository<DnaSeq> *
  */
GenericRepository<DnaSeq> *Adoc::dnaSeqRepository() const
{
    return dnaSeqRepository_;
}

/**
  * @returns MsaRepository<AminoMsa, AminoSeq> *
  */
MsaRepository<AminoMsa, AminoSeq> *Adoc::aminoMsaRepository() const
{
    return aminoMsaRepository_;
}

/**
  * @returns MsaRepository<DnaMsa, DnaSeq> *
  */
MsaRepository<DnaMsa, DnaSeq> *Adoc::dnaMsaRepository() const
{
    return dnaMsaRepository_;
}

/**
  * @returns MemoryOnlyRepository<TransientTask> *
  */
MemoryOnlyRepository<TransientTask> *Adoc::transientTaskRepository() const
{
    return transientTaskRepository_;
}

/**
  * @returns GenericRepository<BlastReport *>
  */
GenericRepository<BlastReport> *Adoc::blastReportRepository() const
{
    return blastReportRepository_;
}

/**
  * @param astring [Astring *]
  * @returns AnonSeqRepository<Astring> *
  */
AnonSeqRepository<Astring> *Adoc::repository(const AstringSPtr & /* astring */) const
{
    return astringRepository_;
}

/**
  * @param aminoSeq [AminoSeq *]
  * @returns GenericRepository<AminoSeq> *
  */
GenericRepository<AminoSeq> *Adoc::repository(const AminoSeqSPtr & /* aminoSeq */) const
{
    return aminoSeqRepository_;
}

/**
  * @param dstring [Dstring *]
  * @returns AnonSeqRepository<Dstring> *
  */
AnonSeqRepository<Dstring> *Adoc::repository(const DstringSPtr & /* dstring */) const
{
    return dstringRepository_;
}

/**
  * @param dnaSeq [DnaSeq *]
  * @returns GenericRepository<DnaSeq> *
  */
GenericRepository<DnaSeq> *Adoc::repository(const DnaSeqSPtr & /* dnaSeq */) const
{
    return dnaSeqRepository_;
}

/**
  * @param dnaMsa [DmaMsa *]
  * @returns MsaRepository<DnaMsa> *
  */
MsaRepository<DnaMsa, DnaSeq> *Adoc::repository(const DnaMsaSPtr & /* dnaMsa */) const
{
    return dnaMsaRepository_;
}

/**
  * @param transientTask [TransientTask *]
  * @returns MemoryOnlyRepository<TransientTask> *
  */
MemoryOnlyRepository<TransientTask> *Adoc::repository(const TransientTaskSPtr & /* transientTask */) const
{
    return transientTaskRepository_;
}

/**
  * @returns GenericRepository<BlastReport> *
  */
GenericRepository<BlastReport> *Adoc::repository(const BlastReportSPtr & /* blastReport */) const
{
    return blastReportRepository_;
}

/**
  * @param entity [IEntitySPtr &]
  * @returns IRepository *
  */
IRepository *Adoc::repository(const IEntitySPtr &entity) const
{
    if (entity == nullptr)
        return nullptr;

    return repository(static_cast<EntityType>(entity->type()));
}

/**
  * @param aminoMsa [AminoMsa *]
  * @returns MsaRepository<AminoMsa, AminoSeq> *
  */
MsaRepository<AminoMsa, AminoSeq> *Adoc::repository(const AminoMsaSPtr & /* aminoMsa */) const
{
    return aminoMsaRepository_;
}

/**
  * @param entityType [EntityType]
  * @returns IRepository *
  */
IRepository *Adoc::repository(EntityType entityType) const
{
    switch (entityType)
    {
    case eAstringEntity:
        return astringRepository_;
    case eAminoSeqEntity:
        return aminoSeqRepository_;
    case eAminoMsaEntity:
        return aminoMsaRepository_;
    case eDstringEntity:
        return dstringRepository_;
    case eDnaSeqEntity:
        return dnaSeqRepository_;
    case eDnaMsaEntity:
        return dnaMsaRepository_;
    case eTransientTaskEntity:
        return transientTaskRepository_;
    case eBlastReportEntity:
        return blastReportRepository_;

    default:
        return nullptr;
    }
}

/**
  * @param abstractMsa [const AbstractMsa *]
  * @returns IMsaRepository *
  */
IMsaRepository *Adoc::msaRepository(const AbstractMsaSPtr &abstractMsa) const
{
    if (!abstractMsa)
        return nullptr;

    switch (abstractMsa->type())
    {
    case eAminoMsaEntity:
        return aminoMsaRepository_;
    case eDnaMsaEntity:
        return dnaMsaRepository_;

    default:
        return nullptr;
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool Adoc::open(const QString &fileName)
{
    if (isOpen())
        return false;

    if (!sqliteAdocSource_.open(fileName))
        return false;

    ASSERT(modified_ == false);
    ASSERT(temporary_ == false);

    initialize();

    // Note: we call the fileName method instead of simply returning the fileName method argument to return a consistent
    // filename that may be retrieved later via the fileName() method.
    emit opened(Adoc::fileName());

    return true;
}

/**
  */
void Adoc::close()
{
    // The order of de-allocating repositories is quite important here because of inter-repository relationships. For
    // example, the aminoSeqRepository should be deleted before the astringRepository, because each AminoSeq contains an
    // Astring pointer and the teardown method of the AminoSeqMapper unfinds each astring of each AminoSeq.
    //
    // A more pressing need for deleting these objects in the specified order is that the aminoSeqRepository contains a
    // pointer to the astringRepository. If the astringRepository is deleted first, the aminoSeqRepository will have a
    // dangling pointer.
    //
    // TODO: Make these utilize a QObject hierarchy such that they are deleted in the reverse order they are allocated?

    // It is mandatory that repositories are deleted before their corresponding mappers becuase presumably the
    // repository will be calling the mapper instance!

    // First order of business: deallocate the entity tree to release any associated entities. This is essential to
    // perform *before* the corresponding repository confirms that all references have been released.
    delete entityTreeRoot_;             entityTreeRoot_ = nullptr;

    // Second order of business: Msa mappers because they aggregate the most entities and have the most dependencies.

    delete aminoMsaRepository_;         aminoMsaRepository_ = nullptr;
    delete aminoMsaMapper_;             aminoMsaMapper_ = nullptr;

    delete aminoSeqRepository_;         aminoSeqRepository_ = nullptr;
    delete astringRepository_;          astringRepository_ = nullptr;
    delete aminoSeqMapper_;             aminoSeqMapper_ = nullptr;
    delete astringMapper_;              astringMapper_ = nullptr;

    delete dnaMsaRepository_;           dnaMsaRepository_ = nullptr;
    delete dnaMsaMapper_;               dnaMsaMapper_ = nullptr;

    delete dnaSeqRepository_;           dnaSeqRepository_ = nullptr;
    delete dstringRepository_;          dstringRepository_ = nullptr;
    delete dnaSeqMapper_;               dnaSeqMapper_ = nullptr;
    delete dstringMapper_;              dstringMapper_ = nullptr;

    delete blastReportRepository_;      blastReportRepository_ = nullptr;
    delete blastReportMapper_;          blastReportMapper_ = nullptr;


    // Finally the MemoryOnlyRepositories
    delete transientTaskRepository_;    transientTaskRepository_ = nullptr;

    if (isOpen())
    {
        sqliteAdocSource_.close();
        modified_ = false;
        temporary_ = false;
        temporaryDocumentNumber_ = 0;

        emit closed();
    }
}

/**
  * @returns bool
  */
bool Adoc::create()
{
    if (isOpen())
        return false;

    if (!sqliteAdocSource_.createAndOpen(":memory:"))
        return false;

    initialize();

    temporaryDocumentNumber_ = temporaryDocumentNumberIncrementor_.nextValue();
    temporary_ = true;
    setModified(false);

    emit opened(fileName());

    return true;
}

/**
  * @returns bool
  */
bool Adoc::save()
{
    if (!isOpen())
        return false;

    sqliteAdocSource_.begin();

    // Note: it is important to save all the repositories before the entity tree so that any newly added entities
    //       will have their ids updated before saving the entity tree.
    if (!astringRepository_->saveAll())
        return false;
    if (!aminoSeqRepository_->saveAll())
        return false;
    if (!aminoMsaRepository_->saveAll())
        return false;
    if (!dstringRepository_->saveAll())
        return false;
    if (!dnaSeqRepository_->saveAll())
        return false;
    if (!dnaMsaRepository_->saveAll())
        return false;
    if (!blastReportRepository_->saveAll())
        return false;

    // All of the persistent storage repos will remove cruft automatically in the saveAll() command.
    transientTaskRepository_->removeCruft();

    try
    {
        sqliteAdocSource_.saveEntityTree(entityTreeRoot_);
    }
    catch (...)
    {
        sqliteAdocSource_.rollback();
        return false;
    }

    // Yehaw! Let's clean 'er up!
    sqliteAdocSource_.end();

    sqliteAdocSource_.begin();
    sqliteAdocSource_.removeCruft();
    sqliteAdocSource_.end();

    // Note: cannot vacuum from within transaction
    sqliteAdocSource_.vacuum();

    setModified(false);
    return true;
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool Adoc::saveAs(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    // Case 1: File -> File
    //      1.1: Unsaved input file
    //      1.2: Saved input file
    // Case 2: Memory -> File
    //      2.1: Unsaved input memory
    //      2.2: Saved input memory

    if (!sqliteAdocSource_.saveAs(fileName))
        return false;

    if (!save())
        return false;

    temporary_ = false;
    return true;
}

/**
  */
void Adoc::setModified()
{
    setModified(true);
}

/**
  * @param modified [bool]
  */
void Adoc::setModified(bool modified)
{
    if (modified_ != modified)
    {
        modified_ = modified;
        emit modifiedChanged(modified_);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  */
void Adoc::initialize()
{
    initializeDDD();
    loadEntityTree();
}

/**
  */
void Adoc::initializeDDD()
{
    ASSERT(isOpen());
    ASSERT(astringMapper_ == nullptr);
    ASSERT(astringRepository_ == nullptr);
    ASSERT(aminoSeqMapper_ == nullptr);
    ASSERT(aminoSeqRepository_ == nullptr);
    astringMapper_ = new AnonSeqMapper<Astring, AstringPod>(adocSource());
    astringRepository_ = new AnonSeqRepository<Astring>(astringMapper_);
    aminoSeqMapper_ = new AminoSeqMapper(adocSource(), astringRepository_);
    aminoSeqRepository_ = new GenericRepository<AminoSeq>(aminoSeqMapper_);

    ASSERT(aminoMsaMapper_ == nullptr);
    ASSERT(aminoMsaRepository_ == nullptr);
    aminoMsaMapper_ = new AminoMsaMapper(adocSource(), aminoSeqRepository_);
    aminoMsaRepository_ = new MsaRepository<AminoMsa, AminoSeq>(aminoMsaMapper_, aminoSeqRepository_);

    ASSERT(dstringMapper_ == nullptr);
    ASSERT(dstringRepository_ == nullptr);
    ASSERT(dnaSeqMapper_ == nullptr);
    ASSERT(dnaSeqRepository_ == nullptr);
    dstringMapper_ = new AnonSeqMapper<Dstring, DstringPod>(adocSource());
    dstringRepository_ = new AnonSeqRepository<Dstring>(dstringMapper_);
    dnaSeqMapper_ = new DnaSeqMapper(adocSource(), dstringRepository_);
    dnaSeqRepository_ = new GenericRepository<DnaSeq>(dnaSeqMapper_);

    ASSERT(dnaMsaMapper_ == nullptr);
    ASSERT(dnaMsaRepository_ == nullptr);
    dnaMsaMapper_ = new DnaMsaMapper(adocSource(), dnaSeqRepository_);
    dnaMsaRepository_ = new MsaRepository<DnaMsa, DnaSeq>(dnaMsaMapper_, dnaSeqRepository_);

    ASSERT(transientTaskRepository_ == nullptr);
    transientTaskRepository_ = new MemoryOnlyRepository<TransientTask>();

    ASSERT(blastReportMapper_ == nullptr);
    ASSERT(blastReportRepository_ == nullptr);
    blastReportMapper_ = new BlastReportMapper(adocSource(), blastDataPath());
    blastReportRepository_ = new GenericRepository<BlastReport>(blastReportMapper_);
}

/**
  */
void Adoc::loadEntityTree()
{
    ASSERT(isOpen());
    ASSERT(entityTreeRoot_ == nullptr);
    sqliteAdocSource_.begin();
    entityTreeRoot_ = sqliteAdocSource_.readEntityTree();
    sqliteAdocSource_.end();
    ASSERT(entityTreeRoot_ != nullptr);
}

/**
  * @returns QString
  */
QString Adoc::temporaryDocumentName() const
{
    return QString("Untitled-%1").arg(temporaryDocumentNumber_);
}
