/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOC_H
#define ADOC_H

#include <QtCore/QDir>
#include <QtCore/QObject>

#include "DataMappers/AnonSeqMapper.h"
#include "DataMappers/BlastReportMapper.h"
#include "DataSources/SqliteAdocSource.h"
#include "Repositories/AnonSeqRepository.h"
#include "Repositories/GenericRepository.h"
#include "Repositories/MemoryOnlyRepository.h"
#include "Repositories/MsaRepository.h"
#include "util/IncrementNumberGenerator.h"

#include "Entities/Astring.h"
#include "Entities/AminoSeq.h"
#include "Entities/BlastReport.h"
#include "Entities/Dstring.h"
#include "Entities/DnaSeq.h"
#include "Entities/AminoMsa.h"
#include "Entities/DnaMsa.h"
#include "Entities/TransientTask.h"

class AdocTreeNode;
class AminoSeqMapper;
class AminoMsaMapper;

class DnaSeqMapper;
class DnaMsaMapper;

class IAdocSource;
class IRepository;
class SqliteAdocSource;

class Adoc : public QObject
{
    Q_OBJECT

public:
    Adoc(QObject *parent = 0);
    ~Adoc();

    QDir blastDataPath() const;                     //!< Returns the directory for storing all BLAST searches
    QString dataPath() const;                          //!< Returns the root directory for storing data on the filesystem for this document
    QString dataPathSuffix() const;
    QDir documentDirectory() const;                 //!< Returns the parent directory containing this document
    QString fileName() const;
    bool isModified() const;
    bool isOpen() const;
    bool isTemporary() const;

    IAdocSource *adocSource();
    AdocTreeNode *entityTree() const;

    void vacuum();

    AnonSeqRepository<Astring> *astringRepository() const;
    GenericRepository<AminoSeq> *aminoSeqRepository() const;
    AnonSeqRepository<Dstring> *dstringRepository() const;
    GenericRepository<DnaSeq> *dnaSeqRepository() const;
    MsaRepository<AminoMsa, AminoSeq> *aminoMsaRepository() const;
    MsaRepository<DnaMsa, DnaSeq> *dnaMsaRepository() const;
    MemoryOnlyRepository<TransientTask> *transientTaskRepository() const;
    GenericRepository<BlastReport> *blastReportRepository() const;

    AnonSeqRepository<Astring> *repository(const AstringSPtr &astring) const;
    GenericRepository<AminoSeq> *repository(const AminoSeqSPtr &aminoSeq) const;
    AnonSeqRepository<Dstring> *repository(const DstringSPtr &astring) const;
    GenericRepository<DnaSeq> *repository(const DnaSeqSPtr &aminoSeq) const;
    MsaRepository<AminoMsa, AminoSeq> *repository(const AminoMsaSPtr &aminoMsa) const;
    MsaRepository<DnaMsa, DnaSeq> *repository(const DnaMsaSPtr &dnaMsa) const;
    MemoryOnlyRepository<TransientTask> *repository(const TransientTaskSPtr &transientTask) const;
    GenericRepository<BlastReport> *repository(const BlastReportSPtr &blastReport) const;

    IRepository *repository(const IEntitySPtr &entity) const;
    IRepository *repository(EntityType entityType) const;
    IMsaRepository *msaRepository(const AbstractMsaSPtr &abstractMsa) const;

public Q_SLOTS:
    bool open(const QString &fileName);
    void close();
    bool create();                          // The equivalent of new document
    bool save();
    bool saveAs(const QString &fileName);
    void setModified();
    void setModified(bool modified);

Q_SIGNALS:
    void closed();
    void modifiedChanged(bool isModified);
    void opened(const QString &fileName);

private:
    void initialize();
    void initializeDDD();
    void loadEntityTree();
    QString temporaryDocumentName() const;

    SqliteAdocSource sqliteAdocSource_;
    AdocTreeNode *entityTreeRoot_;

    AnonSeqMapper<Astring, AstringPod> *astringMapper_;
    AminoSeqMapper *aminoSeqMapper_;

    AnonSeqMapper<Dstring, DstringPod> *dstringMapper_;
    DnaSeqMapper *dnaSeqMapper_;

    AnonSeqRepository<Astring> *astringRepository_;
    GenericRepository<AminoSeq> *aminoSeqRepository_;

    AnonSeqRepository<Dstring> *dstringRepository_;
    GenericRepository<DnaSeq> *dnaSeqRepository_;

    AminoMsaMapper *aminoMsaMapper_;
    MsaRepository<AminoMsa, AminoSeq> *aminoMsaRepository_;

    DnaMsaMapper *dnaMsaMapper_;
    MsaRepository<DnaMsa, DnaSeq> *dnaMsaRepository_;

    MemoryOnlyRepository<TransientTask> *transientTaskRepository_;

    BlastReportMapper *blastReportMapper_;
    GenericRepository<BlastReport> *blastReportRepository_;

    bool modified_;
    bool temporary_;
    int temporaryDocumentNumber_;

    static IncrementNumberGenerator temporaryDocumentNumberIncrementor_;
};

#endif // ADOC_H
