/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBDNASEQCRUD_H
#define DBDNASEQCRUD_H

#include "AbstractDbEntityCrud.h"

class DnaSeq;
struct DnaSeqPod;
class Primer;
typedef QVector<Primer> PrimerVector;
class PrimerSearchParameters;

class DbDnaSeqCrud : public AbstractDbEntityCrud<DnaSeq, DnaSeqPod>
{
public:
    DbDnaSeqCrud(IDbSource *dbSource);

    virtual void save(const QVector<DnaSeq *> &dnaSeqs);
    virtual void erase(const QVector<DnaSeq *> &dnaSeqs);
    virtual void eraseByIds(const QVector<int> &ids);
//    virtual QVector<DnaSeq *> read(const QVector<int> &ids);
    virtual QVector<DnaSeqPod> read(const QVector<int> &ids);

private:
    void erasePrimers(const int dnaSeqId);
    void update(DnaSeq *dnaSeq);
    void insert(DnaSeq *aminoSeq);
    void insertCoreDnaSeq(DnaSeq *aminoSeq);
    void updateCoreDnaSeq(const DnaSeq *aminoSeq);
    void savePrimers(int dnaSeqId, PrimerVector &primers);
    QVector<int> primerIdVector(const PrimerVector &primers) const;
    void deleteDnaSeqPrimersNotIn(const int dnaSeqId, const QVector<int> &validPrimerIds);
    void deleteFromPrimersTable(const int dnaSeqId, const QString validPrimerIdString);
    void deleteFromDnaSeqPrimersTable(const int dnaSeqId, const QString validPrimerIdString);
    void insertPrimer(Primer &primer);
    int insertPrimerSearchParameters(const PrimerSearchParameters *primerSearchParameters);
    void associatePrimerWithDnaSeq(int dnaSeqId, int primerId);
    void updatePrimer(Primer &primer);

    PrimerVector readPrimers(const int dnaSeqId) const;
    QVector<int> fetchUniquePrimerSearchParameterIds(const int dnaSeqId) const;
};

#endif // DBDNASEQCRUD_H
