/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKDBSOURCE_H
#define MOCKDBSOURCE_H

#include <QtCore/QVector>

#include <QtSql/QSqlDatabase>

#include "AbstractDbSource.h"
#include "../Seq.h"
#include "../macros.h"

class MockDbSource : public AbstractDbSource
{
public:
    MockDbSource() : AbstractDbSource()
    {
        connectionName_ = "MockDbSource";

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName_);
        db.setDatabaseName(":memory:");
        db.open();

        createTables(db);
        createSampleData(db);
    }

    ~MockDbSource()
    {
        QSqlDatabase::removeDatabase(connectionName_);
    }

    static QVector<Seq> aseqs()
    {
        return aseqs_;
    }

    QSqlDatabase database() const
    {
        return QSqlDatabase::database(connectionName_, false);
    }

    static void createTables(QSqlDatabase &db)
    {
        ASSERT(db.isValid() && db.isOpen());
        QSqlQuery create(db);
        db.transaction();
        ASSERT(create.exec("pragma foreign_keys = on"));
        if (!create.exec("CREATE TABLE astrings ( "
                         "  id integer primary key autoincrement,"
                         "  digest text not null,"
                         "  length integer not null,"
                         "  sequence text not null,"
                         "  check(length > 0),"
                         "  check(length == length(sequence)),"
                         "  unique(digest)"
                         ");"))
            throw 0;

        if (!create.exec("CREATE TABLE coils ("
                         "  id integer primary key autoincrement,"
                         "  astring_id integer not null,"
                         "  start integer not null,"
                         "  stop integer not null,"
                         "  check (start > 0),"
                         "  check (stop >= start),"
                         "  foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                         ");"))
            throw 0;

        if (!create.exec("CREATE TABLE segs("
                         "  id integer primary key autoincrement,"
                         "  astring_id integer not null,"
                         "  start integer not null,"
                         "  stop integer not null,"
                         "  check (start > 0),"
                         "  check (stop >= start),"
                         "  foreign key(astring_id) references astrings(id) on update cascade on delete cascade"
                         ");"))
            throw 0;

        if (!create.exec("CREATE TABLE amino_seqs ("
                         "  id integer not null primary key autoincrement,"
                         "  astring_id integer not null,"
                         "  start integer not null,"
                         "  stop integer not null,"
                         "  name text,"
                         "  source text,"
                         "  description text,"
                         "  notes text,"
                         "  check(astring_id > 0),"
                         "  check(start > 0),"
                         "  check(stop >= start)"
                         ")"))
            throw 0;
        db.commit();
    }

    static void createSampleData(QSqlDatabase &db)
    {
        db.transaction();
        QSqlQuery query(db);
        QVERIFY(query.prepare("INSERT INTO astrings (id, digest, length, sequence) VALUES (?, ?, ?, ?)"));
        for (int i=0; i<3; ++i)
        {
            query.addBindValue(i+1);
            query.addBindValue(aseqs_.at(i).digest());
            query.addBindValue(aseqs_.at(i).length());
            query.addBindValue(aseqs_.at(i).asByteArray());
            QVERIFY(query.exec());
        }

        // First sequence has 1 coils, 1 segs
        QVERIFY(query.exec("INSERT INTO coils (id, astring_id, start, stop) VALUES (1, 1, 1, 3)"));
        QVERIFY(query.exec("INSERT INTO segs (id, astring_id, start, stop) VALUES (1, 1, 4, 6)"));

        // Second sequence has 2 coils, 0 segs
        QVERIFY(query.exec("INSERT INTO coils (id, astring_id, start, stop) VALUES (2, 2, 4, 5)"));
        QVERIFY(query.exec("INSERT INTO coils (id, astring_id, start, stop) VALUES (3, 2, 8, 10)"));

        // Third sequence has 0 coils, 2 segs
        QVERIFY(query.exec("INSERT INTO segs (id, astring_id, start, stop) VALUES (2, 3, 1, 6)"));
        QVERIFY(query.exec("INSERT INTO segs (id, astring_id, start, stop) VALUES (3, 3, 9, 12)"));

        // ----------------------------------
        // Amino seqs
        QVERIFY(query.exec("INSERT INTO amino_seqs VALUES (10, 1, 1, 6, 'Alpha', 'E coli', 'No description', NULL)"));
        QVERIFY(query.exec("INSERT INTO amino_seqs VALUES (11, 3, 1, 7, 'Beta', NULL, NULL, 'No notes')"));
        QVERIFY(query.exec("INSERT INTO amino_seqs VALUES (12, 3, 8, 14, 'Gamma', 'Source', 'last half', 'Notes')"));

        db.commit();
    }

private:
    QString connectionName_;
    static QVector<Seq> aseqs_;
};

QVector<Seq> MockDbSource::aseqs_(QVector<Seq>() << Seq("ABCDEF", eAminoGrammar)
                                                 << Seq("ABCDEFGHIJKLM", eAminoGrammar)
                                          //             12345678901234
                                                 << Seq("ZYXWVUTSRQPONM", eAminoGrammar));


#endif // MOCKDBSOURCE_H
