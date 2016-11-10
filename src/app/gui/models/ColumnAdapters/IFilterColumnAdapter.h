/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IFILTERCOLUMNADAPTER_H
#define IFILTERCOLUMNADAPTER_H

#include "IColumnAdapter.h"
#include "../../../core/global.h"

/**
  * IFilterColumnAdapter extends the IColumnAdapter interface with proxy methods for filtering columns passed between
  * another adapter and a downstream target.
  *
  * Note: It is necessary to define the filtered columns prior to any connected components calling columnCount().
  */
class IFilterColumnAdapter : public IColumnAdapter
{
    Q_OBJECT

public:
    virtual void exclude(int sourceColumn) = 0;
    virtual void include(int sourceColumn) = 0;
    virtual int mapFromSource(int sourceColumn) const = 0;
    virtual int mapToSource(int proxyColumn) const = 0;
    virtual void setSourceAdapter(IColumnAdapter *sourceAdapter) = 0;
    virtual IColumnAdapter *sourceAdapter() const = 0;

protected:
    IFilterColumnAdapter(QObject *parent = nullptr) : IColumnAdapter(parent) {}
};

#endif // IFILTERCOLUMNADAPTER_H
