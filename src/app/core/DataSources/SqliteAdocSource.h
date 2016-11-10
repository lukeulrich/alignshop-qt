/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SQLITEADOCSOURCE_H
#define SQLITEADOCSOURCE_H

#include <QtCore/QString>
#include <QtSql/QSqlDatabase>

#include "AbstractAdocSource.h"
#include "AbstractDbSource.h"

#include "Crud/DbAstringCrud.h"
#include "Crud/DbAminoSeqCrud.h"
#include "Crud/DbBlastReportCrud.h"
#include "Crud/DbDstringCrud.h"
#include "Crud/DbDnaSeqCrud.h"
#include "Crud/DbAminoMsaCrud.h"
#include "Crud/DbDnaMsaCrud.h"

class SqliteAdocSource : public AbstractAdocSource,
                         public AbstractDbSource
{
public:
    // -------------------------------------------------------------------------------------------------
    // Constructors and destructor
    SqliteAdocSource();
    ~SqliteAdocSource();


    // -------------------------------------------------------------------------------------------------
    // Public methods
    void close();
    bool createAndOpen(const QString &fileName);
    bool isOpen() const;
    QString fileName() const;
    bool open(const QString &fileName);
    bool saveAs(const QString &fileName);


    // -------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    void begin();
    void rollback();
    void end();

    void removeCruft();
    void vacuum();

    AdocTreeNode *readEntityTree();
    void saveEntityTree(AdocTreeNode *root);

    IAnonSeqEntityCrud<Astring, AstringPod> *astringCrud();
    IEntityCrud<AminoSeq, AminoSeqPod> *aminoSeqCrud();
    IAnonSeqEntityCrud<Dstring, DstringPod> *dstringCrud();
    IEntityCrud<DnaSeq, DnaSeqPod> *dnaSeqCrud();

    IMsaCrud<AminoMsa, AminoMsaPod> *aminoMsaCrud();
    IMsaCrud<DnaMsa, DnaMsaPod> *dnaMsaCrud();

    IBlastReportCrud *blastReportCrud();

private:
    void createTables() const;
    QSqlDatabase database() const;
    bool isValidDatabase();
    bool openOrCreate(const QString &fileName);
    void runPragmas();              // Sets up pragmas that should be present for every database connection

    // Specific cruft-removal methods
    void removeCruftAstrings();
    void removeCruftDstrings();
    void removeOrphanPrimerSearchParameters();

    QString connectionName_;
    static int connectionNumber_;
    QString fileName_;

    DbAstringCrud astringCrud_;
    DbAminoSeqCrud aminoSeqCrud_;
    DbDstringCrud dstringCrud_;
    DbDnaSeqCrud dnaSeqCrud_;
    DbAminoMsaCrud aminoMsaCrud_;
    DbDnaMsaCrud dnaMsaCrud_;
    DbBlastReportCrud blastReportCrud_;

#ifdef TESTING
    friend class TestSqliteAdocSource;
#endif
};

#endif // SQLITEADOCSOURCE_H
