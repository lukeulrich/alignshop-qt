/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "DbAnonSeqFactory.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QStringList>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "SynchronousAdocDataSource.h"

#include <QtDebug>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * The ready_ flag is set to true if tableName is not empty and adocDataSource is not null.
  *
  * @param digestAlgorithm [QCryptographicHash::Algorithm]
  */
DbAnonSeqFactory::DbAnonSeqFactory(QCryptographicHash::Algorithm digestAlgorithm, const QString &tableName, SynchronousAdocDataSource *adocDataSource) :
    AnonSeqFactory(digestAlgorithm), sourceTable_(tableName.trimmed()), adocDataSource_(adocDataSource)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Add bioString to the relevant table of AnonSeqs. bioString is first reduced, hashed, and added
  * if another AnonSeq with this sequence does not already exist. If no AnonSeq exists with this exact
  * sequence, it is inserted into the database and returns an AnonSeq with this information and with
  * an identifier assigned by the database.
  *
  * Note, if AnonSeqFactory::bioStringPrototype_ is set, the returned AnonSeq's biostring will reflect
  * this prototype's class otherwise it will be a reduced BioString.
  *
  * Returns an invalid AnonSeq if the factory is not properly initialized or the reduced bioString is
  * empty.
  *
  * @param bioString [const BioString &]
  * @return const AnonSeq &
  * @see BioString::reduced()
  */
AnonSeq DbAnonSeqFactory::add(const BioString &bioString)
{
    if (!ready())
        return AnonSeq();

    BioString reducedBioString = bioString.reduced();
    if (reducedBioString.sequence().isEmpty())
        return AnonSeq();

    // Attempt to fetch this sequence from the database using its bioString
    AnonSeq dbAnonSeq = fetch(reducedBioString);
    if (dbAnonSeq.isValid())
        return dbAnonSeq;

    // If we get here, it was not found in the database. Insert a record for it into the database and
    // create a corresponding AnonSeq for it.
    QSqlQuery i_seq(adocDataSource_->getDatabase());
    if (!i_seq.prepare("INSERT INTO " % sourceTable_ % " (digest, sequence) VALUES (?, ?)"))
        return AnonSeq();

    // Bind the values
    i_seq.bindValue(0, base64Digest(reducedBioString).constData());
    i_seq.bindValue(1, reducedBioString.sequence());
    if (i_seq.exec())
    {
        // Successfully inserted the sequence into the database. Retrieve it's id and return a properly
        // initialized AnonSeq
        return AnonSeq(i_seq.lastInsertId().toInt(), reducedBioString);
    }

    qDebug() << i_seq.lastError().text();

    return AnonSeq();
}

/**
  * @returns AdocDataSource *
  */
const SynchronousAdocDataSource *DbAnonSeqFactory::adocDataSource() const
{
    return adocDataSource_;
}

/**
  * Before searching the database table for the presence of bioString, it is first reduced and hashed
  * and then used to query the associated factory table.
  *
  * Note, if AnonSeqFactory::bioStringPrototype_ is set, the returned AnonSeq's biostring will reflect
  * this prototype's class otherwise it will be a reduced BioString.
  *
  * Returns an invalid AnonSeq if the factory is not properly initialized or bioString is not found in
  * the database table.
  *
  * @param bioString [const BioString &]
  * @returns AnonSeq
  * @see fetch(int)
  */
AnonSeq DbAnonSeqFactory::fetch(const BioString &bioString) const
{
    if (!ready())
        return AnonSeq();

    ASSERT_X(adocDataSource_->getDatabase().tables().contains(sourceTable_), QString("database does not contain table: %1").arg(sourceTable_).toAscii());

    // Assumption: tableName_.digest uniquely identifies a single row; however, it is possible that if
    //             the database is corrupt that there will be multiple rows with the same digest.
    // Therefore, even if there are multiple rows, we only fetch the one with the smallest value for id
    QSqlQuery s_seq(adocDataSource_->getDatabase());
    if (!s_seq.prepare("SELECT id, sequence FROM " % sourceTable_ % " WHERE digest = ? ORDER BY id LIMIT 1"))
        return AnonSeq();

    // base64Digest returns a QByteArray, which as revealed by testing are not equivalent when performing
    // database searches as their QString or char * counterparts. Thus, it is essential to convert it to
    // a const char * before binding the database value to the prepared query.
    s_seq.bindValue(0, base64Digest(bioString.reduced()).constData());
    if (s_seq.exec() && s_seq.next())
        return createPrototypedAnonSeq(s_seq.value(0).toInt(), s_seq.value(1).toString());

    return AnonSeq();
}

