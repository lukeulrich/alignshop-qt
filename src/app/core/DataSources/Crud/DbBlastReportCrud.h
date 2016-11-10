/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DBBLASTREPORTCRUD_H
#define DBBLASTREPORTCRUD_H

#include <QtCore/QStringList>

#include "AbstractDbEntityCrud.h"
#include "IBlastReportCrud.h"

class BlastReport;
struct BlastReportPod;

class DbBlastReportCrud : public AbstractDbEntityCrud<BlastReport, BlastReportPod>,
                          public IBlastReportCrud
{
public:
    DbBlastReportCrud(IDbSource *dbSource);

    virtual void erase(const QVector<BlastReport *> &blastReports);
    virtual void eraseByIds(const QVector<int> &ids);
    virtual QVector<BlastReportPod> read(const QVector<int> &ids);
    virtual void save(const QVector<BlastReport *> &blastReports);

    QStringList sourceFiles(const QVector<int> &ids);               // Returns a list of source files corresponding to ids

private:
    void insert(BlastReport *blastReport);
    void update(BlastReport *blastReport);
};

#endif // DBBLASTREPORTCRUD_H
