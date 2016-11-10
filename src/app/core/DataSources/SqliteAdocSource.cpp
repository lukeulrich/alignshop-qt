#include <QtGlobal>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVariant>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>

// Include the sqlite library for accessing the sqlite backup API
#ifdef Q_OS_LINUX
#include <sqlite3.h>
#elif defined Q_OS_MACX
#include <sqlite3.h>
#elif defined Q_OS_WIN
#include "sqlite3.h"
#endif

#include "SqliteAdocSource.h"

#include "../exceptions/InvalidMpttNodeError.h"
#include "../AdocTreeNode.h"
#include "../Mptt.h"
#include "../MpttNode.h"
#include "../enums.h"
#include "../macros.h"

#include <QtDebug>

typedef MpttNode<AdocTreeNode> AdocMpttNode;

int SqliteAdocSource::connectionNumber_ = 1;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
SqliteAdocSource::SqliteAdocSource()
    : AbstractAdocSource(),
      AbstractDbSource(),
      astringCrud_(this),
      aminoSeqCrud_(this),
      dstringCrud_(this),
      dnaSeqCrud_(this),
      aminoMsaCrud_(this),
      dnaMsaCrud_(this),
      blastReportCrud_(this)
{
}

/**
  */
SqliteAdocSource::~SqliteAdocSource()
{
    close();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SqliteAdocSource::close()
{
    clearPreparedQueries();
    QSqlDatabase::removeDatabase(connectionName_);
    connectionName_.clear();
    fileName_.clear();
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool SqliteAdocSource::createAndOpen(const QString &fileName)
{
    if (fileName != ":memory:" && QFile::exists(fileName))
        return false;

    if (!openOrCreate(fileName))
        return false;

    // Now create the necessary database tables
    createTables();

    return true;
}

/**
  * @returns bool
  */
bool SqliteAdocSource::isOpen() const
{
    return connectionName_.isEmpty() == false;
}

/**
  * @returns QString
  */
QString SqliteAdocSource::fileName() const
{
    return fileName_;
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool SqliteAdocSource::open(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return false;

    if (!openOrCreate(fileName))
        return false;

    if (!isValidDatabase())
    {
        close();
        return false;
    }

    return true;
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool SqliteAdocSource::saveAs(const QString &dstFileName)
{
    if (!isOpen())
        return false;

    QByteArray array = fileName_.toLocal8Bit();

    sqlite3 *srcHandle = nullptr;
    if (fileName_ == ":memory:")
    {
        QVariant variantHandle = database().driver()->handle();
        ASSERT(variantHandle.isValid() && qstrcmp(variantHandle.typeName(), "sqlite3*") == 0);
        srcHandle = *static_cast<sqlite3 **>(variantHandle.data());
        if (srcHandle == nullptr)
            return false;
    }
    else
    {
        int returnCode = sqlite3_open(array.data(), &srcHandle);
        if (returnCode != SQLITE_OK)
            return false;
    }

    sqlite3 *dstHandle = nullptr;
    int returnCode = sqlite3_open(dstFileName.toLocal8Bit().data(), &dstHandle);
    if (returnCode == SQLITE_OK)
    {
        sqlite3_backup *backup = sqlite3_backup_init(dstHandle, "main", srcHandle, "main");
        if (backup != nullptr)
        {
            while (returnCode == SQLITE_OK || returnCode == SQLITE_BUSY || returnCode == SQLITE_LOCKED)
            {
                returnCode = sqlite3_backup_step(backup, -1);
                if (returnCode == SQLITE_OK || returnCode == SQLITE_BUSY || returnCode == SQLITE_LOCKED)
                    sqlite3_sleep(30);
            }

            (void)sqlite3_backup_finish(backup);
        }

        returnCode = sqlite3_errcode(dstHandle);
    }

    (void)sqlite3_close(dstHandle);
    (void)sqlite3_close(srcHandle);

    if (returnCode != SQLITE_OK || !QFile::exists(dstFileName))
        return false;

    // Switch databases to the new file
    QString connectionName = QFileInfo(dstFileName).fileName() + QString::number(connectionNumber_);
    bool opened = false;
    {
        // Hide the QSqlDatabase reference inside a code block so that we can remove it later if it failed to open
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(dstFileName);
        opened = db.open();
    }
    if (!opened)
    {
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    close();

    connectionName_ = connectionName;
    fileName_ = dstFileName;

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  */
void SqliteAdocSource::begin()
{
//    qDebug() << Q_FUNC_INFO;
    QSqlQuery query = getPreparedQuery("beginSavepoint",
                                       "SAVEPOINT savePoint");

    if (!query.exec())
        throw 0;
}

/**
  */
void SqliteAdocSource::rollback()
{
//    qDebug() << Q_FUNC_INFO;
    QSqlQuery query = getPreparedQuery("rollbackSavepoint",
                                       "ROLLBACK TO SAVEPOINT savePoint");

    if (!query.exec())
        throw 0;
}

/**
  */
void SqliteAdocSource::end()
{
//    qDebug() << Q_FUNC_INFO;
    QSqlQuery query = getPreparedQuery("endSavepoint",
                                       "RELEASE SAVEPOINT savePoint");

    if (!query.exec())
        throw 0;
}

/**
  * Remove all unreferenced records
  */
void SqliteAdocSource::removeCruft()
{
    removeCruftAstrings();
    removeCruftDstrings();
    removeOrphanPrimerSearchParameters();
}

/**
  */
void SqliteAdocSource::vacuum()
{
    QSqlQuery query = getPreparedQuery("vacuumDatabase",
                                       "vacuum");
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
}

/**
  * @return AdocTreeNode *
  */
AdocTreeNode *SqliteAdocSource::readEntityTree()
{
    // A: Count the number of records
    QSqlQuery selectTreeRowCount = getPreparedQuery("countEntityTreeRows",
                                                    "SELECT count(*) "
                                                    "FROM entity_tree");

    if (!selectTreeRowCount.exec())
    {
        qDebug() << selectTreeRowCount.lastError().text();
        throw 0;
    }

    selectTreeRowCount.next();
    int nTreeRows = selectTreeRowCount.value(0).toInt();
    if (nTreeRows == 0)
        return new AdocTreeNode(eRootNode, "Root");

    // B: Read in all the entity data
    QSqlQuery selectEntityNodes = getPreparedQuery("readEntityTree",
                                                   "SELECT type_id, entity_id, label, lft, rgt "
                                                   "FROM entity_tree "
                                                   "ORDER BY lft");

    if (!selectEntityNodes.exec())
    {
        qDebug() << selectEntityNodes.lastError().text();
        throw 0;
    }

    QVector<AdocMpttNode> mpttNodes;
    mpttNodes.reserve(nTreeRows);
    while (selectEntityNodes.next())
    {
        AdocNodeType nodeType = AdocTreeNode::mapNodeType(selectEntityNodes.value(0).toInt());
        ASSERT(nodeType != eUndefinedNode);
        if (nodeType == eUndefinedNode)
        {
            Mptt::freeMpttTreeNodes(mpttNodes);
            throw 0;
        }

        int entityId = (selectEntityNodes.value(1).isNull() == false) ? selectEntityNodes.value(1).toInt() : 0;
        AdocTreeNode *treeNode = new AdocTreeNode(nodeType, selectEntityNodes.value(2).toString(), entityId);
        int left = selectEntityNodes.value(3).toInt();
        int right = selectEntityNodes.value(4).toInt();
        mpttNodes << AdocMpttNode(treeNode, left, right);
    }

    ASSERT(mpttNodes.count() > 0);

    selectTreeRowCount.finish();
    selectEntityNodes.finish();

    // C: Convert nodes to a tree
    try
    {
        return Mptt::fromMpttVector(mpttNodes);
    }
    catch (InvalidMpttNodeError &e)
    {
        qDebug() << Q_FUNC_INFO << e.what();
        Mptt::freeMpttTreeNodes(mpttNodes);
        throw;
    }
    catch (...)
    {
        qDebug() << Q_FUNC_INFO;
        Mptt::freeMpttTreeNodes(mpttNodes);
        throw;
    }
}

/**
  * root [AdocTreeNode *]
  */
void SqliteAdocSource::saveEntityTree(AdocTreeNode *root)
{
    QSqlQuery emptyTree = getPreparedQuery("truncateEntityTree",
                                           "DELETE FROM entity_tree");

    if (!emptyTree.exec())
    {
        qDebug() << emptyTree.lastError().text();
        throw 0;
    }

    // A. Convert to a list of MpttNodes
    QVector<AdocMpttNode> mpttNodes = Mptt::toMpttVector(root);

    // B. Insert into the database
    QSqlQuery insert = getPreparedQuery("insertEntityTreeRow",
                                        "INSERT INTO entity_tree (type_id, type, entity_id, label, lft, rgt) "
                                        "VALUES (?, ?, ?, ?, ?, ?)");

    foreach (const AdocMpttNode &mpttNode, mpttNodes)
    {
        AdocTreeNode *treeNode = mpttNode.treeNode_;

        insert.bindValue(0, treeNode->nodeType_);
        insert.bindValue(1, AdocTreeNode::textForType(treeNode->nodeType_));
        if (treeNode->entityId() != 0)
            insert.bindValue(2, treeNode->entityId());
        else
            insert.bindValue(2, QVariant(QVariant::Int));
        if (treeNode->label_.isEmpty() == false)
            insert.bindValue(3, treeNode->label_);
        else
            insert.bindValue(3, QVariant(QVariant::String));
        insert.bindValue(4, mpttNode.left_);
        insert.bindValue(5, mpttNode.right_);

        if (!insert.exec())
        {
            qDebug() << insert.lastError().text();
            throw 0;
        }
    }

    insert.finish();
}

/**
  * @returns IAnonSeqEntityCrud<Astring, AstringPod> *
  */
IAnonSeqEntityCrud<Astring, AstringPod> *SqliteAdocSource::astringCrud()
{
    return &astringCrud_;
}

/**
  * @returns IEntityCrud<AminoSeq, AminoSeqPod> *
  */
IEntityCrud<AminoSeq, AminoSeqPod> *SqliteAdocSource::aminoSeqCrud()
{
    return &aminoSeqCrud_;
}

/**
  * @returns IAnonSeqEntityCrud<Dstring, DstringPod> *
  */
IAnonSeqEntityCrud<Dstring, DstringPod> *SqliteAdocSource::dstringCrud()
{
    return &dstringCrud_;
}

/**
  * @returns IEntityCrud<DnaSeq, DnaSeqPod> *
  */
IEntityCrud<DnaSeq, DnaSeqPod> *SqliteAdocSource::dnaSeqCrud()
{
    return &dnaSeqCrud_;
}

/**
  * @returns IMsaCrud<AminoMsa, AminoMsaPod> *
  */
IMsaCrud<AminoMsa, AminoMsaPod> *SqliteAdocSource::aminoMsaCrud()
{
    return &aminoMsaCrud_;
}

/**
  * @returns IMsaCrud<DnaMsa, DnaMsaPod> *
  */
IMsaCrud<DnaMsa, DnaMsaPod> *SqliteAdocSource::dnaMsaCrud()
{
    return &dnaMsaCrud_;
}

/**
  * @returns IBlastReportCrud *
  */
IBlastReportCrud *SqliteAdocSource::blastReportCrud()
{
    return &blastReportCrud_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Removed autoincrement for small speed improvement on insertion. See log, 16 June 2011
  *
  */
void SqliteAdocSource::createTables() const
{
    QSqlDatabase db = database();
    QSqlQuery query(db);

    // Table: entity_tree
    if (!query.exec("CREATE TABLE entity_tree ("
                    "    type_id integer not null,"
                    "    type text not null,"
                    "    entity_id integer,"
                    "    label text,"
                    "    lft integer not null,"
                    "    rgt integer not null,"
                    "    check(lft > 0),"
                    "    check(rgt > lft)"
                    ")"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: astrings
    if (!query.exec("CREATE TABLE astrings ("
                    "    id integer not null primary key,"
                    "    digest text not null,"
                    "    length integer not null,"
                    "    sequence text not null,"
                    "    check(length = length(sequence))"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE INDEX astrings_digest_index ON astrings(digest)"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: coils
    if (!query.exec("CREATE TABLE coils ("
                    "    id integer not null primary key,"
                    "    astring_id integer not null,"
                    "    start integer not null,"
                    "    stop integer not null,"
                    "    check (start > 0),"
                    "    check (stop >= start),"
                    "    foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: segs
    if (!query.exec("CREATE TABLE segs ("
                    "    id integer not null primary key,"
                    "    astring_id integer not null,"
                    "    start integer not null,"
                    "    stop integer not null,"
                    "    check (start > 0),"
                    "    check (stop >= start),"
                    "    foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: q3
    if (!query.exec("CREATE TABLE q3 ("
//                    "    id integer not null primary key,"
                    "    astring_id integer not null,"
                    "    q3 text not null,"
                    "    confidence text not null,"
                    "    unique(astring_id),"
                    "    foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: amino_seqs
    if (!query.exec("CREATE TABLE amino_seqs ("
                    "    id integer not null primary key,"
                    "    astring_id integer not null,"
                    "    start integer not null,"       //
                    "    stop integer not null,"
                    "    name text,"
                    "    source text,"              // E.g. organism
                    "    description text,"
                    "    notes text,"
                    "    foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE INDEX amino_seqs_astring_id_index ON amino_seqs(astring_id)"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE TABLE amino_msas ("
                    "    id integer not null primary key,"
                    "    name text not null,"
                    "    description text,"
                    "    notes text"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE TABLE amino_msas_members ("
                    "    amino_msa_id integer not null,"
                    "    amino_seq_id integer not null,"
                    "    position integer not null,"
                    "    sequence text not null,"
                    "    primary key(amino_msa_id, amino_seq_id),"
                    "    foreign key(amino_msa_id) references amino_msas(id) on update cascade on delete cascade,"
                    "    foreign key(amino_seq_id) references amino_seqs(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: dstrings
    if (!query.exec("CREATE TABLE dstrings ("
                    "    id integer not null primary key,"
                    "    digest text not null,"
                    "    length integer not null,"
                    "    sequence text not null,"
                    "    check(length = length(sequence))"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE INDEX dstrings_digest_index ON dstrings(digest)"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: dna_seqs
    if (!query.exec("CREATE TABLE dna_seqs ("
                    "    id integer not null primary key,"
                    "    dstring_id integer not null,"
                    "    start integer not null,"       //
                    "    stop integer not null,"
                    "    name text,"
                    "    source text,"              // E.g. organism
                    "    description text,"
                    "    notes text,"
                    "    foreign key(dstring_id) references dstrings(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE INDEX dna_seqs_dstring_id_index ON dna_seqs(dstring_id)"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: primer search parameters
    if (!query.exec("CREATE TABLE primer_search_parameters ("
                    "    id integer not null primary key,"
                    "    amplicon_length_min integer not null,"
                    "    amplicon_length_max integer not null,"
                    "    primer_length_min integer not null,"
                    "    primer_length_max integer not null,"
                    "    forward_re_name text,"
                    "    forward_re_site text,"
                    "    forward_re_sense_cuts text,"
                    "    forward_re_antisense_cuts text,"
                    "    reverse_re_name text,"
                    "    reverse_re_site text,"
                    "    reverse_re_sense_cuts text,"
                    "    reverse_re_antisense_cuts text,"
                    "    forward_terminal_pattern text,"
                    "    reverse_terminal_pattern text,"
                    "    primer_tm_range_min integer not null,"
                    "    primer_tm_range_max integer not null,"
                    "    na_molar_concentration real not null,"
                    "    primer_dna_molar_concentration real not null,"
                    "    max_primer_pair_delta_tm real not null"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: primers
    if (!query.exec("CREATE TABLE primers ("
                    "    id integer not null primary key,"
                    "    primer_search_parameters_id integer,"
                    "    name text,"
                    "    re_name text,"
                    "    re_site text,"
                    "    re_sense_cuts text,"
                    "    re_antisense_cuts text,"
                    "    core_sequence text not null,"
                    "    tm real not null,"
                    "    homo_dimer_score real not null"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Table: dna_seq_primers
    if (!query.exec("CREATE TABLE dna_seqs_primers ("
                    "    dna_seq_id integer not null,"
                    "    primer_id integer not null,"
                    "    primary key(dna_seq_id, primer_id),"
                    "    foreign key(dna_seq_id) references dna_seqs(id) on update cascade on delete cascade,"
                    "    foreign key(primer_id) references primers(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE TABLE dna_msas ("
                    "    id integer not null primary key,"
                    "    name text not null,"
                    "    description text,"
                    "    notes text"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("CREATE TABLE dna_msas_members ("
                    "    dna_msa_id integer not null,"
                    "    dna_seq_id integer not null,"
                    "    position integer not null,"
                    "    sequence text not null,"
                    "    primary key(dna_msa_id, dna_seq_id),"
                    "    foreign key(dna_msa_id) references dna_msas(id) on update cascade on delete cascade,"
                    "    foreign key(dna_seq_id) references dna_seqs(id) on update cascade on delete cascade"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    // Blast reports
    if (!query.exec("CREATE TABLE blast_reports ("
                    "    id integer not null primary key,"
                    "    name text not null,"
                    "    description text,"
                    "    notes text,"
                    "    query_type text not null,"
                    "    query_seq_id integer not null,"
                    "    query_start integer not null,"
                    "    query_stop integer not null,"
                    "    query_sequence text not null,"
                    "    source_file text not null,"
                    "    database text,"
                    "    sequences int default 0,"
                    "    letters int default 0,"
                    "    bytes int default 0,"
                    "    check(query_start > 0),"
                    "    check(query_stop >= query_start),"
                    "    check(query_type IN ('amino', 'dna')),"
                    "    check(sequences >= 0),"
                    "    check(letters >= 0),"
                    "    check(bytes >= 0)"
                    ");"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }
}

/**
  * @returns QSqlDatabase
  */
QSqlDatabase SqliteAdocSource::database() const
{
    return QSqlDatabase::database(connectionName_, false);
}

/**
  * @returns bool
  */
bool SqliteAdocSource::isValidDatabase()
{
    QSqlDatabase db = database();
    QSqlQuery query(db);

    return query.exec("PRAGMA integrity_check");
}

/**
  * @param fileName [const QString &]
  * @returns bool
  */
bool SqliteAdocSource::openOrCreate(const QString &fileName)
{
    // Close any open connection
    if (connectionName_.isEmpty() == false)
        close();

    QString connectionName = QFileInfo(fileName).fileName() + QString::number(connectionNumber_);
    bool opened = false;
    {
        // Hide the QSqlDatabase reference inside a code block so that we can remove it later if it failed to open
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(fileName);
        opened = db.open();
    }
    if (!opened)
    {
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    connectionName_ = connectionName;
    ++connectionNumber_;

    fileName_ = fileName;

    runPragmas();

    return true;
}

/**
  */
void SqliteAdocSource::runPragmas()
{
    ASSERT(isOpen());

    QSqlDatabase db = database();
    QSqlQuery query(db);

    // Initialize foreign keys
    if (!query.exec("pragma foreign_keys = ON"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("pragma page_size = 4096"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

    if (!query.exec("pragma journal_mode = memory"))
    {
        qDebug() << Q_FUNC_INFO << query.lastError().text();
        throw 0;
    }

//    if (!query.exec("pragma synchronous = off"))
    //        throw;
}

/**
  */
void SqliteAdocSource::removeCruftAstrings()
{
    QSqlQuery query = getPreparedQuery("removeCruftAstrings",
                                       "DELETE FROM astrings "
                                       "WHERE id IN ( "
                                       "  SELECT a.id "
                                       "  FROM astrings a LEFT OUTER JOIN amino_seqs b ON (a.id = b.astring_id) "
                                       "  WHERE b.astring_id is null)");
    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        throw 0;
    }
}

/**
  */
void SqliteAdocSource::removeCruftDstrings()
{
    QSqlQuery query = getPreparedQuery("removeCruftDstrings",
                                       "DELETE FROM dstrings "
                                       "WHERE id IN ( "
                                       "  SELECT a.id "
                                       "  FROM dstrings a LEFT OUTER JOIN dna_seqs b ON (a.id = b.dstring_id) "
                                       "  WHERE b.dstring_id is null)");
    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        throw 0;
    }
}

void SqliteAdocSource::removeOrphanPrimerSearchParameters()
{
    QSqlQuery query = getPreparedQuery("removeOrphanPrimerSearchParameters",
                                       "DELETE FROM primer_search_parameters "
                                       "WHERE id IN ("
                                       "   SELECT a.id"
                                       "   FROM primer_search_parameters a LEFT OUTER JOIN primers b ON (a.id = b.primer_search_parameters_id) "
                                       "   WHERE b.primer_search_parameters_id is null)");

    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        throw 0;
    }
}
