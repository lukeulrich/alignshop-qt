/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTREPORTMAPPER_H
#define BLASTREPORTMAPPER_H

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "GenericEntityMapper.h"
#include "../Entities/BlastReport.h"

class BlastReportMapper : public GenericEntityMapper<BlastReport, BlastReportPod>
{
public:
    BlastReportMapper(IAdocSource *adocSource, const QDir &blastDirectory)
        : GenericEntityMapper<BlastReport, BlastReportPod>(adocSource),
          blastDirectory_(blastDirectory)
    {
    }

    /**
      * Extends the generic version by erasing all associated files when erased.
      */
    virtual bool erase(const QVector<BlastReport *> &blastReports) const
    {
        if (!GenericEntityMapper<BlastReport, BlastReportPod>::erase(blastReports))
            return false;

        foreach (const BlastReport *blastReport, blastReports)
            QFile::remove(blastDirectory_.filePath(blastReport->sourceFile()));

        return true;
    }

    /**
      * As with the other erase method, this one erases all associated files but first fetches the source files from the
      * data source before erasing from the data source.
      */
    virtual bool erase(const QVector<int> &ids) const
    {
        // Since we do not know the source files, we must first fetch them from the data source, delete the files, and
        // then the entities from the database.
        QStringList sourceFiles = adocSource()->blastReportCrud()->sourceFiles(ids);
        foreach (const QString &sourceFile, sourceFiles)
            QFile::remove(blastDirectory_.filePath(sourceFile));

        return GenericEntityMapper<BlastReport, BlastReportPod>::erase(ids);
    }

private:
    QDir blastDirectory_;
};

#endif // BLASTREPORTMAPPER_H
