/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IBLASTREPORTCRUD_H
#define IBLASTREPORTCRUD_H

#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "IEntityCrud.h"

class BlastReport;
struct BlastReportPod;

class IBlastReportCrud : public virtual IEntityCrud<BlastReport, BlastReportPod>
{
public:
    virtual QStringList sourceFiles(const QVector<int> &ids) = 0;
};

#endif // IBLASTREPORTCRUD_H