/**
  * Queries the associated database table for a sequence with an identifier equal to id.
  *
  * Note, if AnonSeqFactory::bioStringPrototype_ is set, the returned AnonSeq's biostring will reflect
  * this prototype's class otherwise it will be a reduced BioString.
  *
  * Returns an invalid AnonSeq if the factory is not properly initialized or id is not found in the
  * database table.
  *
  * @param id [int]
  * @returns AnonSeq
  * @see fetch(const BioString &)
  */
AnonSeq DbAnonSeqFactory::fetch(int id) const
{
    if (!ready())
        return AnonSeq();

    ASSERT_X(adocDataSource_->getDatabase().tables().contains(sourceTable_), QString("database does not contain table: %1").arg(sourceTable_).toAscii());

    QSqlQuery s_seq(adocDataSource_->getDatabase());
    if (!s_seq.prepare("SELECT sequence FROM " % sourceTable_ % " WHERE id = ?"))
        return AnonSeq();

    s_seq.bindValue(0, id);
    if (s_seq.exec() && s_seq.next())
        return createPrototypedAnonSeq(id, s_seq.value(0).toString());

    return AnonSeq();
}

/**
  * @returns bool
  */
bool DbAnonSeqFactory::ready() const
{
    return sourceTable_.size() && adocDataSource_;
}

/**
  * Deletes from the database table any records that have a sequence equal to the reduced form of bioString.
  *
  * Returns 0 if no rows with a matching sequence are found or the factory is not properly initialized.
  *
  * @param bioString [const BioString &]
  * @returns int
  * @see remove(int)
  */
int DbAnonSeqFactory::remove(const BioString &bioString)
{
    if (!ready())
        return 0;

    ASSERT_X(adocDataSource_->getDatabase().tables().contains(sourceTable_), QString("database does not contain table: %1").arg(sourceTable_).toAscii());

    QSqlQuery d_seq(adocDataSource_->getDatabase());
    if (!d_seq.prepare("DELETE FROM " + sourceTable_ + " WHERE digest = ?"))
        return 0;

    d_seq.bindValue(0, base64Digest(bioString.reduced()).constData());
    if (d_seq.exec())
        return d_seq.numRowsAffected();

    return 0;
}

/**
  * Deletes from the database table any records with the id field equal to id.
  *
  * Returns 0 if no rows with the identifier id are found or the factory is not properly initialized.
  *
  * @param int
  * @returns int
  * @see remove(const BioString &)
  */
int DbAnonSeqFactory::remove(int id)
{
    if (!ready())
        return 0;

    ASSERT_X(adocDataSource_->getDatabase().tables().contains(sourceTable_), QString("database does not contain table: %1").arg(sourceTable_).toAscii());

    QSqlQuery d_seq(adocDataSource_->getDatabase());
    if (!d_seq.prepare("DELETE FROM " + sourceTable_ + " WHERE id = ?"))
        return 0;

    d_seq.bindValue(0, id);
    if (d_seq.exec())
        return d_seq.numRowsAffected();

    return 0;
}

/**
  * @param adocDataSource [AdocDataSource *]
  */
void DbAnonSeqFactory::setAdocDataSource(SynchronousAdocDataSource *adocDataSource)
{
    adocDataSource_ = adocDataSource;
}

/**
  * @param tableName [const QString &]
  * @see sourceTable()
  */
void DbAnonSeqFactory::setSourceTable(const QString &tableName)
{
    sourceTable_ = tableName.trimmed();
}

/**
  * @returns int
  */
int DbAnonSeqFactory::size() const
{
    if (!ready())
        return 0;

    ASSERT_X(adocDataSource_->getDatabase().tables().contains(sourceTable_), QString("database does not contain table: %1").arg(sourceTable_).toAscii());

    QSqlQuery s_count(adocDataSource_->getDatabase());
    if (!s_count.prepare("SELECT count(*) FROM " + sourceTable_))
        return 0;

    if (!s_count.exec() || !s_count.next())
        return 0;

    return s_count.value(0).toInt();
}

/**
  * @returns QString
  */
QString DbAnonSeqFactory::sourceTable() const
{
    return sourceTable_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param id [int]
  * @param bioString [const BioString &]
  * @returns AnonSeq
  */
AnonSeq DbAnonSeqFactory::createPrototypedAnonSeq(int id, const BioString &bioString) const
{
    ASSERT_X(id > 0, "id must be greater than 0");

    return AnonSeqFactory::createPrototypedAnonSeq(id, bioString);
}
